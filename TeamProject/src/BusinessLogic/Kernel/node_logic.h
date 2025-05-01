#ifndef NODE_LOGIC_H
#define NODE_LOGIC_H

#include "types.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using NodeIpAddress = std::string;

// Класс логики узла
class NodeLogic {
public:
    explicit NodeLogic(const NodeIpAddress& address);

    // Основные операции
    void start(); // Запуск узла
    void stop(); // Остановка узла
    bool check_if_node_alive(const NodeIpAddress& address); // Проверка доступности узла
    bool check_if_cluster_consistent(); // Проверка целостности кластера
    void calculate_new_file_parts_map(const std::string& file, const FileMetaData& metadata); // Распределение частей
    void alert_node_that_reconfiguration_needed(const NodeIpAddress& address); // Уведомление о реконфигурации
    void alert_cluster_that_reconfiguration_needed(); // Уведомление кластера
    void add_file_part(const FilePart& part, const std::string& content); // Добавление части файла
    std::vector<FilePart> get_file_parts(const std::string& file); // Получение частей файла
    void remove_file(const std::string& file); // Удаление файла
    json get_file_list(); // Получение списка файлов
    void sync_metadata(const json& metadata); // Синхронизация метаданных
    size_t get_storage_usage(); // Получение объема хранилища
    bool check_storage_threshold(); // Проверка лимита хранилища
    void add_new_node(); // Добавление нового узла

private:
    // Вспомогательные методы
    void load_nodes_from_config(); // Загрузка конфигурации
    void save_nodes_to_config(); // Сохранение конфигурации

    StatusEnum status_; // Статус узла
    NodeIpAddress my_address_; // Адрес текущего узла
    std::vector<NodeIpAddress> cluster_nodes_; // Список узлов кластера
    std::unordered_map<std::string, FileMetaData> files_metadata_; // Метаданные файлов
    std::unordered_map<NodeIpAddress, std::vector<FilePart>> file_parts_map_; // Карта частей файлов
};

#endif