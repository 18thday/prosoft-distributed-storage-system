#include "storage.h"
#include <openssl/sha.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>

// Подключаем библиотеки для работы с файлами и хешированием
namespace fs = std::filesystem;

// Разделяет файл на части
std::vector<FilePart> Storage::splitFile(const std::string& filePath, FileMetaData& metadata, size_t nodesCount) {
    std::ifstream in(filePath, std::ios::binary);
    if (!in) {
        std::cerr << "Failed to open file: " << filePath << "\n";
        return {};
    }

    // Читаем весь файл в буфер
    std::vector<char> buffer(fs::file_size(filePath));
    in.read(buffer.data(), buffer.size());
    in.close();
    std::string data(buffer.data(), buffer.size());

    // Заполняем метаданные файла
    metadata.name = fs::path(filePath).filename().string();
    metadata.creation_time = std::to_string(fs::last_write_time(filePath).time_since_epoch().count());
    metadata.size = buffer.size();
    metadata.hash = calculateHash(data);
    metadata.part_count = nodesCount;
    metadata.part_hashes.clear();

    std::vector<FilePart> parts;
    size_t part_size = (buffer.size() + nodesCount - 1) / nodesCount;
    // Разделяем файл на части
    for (size_t i = 0; i < nodesCount; ++i) {
        size_t start = i * part_size;
        size_t end = std::min(start + part_size, buffer.size());
        if (start >= buffer.size()) break;

        std::string part_data(buffer.data() + start, end - start);
        std::string part_hash = calculateHash(part_data);

        // Сохраняем часть во временный файл
        std::string part_path = "parts/part_" + std::to_string(i);
        fs::create_directories(fs::path(part_path).parent_path());
        std::ofstream out(part_path, std::ios::binary);
        if (!out) {
            std::cerr << "Failed to create part file: " << part_path << "\n";
            return {};
        }
        out.write(part_data.data(), part_data.size());
        out.close();

        parts.push_back({metadata.name, i, part_path, part_hash});
        metadata.part_hashes.push_back(part_hash);
    }
    return parts;
}

// Объединяет части файла
bool Storage::mergeFile(const std::vector<FilePart>& parts, const std::string& outputPath, const FileMetaData& metadata) {
    if (parts.empty()) {
        std::cerr << "No parts provided for merging\n";
        return false;
    }

    // Проверяем хеши частей
    for (const auto& part : parts) {
        std::ifstream part_file(part.file_path, std::ios::binary);
        if (!part_file) {
            std::cerr << "Failed to open part file: " << part.file_path << "\n";
            return false;
        }
        std::vector<char> buffer(fs::file_size(part.file_path));
        part_file.read(buffer.data(), buffer.size());
        part_file.close();
        std::string data(buffer.data(), buffer.size());
        std::string computed_hash = calculateHash(data);
        if (computed_hash != part.hash) {
            std::cerr << "Hash mismatch for part " << part.part_index << ": expected " << part.hash
                      << ", got " << computed_hash << "\n";
            return false;
        }
    }

    // Сортируем части по индексу
    std::vector<FilePart> sorted_parts = parts;
    std::sort(sorted_parts.begin(), sorted_parts.end(),
              [](const FilePart& a, const FilePart& b) { return a.part_index < b.part_index; });

    // Собираем файл
    std::ofstream out(outputPath, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open output file: " << outputPath << "\n";
        return false;
    }

    std::string final_data;
    for (const auto& part : sorted_parts) {
        std::ifstream part_file(part.file_path, std::ios::binary);
        if (!part_file) {
            std::cerr << "Failed to open part file: " << part.file_path << "\n";
            out.close();
            fs::remove(outputPath);
            return false;
        }
        std::vector<char> buffer(fs::file_size(part.file_path));
        part_file.read(buffer.data(), buffer.size());
        final_data.append(buffer.data(), buffer.size());
        out.write(buffer.data(), buffer.size());
        part_file.close();
    }
    out.close();

    // Проверяем хеш итогового файла
    std::string final_hash = calculateHash(final_data);
    std::cout << "Computed file hash in mergeFile: " << final_hash << ", expected: " << metadata.hash << "\n";
    if (final_hash != metadata.hash) {
        std::cerr << "Final file hash mismatch: expected " << metadata.hash << ", got " << final_hash << "\n";
        fs::remove(outputPath);
        return false;
    }

    std::cout << "File merged successfully: " << outputPath << "\n";
    return true;
}

// Вычисляет SHA-256 хеш
std::string Storage::calculateHash(const std::string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(data.data()), data.size(), hash);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}