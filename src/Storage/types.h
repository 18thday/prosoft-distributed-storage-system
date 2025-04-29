#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>

using NodeIpAddress = std::string;

// Перечисление статусов узла
enum class StatusEnum {
    stopped,           // Узел остановлен
    working,           // Узел работает
    ready_to_reconfigure, // Готов к реконфигурации
    reconfiguring      // В процессе реконфигурации
};

// Структура для части файла
struct FilePart {
    std::string file_name; // Имя файла
    size_t part_index;     // Индекс части
    std::string file_path; // Путь к файлу части
    std::string hash;      // Хеш части
};

// Структура для метаданных файла
struct FileMetaData {
    std::string name;           // Имя файла
    std::string creation_time;  // Время создания
    std::string hash;           // Хеш файла
    size_t size;                // Размер файла
    size_t part_count;          // Количество частей
    std::vector<std::string> part_hashes; // Хеши частей
};

#endif