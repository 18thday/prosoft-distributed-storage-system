#ifndef STORAGE_H
#define STORAGE_H

#include "types.h"
#include <string>
#include <vector>

// Класс для работы с файлами
class Storage {
public:
    // Разделяет файл на части
    static std::vector<FilePart> splitFile(const std::string& filePath, FileMetaData& metadata, size_t part_count);
    // Объединяет части файла
    static bool mergeFile(const std::vector<FilePart>& parts, const std::string& outputPath, const FileMetaData& metadata);
    // Вычисляет хеш
    static std::string calculateHash(const std::string& data);
};

#endif