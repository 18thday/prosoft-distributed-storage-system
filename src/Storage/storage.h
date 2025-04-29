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

/**
 * @class Storage
 * @brief Класс для работы с файлами, будетиспользоваться и клиентом и узлами.
 */
class Storage
{
public:
    //Storage();
    //Storage(/*const std::string& filePath, const int numberOfParts*/) {} // , size_t partSizeInBytes)
    /**
     * @brief Делит файл на части заданного размера.
     * @param filePath Путь к исходному файлу.
     * @param tempDir Директория для временного хранения частей.
     * @param chunkSize Размер каждой части в байтах.
     * @return std::string Путь к директории с частями файлов.
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
    

    
    /**
     * @brief Загружает данные из файлов в указанной директории.
     * @param splitFileDir Директория с частями файлов.
     * @return std::string Содержимое каждого отдельного файла в виде строки.
     */
    static std::string uploadData(const std::string& splitFileDir);/// наверное проще вернуть вектор, но не любой файл засунешь в оперативку
    /**
     * @brief Создает JSON из строковых данных.
     * @param tempDir Директория для временного хранения данных.
     * @param data Строка с данными для преобразования в JSON.
     * @return std::string JSON-строка.
     */
    static std::string reciveData(const std::string& tempDir,
                                  const std::string& data);

private:
    /**
     * @brief Проверяет, достаточно ли места на диске для разделения файла. Сейчас не используется.
     * @param fileSize Размер файла в байтах.
     * @return True, если на диске достаточно места; иначе False.
     */
    static bool hasEnoughSpace(size_t fileSize, std::string tempDir);
    /**
     * @brief Очищает имя файла от недопустимых символов (например, пробелов). Сейчас не используется.
     * @param filename Имя файла для очистки.
     * @return Очищенное имя файла.
     */
    static std::string sanitizeFilename(const std::string& filename); // Обработка пробелов и спецсимволов
    /**
     * @brief Создает JSON с информацией о файле.
     * @param file_name Путь к файлу.
     * @param file_size Размер файла.
     * @param file_hash Хэш файла.
     * @param file_type Тип файла.
     * @param chunk_size Размер части.
     * @param chunk_count Количество частей.
     * @param outputDir Директория для сохранения JSON.
     */
    static void createFileInfoJson(const boost::filesystem::path& file_name,
                                   size_t file_size,
                                   size_t file_hash,
                                   const std::string& file_type,
                                   size_t chunk_size,
                                   size_t chunk_count,
                                   const boost::filesystem::path& outputDir);
    /**
     * @brief Создает JSON для отдельной части файла, с именем: filename_chunkX.part.
     * @param file_name Имя исходного файла.
     * @param chunk_name Имя части файла.
     * @param chunk_number Номер части.
     * @param chunk_size Размер части.
     * @param data Строка с бинарными данными в формате base64 части файла.
     * @param output_file Путь для сохранения JSON.
     */
    static void createChunkJson(const std::string& file_name,
                                const std::string& chunk_name,
                                int chunk_number,
                                size_t chunk_size,
                                const std::string& data,
                                const boost::filesystem::path& output_file);

    /**
     * @brief Изменяет JSON файл (функция в разработке).
     */
    static void changeJsonFile();
    /**
     * @brief Создает JSON со списком файлов в директории.
     */
    static void createFilesListJson();
    /**
     * @brief Очищает указанную директорию, dirPath тоже удаляется.
     * @param dirPath Путь к директории для очистки.
     */
    static void clearDirectory(const boost::filesystem::path& dirPath);
    /**
     * @brief Вычисляет хэш файла.
     * @param filePath Путь к файлу.
     * @param chunkSize Размер части.
     * @return size_t Хэш файла.
     */
    static size_t calculateFileHash(const boost::filesystem::path& filePath, const size_t& chunkSize);/// может пригодится при сборке файла или тоже вычислить его в merge
    /**
     * @brief Кодирует бинарные данные в строку в формате base64.
     * @param data Указатель на данные.
     * @param size Размер данных.
     * @return std::string Закодированные данные в формате base64.
     */
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







