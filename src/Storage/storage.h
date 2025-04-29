#ifndef STORAGE_H
#define STORAGE_H

#pragma once
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


class Storage
{
public:
    //Storage();
    //Storage(/*const std::string& filePath, const int numberOfParts*/) {} // , size_t partSizeInBytes)
    /**
      * @brief Деление файла на части
      * @param filePath Путь к файлу
      * @param numberOfParts Число частей
      * @return std::vector<FilePart> Возвращаем вектор структур с заполненными данными деления
    */
    static std::string splitFile(const std::string& filePath,
                                 const std::string& tempDir,
                                 const size_t chunkSize);
    /**
      * @brief Деление файла на части c дефолтными параметрами
      * @param filePath Путь к файлу
      * @return std::vector<FilePart> Возвращаем вектор структур с заполненными данными деления
    */
    static std::string splitFile(const std::string& filePath);

    /**
      * @brief Объединение кусков файла в единый
      * @param std::vector<FilePart> Вектор со всей информация о частях
      * @patam string& outputPath Путь для скачивания объединенного файла
    */
    static void mergeFile(const std::string& tempDir,
                          const std::string& outDir,
                          const size_t chunkSize);

    /**
      * @brief Перераспределение частей файлов
      * @param std::vector<FilePart> Вектор со всей информация о имеющихся частях файла старой конфигурации
      * @patam int newNumbersOfParts Новое колличество частей(узлов)
      * @return vector<FilePart> Возвращаем вектор с актуальными разделениями
    */

    /// идет в директорию, читает каждый файл и преобразует его содержимое в строку
    /// наверное проще вернуть вектор, но не любой файл засунешь в оперативку
    static std::string uploadData(const std::string& splitFileDir);
    /// получает строки создает из них json
    static std::string reciveData(const std::string& tempDir,
                                  const std::string& data);

private:
    /**
     * @brief Проверяет, достаточно ли места на диске для разделения файла.
     * @param fileSize Размер файла в байтах.
     * @return True, если на диске достаточно места; иначе False.
     */
    static bool hasEnoughSpace(size_t fileSize, std::string tempDir);
    /**
     * @brief Очищает имя файла от недопустимых символов (например, пробелов).
     * @param filename Имя файла для очистки.
     * @return Очищенное имя файла.
     */
    static std::string sanitizeFilename(const std::string& filename); // Обработка пробелов и спецсимволов
    ///
    static void createFileInfoJson(const boost::filesystem::path& file_name,
                                   size_t file_size,
                                   size_t file_hash,
                                   const std::string& file_type,
                                   size_t chunk_size,
                                   size_t chunk_count,
                                   const boost::filesystem::path& outputDir);
    ///
    static void createChunkJson(const std::string& file_name,
                                const std::string& chunk_name,
                                int chunk_number,
                                size_t chunk_size,
                                const std::string& data,
                                const boost::filesystem::path& output_file);

    /// пока не придумал этому применение
    static void changeJsonFile();
    /// создает JSON со списком файлов
    static void createFilesListJson();
    /// чистит директорию
    static void clearDirectory(const boost::filesystem::path& dirPath);
    /// может пригодится при сборке файла или тоже вычислить его в merge
    static size_t calculateFileHash(const boost::filesystem::path& filePath, const size_t& chunkSize);
    /// преобразует бинарные данные в строку
    static std::string base64_encode(const char* data, size_t size);
};

#endif // STORAGE_H


// int numberOfParts; // Число частей на которые нужно разделить фаил
// std::string& filePath; // Путь файла

// struct FilePart {
//     // char* firstBytes;
//     size_t partSize;
//     size_t partNumber;
//     unsigned long filePratHash;
//     unsigned long fileHash;
//     std::string& hashFilePath;
//     std::string& hashFilePartPath;
// };
// /*
//     Эту структуру я создал для реализации метода объединения частей файла, этот момент надо будет обсудить.
//     Использовал вектор vector<filePart2> fileParts первая ячейка которого занята информацие о файле (изначальное название файла)
//     В остальных ячейках будет содержаться информация о частях файла (имя и путь)

// */
// struct filePart2 {
//     std::string fileName;
//     std::string partFileName;
//     std::string partFilePath;
// };


//
// static std::vector<FilePart> splitFile(const std::string& filePath, const int numberOfParts);
//
//
//
//
//
// static void mergeFile(const std::vector<filePart2> fileParts, const std::string& tempDir, const std::string& outputPath);
//
//
//
//
//
//
// static std::vector<FilePart> reconfiguration(const std::vector<FilePart>, const int newNumbersOfParts);







