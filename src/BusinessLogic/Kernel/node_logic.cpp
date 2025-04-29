#include "node_logic.h"
#include "network.h"
#include <boost/asio.hpp>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

// Подключаем библиотеки для работы с сетью, файлами и JSON
using boost::asio::ip::tcp;
using json = nlohmann::json;
namespace fs = std::filesystem;

// Конструктор узла
NodeLogic::NodeLogic(const NodeIpAddress& address) : status_(StatusEnum::stopped), my_address_(address) {
    load_nodes_from_config(); // Загружаем конфигурацию узлов
    fs::create_directories("node_parts/" + my_address_); // Создаем папку для хранения частей
}

// Загружает список узлов из конфигурационного файла
void NodeLogic::load_nodes_from_config() {
    std::ifstream config_file("nodes_config.json");
    if (!config_file.is_open()) {
        std::cerr << "Failed to open nodes_config.json, using default nodes\n";
        cluster_nodes_ = {"127.0.0.1:8080", "127.0.0.1:8081", "127.0.0.1:8082"};
        save_nodes_to_config();
        return;
    }

    try {
        json config;
        config_file >> config;
        cluster_nodes_ = config["nodes"].get<std::vector<std::string>>();
    } catch (const std::exception& e) {
        std::cerr << "Error parsing nodes_config.json: " << e.what() << ", using default nodes\n";
        cluster_nodes_ = {"127.0.0.1:8080", "127.0.0.1:8081", "127.0.0.1:8082"};
        save_nodes_to_config();
    }
}

// Сохраняет список узлов в конфигурационный файл
void NodeLogic::save_nodes_to_config() {
    json config;
    config["nodes"] = cluster_nodes_;
    std::ofstream config_file("nodes_config.json");
    if (config_file.is_open()) {
        config_file << config.dump(4);
    } else {
        std::cerr << "Failed to save nodes_config.json\n";
    }
}

// Запускает узел
void NodeLogic::start() {
    status_ = StatusEnum::working;
    std::cout << "[Node " << my_address_ << "] Started\n";

    // Синхронизируем метаданные с другими узлами
    for (const auto& node : cluster_nodes_) {
        if (node == my_address_ || !check_if_node_alive(node)) continue;

        json request = {{"type", "list_files"}};
        std::string response = sendData(node, request.dump());
        if (!response.empty()) {
            try {
                json file_list = json::parse(response);
                sync_metadata(file_list);
                std::cout << "[Node " << my_address_ << "] Synced metadata with " << node << "\n";
                break;
            } catch (const std::exception& e) {
                std::cerr << "[Node " << my_address_ << "] Failed to sync with " << node << ": " << e.what() << "\n";
            }
        }
    }
}

// Останавливает узел
void NodeLogic::stop() {
    status_ = StatusEnum::stopped;
    std::cout << "[Node " << my_address_ << "] Stopped\n";
}

// Проверяет, доступен ли узел
bool NodeLogic::check_if_node_alive(const NodeIpAddress& address) {
    try {
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(address.substr(0, address.find(':')), address.substr(address.find(':') + 1));
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);
        socket.close();
        return true;
    } catch (std::exception& e) {
        std::cout << "[Node " << my_address_ << "] Node " << address << " is down: " << e.what() << "\n";
        return false;
    }
}

// Проверяет, все ли узлы в кластере активны
bool NodeLogic::check_if_cluster_consistent() {
    for (const auto& node : cluster_nodes_) {
        if (node != my_address_ && !check_if_node_alive(node)) {
            return false;
        }
    }
    return true;
}

// Распределяет части файла по узлам
void NodeLogic::calculate_new_file_parts_map(const std::string& file, const FileMetaData& metadata) {
    file_parts_map_.clear();
    size_t part_count = metadata.part_count;
    size_t node_count = cluster_nodes_.size();

    if (part_count != node_count) {
        std::cerr << "Error: Number of parts (" << part_count << ") must equal number of nodes (" << node_count << ")\n";
        return;
    }

    for (size_t i = 0; i < part_count; ++i) {
        FilePart part{file, i, "", metadata.part_hashes[i]};
        for (size_t j = 0; j < node_count; ++j) {
            if (j == i) continue;
            NodeIpAddress node = cluster_nodes_[j];
            file_parts_map_[node].push_back(part);
        }
    }
    files_metadata_[file] = metadata;
}

// Уведомляет узел о необходимости реконфигурации
void NodeLogic::alert_node_that_reconfiguration_needed(const NodeIpAddress& address) {
    status_ = StatusEnum::ready_to_reconfigure;
    std::cout << "[Node " << my_address_ << "] Alerting node " << address << " for reconfiguration\n";
}

// Уведомляет весь кластер о необходимости реконфигурации
void NodeLogic::alert_cluster_that_reconfiguration_needed() {
    for (const auto& node : cluster_nodes_) {
        if (node != my_address_) {
            alert_node_that_reconfiguration_needed(node);
        }
    }
    status_ = StatusEnum::reconfiguring;
    for (const auto& [file, metadata] : files_metadata_) {
        calculate_new_file_parts_map(file, metadata);
    }
    status_ = StatusEnum::working;
}

// Добавляет часть файла на узел
void NodeLogic::add_file_part(const FilePart& part, const std::string& content) {
    std::string file_name = fs::path(part.file_name).filename().string();
    std::string part_dir = "node_parts/" + my_address_;
    fs::create_directories(part_dir);
    std::string part_path = part_dir + "/" + file_name + "_part_" + std::to_string(part.part_index);
    std::cout << "[Node " << my_address_ << "] Saving part to: " << part_path << "\n";

    std::ofstream out_file(part_path, std::ios::binary);
    if (!out_file) {
        std::cerr << "Failed to create part file: " << part_path << "\n";
        throw std::runtime_error("Failed to create part file: " + part_path);
    }

    out_file.write(content.data(), content.size());
    if (!out_file.good()) {
        out_file.close();
        throw std::runtime_error("Failed to write content to part file: " + part_path);
    }
    out_file.close();

    FilePart new_part = part;
    new_part.file_name = file_name;
    new_part.file_path = part_path;
    file_parts_map_[my_address_].push_back(new_part);
    std::cout << "[Node " << my_address_ << "] Added part " << part.part_index << " for file " << file_name << " at " << part_path << "\n";

    if (check_storage_threshold()) {
        add_new_node(); // Добавляем новый узел, если место заканчивается
    }
}

// Возвращает части файла, хранящиеся на узле
std::vector<FilePart> NodeLogic::get_file_parts(const std::string& file) {
    std::vector<FilePart> parts;
    std::string file_name = fs::path(file).filename().string();
    for (const auto& [node, node_parts] : file_parts_map_) {
        for (const auto& part : node_parts) {
            if (part.file_name == file_name) {
                parts.push_back(part);
            }
        }
    }
    std::cout << "[Node " << my_address_ << "] Found " << parts.size() << " parts for file: " << file_name << "\n";
    return parts;
}

// Удаляет файл и его части
void NodeLogic::remove_file(const std::string& file) {
    for (auto& [node, parts] : file_parts_map_) {
        for (auto it = parts.begin(); it != parts.end();) {
            if (it->file_name == file) {
                fs::remove(it->file_path);
                it = parts.erase(it);
            } else {
                ++it;
            }
        }
    }
    files_metadata_.erase(file);
}

// Возвращает список файлов с метаданными
json NodeLogic::get_file_list() {
    json file_list = json::array();
    for (const auto& [file, metadata] : files_metadata_) {
        json file_info;
        file_info["file_id"] = file;
        file_info["name"] = metadata.name;
        file_info["size"] = metadata.size;
        file_info["creation_time"] = metadata.creation_time;
        file_info["hash"] = metadata.hash;
        file_info["part_count"] = metadata.part_count;
        file_info["part_hashes"] = metadata.part_hashes;
        file_list.push_back(file_info);
    }
    return file_list;
}

// Синхронизирует метаданные файлов
void NodeLogic::sync_metadata(const json& metadata) {
    if (metadata.is_array()) {
        for (const auto& item : metadata) {
            if (!item.is_object() || !item.contains("file_id")) {
                std::cerr << "Invalid metadata item: not an object or missing file_id\n";
                continue;
            }
            std::string file = item["file_id"].get<std::string>();
            FileMetaData meta;
            meta.name = item.value("name", file);
            meta.creation_time = item.value("creation_time", "");
            meta.hash = item.value("hash", "");
            meta.size = item.value("size", 0);
            meta.part_count = item.value("part_count", 0);
            if (item.contains("part_hashes") && item["part_hashes"].is_array()) {
                meta.part_hashes = item["part_hashes"].get<std::vector<std::string>>();
            }
            files_metadata_[file] = meta;
            std::cout << "[Node " << my_address_ << "] Synced metadata for file " << file << "\n";
        }
    } else if (metadata.is_object()) {
        if (!metadata.contains("file_id")) {
            std::cerr << "Invalid metadata object: missing file_id\n";
            return;
        }
        std::string file = metadata["file_id"].get<std::string>();
        FileMetaData meta;
        meta.name = metadata.value("name", file);
        meta.creation_time = metadata.value("creation_time", "");
        meta.hash = metadata.value("hash", "");
        meta.size = metadata.value("size", 0);
        meta.part_count = metadata.value("part_count", 0);
        if (metadata.contains("part_hashes") && metadata["part_hashes"].is_array()) {
            meta.part_hashes = metadata["part_hashes"].get<std::vector<std::string>>();
        }
        files_metadata_[file] = meta;
        std::cout << "[Node " << my_address_ << "] Synced metadata for file " << file << "\n";
    } else {
        std::cerr << "Invalid metadata: neither an array nor an object\n";
    }
}

// Возвращает объем используемого хранилища
size_t NodeLogic::get_storage_usage() {
    size_t total_size = 0;
    for (const auto& [node, parts] : file_parts_map_) {
        if (node != my_address_) continue;
        for (const auto& part : parts) {
            total_size += fs::file_size(part.file_path);
        }
    }
    return total_size;
}

// Проверяет, превышен ли лимит хранилища
bool NodeLogic::check_storage_threshold() {
    const size_t STORAGE_LIMIT = 1024 * 1024 * 1024; // 1 ГБ
    size_t usage = get_storage_usage();
    return usage > STORAGE_LIMIT / 2;
}

// Добавляет новый узел в кластер
void NodeLogic::add_new_node() {
    size_t new_port = 8080 + cluster_nodes_.size();
    std::string new_node = "127.0.0.1:" + std::to_string(new_port);
    cluster_nodes_.push_back(new_node);
    save_nodes_to_config();
    std::cout << "[Node " << my_address_ << "] Added new node: " << new_node << "\n";
    alert_cluster_that_reconfiguration_needed();
}