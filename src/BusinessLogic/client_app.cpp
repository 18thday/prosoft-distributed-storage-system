#include "client_app.h"
#include "network.h"
#include <boost/asio.hpp>
#include <boost/beast/core/detail/base64.hpp>
#include <openssl/sha.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <vector>
#include <random>

// Подключаем библиотеки для работы с сетью, кодированием, хешированием и файловой системой
using boost::asio::ip::tcp;
using json = nlohmann::json;
namespace fs = std::filesystem;
namespace beast = boost::beast;

// Функция для вычисления SHA-256 хеша данных
// Принимает строку (например, содержимое файла) и возвращает хеш в виде строки
std::string calculateHash(const std::string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)data.data(), data.size(), hash);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

// Функция для кодирования данных в base64
// Это нужно, чтобы безопасно передавать двоичные данные (например, части файлов) по сети
std::string base64_encode(const std::string& input) {
    std::size_t encoded_size = beast::detail::base64::encoded_size(input.size());
    std::string output(encoded_size, '\0');
    beast::detail::base64::encode(&output[0], input.data(), input.size());
    return output;
}

// Функция для декодирования данных из base64
// Используется, когда получаем части файлов с сервера
std::string base64_decode(const std::string& input) {
    std::size_t decoded_size = beast::detail::base64::decoded_size(input.size());
    std::string output(decoded_size, '\0');
    auto result = beast::detail::base64::decode(&output[0], input.data(), input.size());
    output.resize(result.first);
    return output;
}

// Конструктор класса, который связывает клиент с интерфейсом GUI
ClientToServerAPI::ClientToServerAPI(IBLClient* client) : m_client(client), m_node_ip("127.0.0.1:8080") {
    // Создаем отдельный поток для сетевых операций, чтобы не блокировать GUI
    network_thread_ = new NetworkThread();
    network_thread_->start();
}

// Деструктор, который корректно завершает сетевой поток
ClientToServerAPI::~ClientToServerAPI() {
    network_thread_->stop();
    network_thread_->quit();
    network_thread_->wait();
    delete network_thread_;
}

// Проверяет, доступен ли узел (сервер) по указанному адресу
bool ClientToServerAPI::isNodeAvailable(const std::string& nodeIpPort) {
    try {
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        // Разделяем адрес на IP и порт (например, "127.0.0.1:8080")
        auto endpoints = resolver.resolve(nodeIpPort.substr(0, nodeIpPort.find(':')), nodeIpPort.substr(nodeIpPort.find(':') + 1));
        tcp::socket socket(io_context);
        // Пробуем подключиться
        boost::asio::connect(socket, endpoints);
        socket.close();
        return true; // Узел доступен
    } catch (std::exception& e) {
        std::cout << "Node availability check failed for " << nodeIpPort << ": " << e.what() << "\n";
        return false; // Узел недоступен
    }
}

// Подключается к указанному узлу
void ClientToServerAPI::connect(const std::string& nodeIpPort) {
    m_node_ip = nodeIpPort; // Сохраняем адрес узла

    // Проверяем доступность узла с тремя попытками и паузой в 500 мс
    bool nodeAvailable = false;
    for (int i = 0; i < 3; ++i) {
        if (isNodeAvailable(nodeIpPort)) {
            nodeAvailable = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (!nodeAvailable) {
        m_client->onError("Узел " + nodeIpPort + " недоступен");
        return;
    }

    // Получаем список активных узлов и проверяем, есть ли наш узел в списке
    std::vector<std::string> nodes = getActiveNodes();
    if (std::find(nodes.begin(), nodes.end(), nodeIpPort) == nodes.end()) {
        m_client->onError("Узел " + nodeIpPort + " неактивен");
        return;
    }
    m_client->onNodesUpdated(nodes); // Обновляем список узлов в GUI
}

// Получает список активных узлов из конфигурационного файла
std::vector<std::string> ClientToServerAPI::getActiveNodes() {
    std::vector<std::string> nodes;
    std::ifstream config_file("nodes_config.json");
    if (!config_file) {
        std::cerr << "Failed to open nodes_config.json\n";
        return nodes; // Возвращаем пустой список, если файл не открыт
    }

    // Читаем JSON из файла
    try {
        json config;
        config_file >> config;
        if (config.contains("nodes") && config["nodes"].is_array()) {
            for (const auto& node : config["nodes"]) {
                if (node.is_string()) {
                    nodes.push_back(node.get<std::string>());
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing nodes_config.json: " << e.what() << "\n";
        return nodes;
    }
    config_file.close();

    // Проверяем, какие узлы активны
    std::vector<std::string> activeNodes;
    for (const auto& node : nodes) {
        if (isNodeAvailable(node)) {
            activeNodes.push_back(node);
        }
    }
    m_client->onNodesUpdated(activeNodes); // Обновляем GUI
    return activeNodes;
}

// Запрашивает список файлов с сервера
void ClientToServerAPI::listFiles() {
    if (!isNodeAvailable(m_node_ip)) {
        m_client->onError("Узел " + m_node_ip + " недоступен");
        return;
    }

    // Формируем запрос на получение списка файлов
    json request = {{"type", "list_files"}};
    std::string response = sendData(m_node_ip, request.dump());
    if (response.empty()) {
        m_client->onError("Не удалось получить список файлов с узла " + m_node_ip);
        return;
    }

    // Разбираем ответ от сервера
    try {
        json file_list = json::parse(response);
        if (!file_list.is_array()) {
            m_client->onError("Некорректный формат ответа от сервера: ожидался массив");
            return;
        }

        m_client->onFilesUpdated(file_list); // Передаем полный список в GUI
        std::vector<FileMetadata> files;
        for (const auto& file : file_list) {
            if (!file.is_object() || !file.contains("file_id") || !file.contains("size")) {
                m_client->onError("Некорректный формат файла в списке");
                continue;
            }
            // Формируем метаданные для GUI
            files.push_back({
                fs::path(file["file_id"].get<std::string>()).filename().string(),
                file["size"].get<size_t>()
            });
        }
        std::cout << "Sending " << files.size() << " files to GUI\n";
        m_client->onListFilesResult(files); // Отправляем список файлов в GUI
    } catch (const std::exception& e) {
        m_client->onError("Не удалось разобрать список файлов: " + std::string(e.what()));
    }
}

// Загружает файл на серверы
void ClientToServerAPI::uploadFile(const std::string& filePath) {
    if (!isNodeAvailable(m_node_ip)) {
        m_client->onUploadFinished(filePath, false, "Узел " + m_node_ip + " недоступен");
        return;
    }

    FileMetaData metadata;
    std::vector<std::string> active_nodes = getActiveNodes();
    size_t node_count = active_nodes.size();
    if (node_count < 2) {
        m_client->onUploadFinished(filePath, false, "Для загрузки требуется минимум 2 узла");
        return;
    }

    // Разделяем файл на части
    std::vector<FilePart> parts = splitFile(filePath, metadata, node_count);
    if (parts.empty()) {
        m_client->onUploadFinished(filePath, false, "Не удалось разделить файл: " + filePath);
        return;
    }
    std::string fileId = fs::path(filePath).filename().string();
    file_metadata_[fileId] = metadata; // Сохраняем метаданные файла
    distributeParts(parts, metadata, active_nodes); // Распределяем части по узлам

    // Удаляем временные файлы частей
    for (const auto& part : parts) {
        fs::remove(part.file_path);
    }

    m_client->onUploadFinished(fileId, true, "Загрузка завершена: " + fileId);
}

// Скачивает файл с серверов
void ClientToServerAPI::downloadFile(const std::string& fileId, const std::string& outputPath) {
    std::vector<std::string> active_nodes = getActiveNodes();
    if (active_nodes.empty()) {
        m_client->onDownloadFinished(fileId, false, "Нет доступных узлов для скачивания");
        return;
    }

    // Восстанавливаем файл из частей
    restoreFile(fileId, outputPath, active_nodes);
    m_client->onDownloadFinished(fileId, true, "Скачивание завершена: " + fileId);
}

// Удаляет файл с серверов
void ClientToServerAPI::deleteFile(const std::string& fileId) {
    std::vector<std::string> active_nodes = getActiveNodes();
    if (active_nodes.empty()) {
        m_client->onDeleteFinished(fileId, false, "Нет доступных узлов");
        return;
    }

    bool success = true;
    // Отправляем запрос на удаление на каждый активный узел
    for (const auto& node : active_nodes) {
        json request = {{"type", "delete"}, {"file_id", fileId}};
        std::string response = sendData(node, request.dump());
        if (response.empty()) {
            m_client->onDeleteFinished(fileId, false, "Не удалось удалить файл с узла " + node);
            success = false;
        }
    }

    if (success) {
        file_metadata_.erase(fileId); // Удаляем метаданные файла
        m_client->onDeleteFinished(fileId, true, "Файл удалён: " + fileId);
    } else {
        m_client->onDeleteFinished(fileId, false, "Не удалось удалить файл: " + fileId);
    }
}

// Разделяет файл на части
std::vector<FilePart> ClientToServerAPI::splitFile(const std::string& filePath, FileMetaData& metadata, size_t node_count) {
    // Используем класс Storage для разделения файла
    std::vector<FilePart> parts = Storage::splitFile(filePath, metadata, node_count);
    for (size_t i = 0; i < parts.size(); ++i) {
        m_client->onUploadProgress(filePath, i + 1, parts.size()); // Обновляем прогресс в GUI
    }
    return parts;
}

// Распределяет части файла по узлам
void ClientToServerAPI::distributeParts(const std::vector<FilePart>& parts, const FileMetaData& metadata, const std::vector<std::string>& active_nodes) {
    if (parts.empty() || active_nodes.empty()) {
        m_client->onError("Нет частей или узлов для распределения");
        return;
    }
    std::string file = fs::path(parts[0].file_name).filename().string();
    // Формируем JSON с метаданными файла
    json metadata_json;
    metadata_json["file_id"] = file;
    metadata_json["name"] = metadata.name;
    metadata_json["creation_time"] = metadata.creation_time;
    metadata_json["hash"] = metadata.hash;
    metadata_json["size"] = metadata.size;
    metadata_json["part_count"] = metadata.part_count;
    metadata_json["part_hashes"] = metadata.part_hashes;

    // Отправляем метаданные на все узлы
    for (const auto& node : active_nodes) {
        json request;
        request["type"] = "initiate_upload";
        request["metadata"] = metadata_json;

        std::string response = sendData(node, request.dump());
        if (response.empty() || response != "OK") {
            m_client->onError("Не удалось начать загрузку на узел " + node + ": " + response);
            return;
        }
        std::cout << "Received initiate_upload response from " << node << ": " << response << "\n";
    }

    size_t node_count = active_nodes.size();
    bool all_parts_uploaded = true;
    // Загружаем каждую часть на два узла для надежности
    for (size_t i = 0; i < parts.size(); ++i) {
        const auto& part = parts[i];
        std::ifstream part_file(part.file_path, std::ios::binary);
        if (!part_file) {
            m_client->onError("Не удалось прочитать часть файла: " + part.file_path);
            all_parts_uploaded = false;
            break;
        }
        std::string content((std::istreambuf_iterator<char>(part_file)), std::istreambuf_iterator<char>());
        part_file.close();

        // Кодируем содержимое части в base64
        std::string encoded_content = base64_encode(content);
        json part_request;
        part_request["type"] = "upload";
        part_request["file_id"] = file;
        part_request["part_index"] = part.part_index;
        part_request["hash"] = part.hash;
        part_request["content"] = encoded_content;

        // Выбираем два узла для загрузки части
        size_t node1_idx = i % node_count;
        size_t node2_idx = (i + 1) % node_count;
        std::vector<size_t> target_nodes = {node1_idx, node2_idx};

        for (size_t node_idx : target_nodes) {
            const std::string& node = active_nodes[node_idx];
            std::string response = sendData(node, part_request.dump());
            if (response.empty() || response != "OK") {
                m_client->onError("Не удалось загрузить часть " + std::to_string(i) + " на узел " + node + ": " + response);
                all_parts_uploaded = false;
                break;
            }
            std::cout << "Received upload response for part " << i << " from " << node << ": " << response << "\n";
        }
        if (!all_parts_uploaded) break;

        m_client->onUploadProgress(file, i + 1, parts.size());
    }

    if (!all_parts_uploaded) {
        m_client->onUploadFinished(file, false, "Не удалось загрузить все части файла " + file);
    }
}

// Восстанавливает файл из частей, полученных с узлов
void ClientToServerAPI::restoreFile(const std::string& fileId, const std::string& outputPath, const std::vector<std::string>& active_nodes) {
    std::map<size_t, FilePart> collected_parts;
    size_t total_parts = 0;

    // Запрашиваем список файлов для получения метаданных
    json list_request;
    list_request["type"] = "list_files";
    std::string list_response = sendData(m_node_ip, list_request.dump());
    if (list_response.empty()) {
        m_client->onError("Не удалось получить список файлов с узла " + m_node_ip + ": пустой ответ");
        return;
    }

    std::cout << "Received list_files response from " << m_node_ip << ": " << list_response << "\n";

    json json_metadata;
    FileMetaData metadata;
    // Разбираем ответ, ищем метаданные нужного файла
    try {
        json file_list = json::parse(list_response);
        if (!file_list.is_array()) {
            m_client->onError("Некорректный формат ответа list_files: ожидался массив, получено: " + list_response);
            return;
        }

        for (const auto& file : file_list) {
            if (file.is_object() && file.contains("file_id") && file["file_id"] == fileId) {
                json_metadata = file;
                break;
            }
        }
    } catch (const std::exception& e) {
        m_client->onError("Не удалось разобрать ответ list_files для файла " + fileId + ": " + e.what());
        return;
    }

    if (json_metadata.empty()) {
        m_client->onError("Файл " + fileId + " не найден на узле " + m_node_ip);
        return;
    }

    // Логируем метаданные для отладки
    std::cout << "Metadata for file " << fileId << ": " << json_metadata.dump(2) << "\n";

    // Преобразуем JSON в структуру FileMetaData
    try {
        if (!json_metadata.contains("name")) {
            m_client->onError("Отсутствует поле 'name' в метаданных файла " + fileId);
            return;
        }
        if (!json_metadata.contains("creation_time")) {
            m_client->onError("Отсутствует поле 'creation_time' в метаданных файла " + fileId);
            return;
        }
        if (!json_metadata.contains("hash")) {
            m_client->onError("Отсутствует поле 'hash' в метаданных файла " + fileId);
            return;
        }
        if (!json_metadata.contains("size")) {
            m_client->onError("Отсутствует поле 'size' в метаданных файла " + fileId);
            return;
        }
        if (!json_metadata.contains("part_count")) {
            m_client->onError("Отсутствует поле 'part_count' в метаданных файла " + fileId);
            return;
        }
        if (!json_metadata.contains("part_hashes") || !json_metadata["part_hashes"].is_array()) {
            m_client->onError("Отсутствует или некорректное поле 'part_hashes' в метаданных файла " + fileId);
            return;
        }

        metadata.name = json_metadata["name"].get<std::string>();
        metadata.creation_time = json_metadata["creation_time"].get<std::string>();
        metadata.hash = json_metadata["hash"].get<std::string>();
        metadata.size = json_metadata["size"].get<size_t>();
        metadata.part_count = json_metadata["part_count"].get<size_t>();
        for (const auto& hash : json_metadata["part_hashes"]) {
            if (hash.is_string()) {
                metadata.part_hashes.push_back(hash.get<std::string>());
            }
        }

        total_parts = metadata.part_count;
    } catch (const std::exception& e) {
        m_client->onError("Не удалось преобразовать метаданные для файла " + fileId + ": " + e.what());
        return;
    }

    if (total_parts == 0) {
        m_client->onError("Количество частей для файла " + fileId + " равно нулю");
        return;
    }

    // Собираем части файла с узлов
    for (const auto& node : active_nodes) {
        json request;
        request["type"] = "download";
        request["file_id"] = fileId;
        std::string response = sendData(node, request.dump());
        if (response.empty()) {
            std::cerr << "Не удалось скачать файл " << fileId << " с узла " << node << "\n";
            continue;
        }

        // Разбираем ответ с частями
        try {
            std::cout << "Received download response from " << node << ": " << response.size() << " bytes\n";
            json parts_json = json::parse(response);
            if (!parts_json.is_array()) {
                m_client->onError("Некорректный ответ скачивания с узла " + node + ": ожидался массив");
                continue;
            }

            for (const auto& part : parts_json) {
                if (!part.contains("content") || !part.contains("part_index") || !part.contains("hash")) {
                    m_client->onError("Отсутствуют обязательные поля в части файла от узла " + node);
                    continue;
                }

                size_t part_index = part["part_index"].get<size_t>();
                if (collected_parts.find(part_index) != collected_parts.end()) {
                    std::cout << "Part " << part_index << " already collected from another node, skipping\n";
                    continue; // Пропускаем, если часть уже собрана
                }

                // Декодируем содержимое части
                std::string encoded_content = part["content"].get<std::string>();
                std::string content = base64_decode(encoded_content);
                std::string expected_hash = part["hash"].get<std::string>();
                std::string computed_hash = calculateHash(content);

                // Проверяем целостность части
                if (computed_hash != expected_hash) {
                    m_client->onError("Хеш части " + std::to_string(part_index) + " не совпадает: ожидался " +
                                      expected_hash + ", получен " + computed_hash);
                    continue;
                }

                // Сохраняем часть во временный файл
                std::string temp_part_path = "temp_part_" + fileId + "_" + std::to_string(part_index);
                std::ofstream temp_file(temp_part_path, std::ios::binary);
                if (!temp_file) {
                    m_client->onError("Не удалось создать временный файл для части: " + temp_part_path);
                    return;
                }
                temp_file.write(content.data(), content.size());
                temp_file.close();

                std::cout << "Collected part " << part_index << " from " << node << ", size: " << content.size() << " bytes\n";
                collected_parts[part_index] = FilePart{
                    fileId,
                    part_index,
                    temp_part_path,
                    expected_hash
                };
            }
        } catch (const std::exception& e) {
            m_client->onError("Не удалось разобрать ответ скачивания с узла " + node + ": " + e.what());
            continue;
        }
    }

    // Проверяем, собрали ли все части
    std::cout << "Collected " << collected_parts.size() << " of " << total_parts << " parts for file " << fileId << "\n";
    if (collected_parts.size() != total_parts) {
        std::string error = "Не удалось собрать все части файла " + fileId + ": собрано " +
                           std::to_string(collected_parts.size()) + ", ожидалось " + std::to_string(total_parts);
        m_client->onError(error);
        // Удаляем временные файлы
        for (const auto& part : collected_parts) {
            fs::remove(part.second.file_path);
        }
        return;
    }

    // Преобразуем map в vector для объединения
    std::vector<FilePart> file_parts;
    for (const auto& pair : collected_parts) {
        file_parts.push_back(pair.second);
        m_client->onDownloadProgress(fileId, pair.first + 1, total_parts);
    }

    // Сортируем части по индексу
    std::sort(file_parts.begin(), file_parts.end(),
              [](const FilePart& a, const FilePart& b) { return a.part_index < b.part_index; });

    // Объединяем части в итоговый файл
    Storage storage;
    if (!storage.mergeFile(file_parts, outputPath, metadata)) {
        m_client->onError("Не удалось объединить части файла " + fileId);
        // Удаляем временные файлы
        for (const auto& part : file_parts) {
            fs::remove(part.file_path);
        }
        return;
    }

    // Удаляем временные файлы
    for (const auto& part : file_parts) {
        fs::remove(part.file_path);
    }

    std::cout << "File " << fileId << " successfully restored to " << outputPath << "\n";
}