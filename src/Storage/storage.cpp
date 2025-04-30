/*
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
}*/

#include "storage.h"
#include <fstream>
#include <stdexcept> // Для std::runtime_error
#include <cmath>
#include <memory>

namespace fs = boost::filesystem;

std::string Storage::splitFile(const std::string& filePath,
                               const std::string& tempDir,
                               const size_t chunkSize)
{
    fs::path inputFile(filePath);
    if (!fs::exists(inputFile))
    {
        throw std::runtime_error("File not found: " + filePath);
    }

    fs::path tempDir_(tempDir);
    if (!fs::exists(tempDir_) || !fs::is_directory(tempDir_))
    {
        throw std::runtime_error("Temporary directory is not valid: " + tempDir);
    }

    // std::string sanitizedFilename = sanitizeFilename(inputFile.filename().string());
    fs::path chunkDir = tempDir + "/" + inputFile.stem().string() + "_chunks";
    if (!fs::exists(chunkDir))
    {
        fs::create_directories(chunkDir);
    } else if (fs::exists(chunkDir)  && fs::is_directory(chunkDir))
    {
        clearDirectory(chunkDir);
        fs::create_directories(chunkDir);
    } else return ""; // решил перестраховаться возможно излишне


    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile.is_open()) {
        throw std::runtime_error("Could not open input file: " + filePath);
    }

    size_t fileSize = fs::file_size(inputFile);
    // if (!hasEnoughSpace(fileSize, tempDir)) {
    //     throw std::runtime_error("Not enough space on disk to split the file.");
    // }
    size_t numChunks = static_cast<size_t>(std::ceil(static_cast<double>(fileSize) / chunkSize)); // Округление вверх

    /// дублирую здесь вычисление хэша чтобы не читать файл 2 раза
    size_t hash = 0;

    /// основная логика
    for (size_t i = 0; i < numChunks; ++i)
    {
        std::string chunkFilename = inputFile.stem().string() + "_" + std::to_string(i) + ".part";

        boost::filesystem::path chunkPath = chunkDir / chunkFilename;

        std::ofstream outFile(chunkPath.string(), std::ios::binary);
        if (!outFile.is_open()) {
            throw std::runtime_error("Could not create output file: " + chunkPath.string());
        }

        std::unique_ptr<char[]> buffer(new char[chunkSize]);

        /// самое главное - чтение бинарными кусками
        inFile.read(buffer.get(), chunkSize);
        outFile.write(buffer.get(), inFile.gcount());

        outFile.close();
        /// преобразуем прочитанное в формат для json
        //std::string encodedChunk = base64_encode(buffer.get(), inFile.gcount());

        boost::hash_combine(hash, std::string(buffer.get(), inFile.gcount()));

        /// запись CHUNK в json
        // createChunkJson(inputFile,
        //                 chunkFilename,
        //                 i,
        //                 inFile.gcount(),
        //                 encodedChunk,
        //                 chunkPath);
    }

    /// создаем FileInfo
    createFileInfoJson(inputFile,
                       fileSize,
                       hash,
                       inputFile.extension().string(),
                       chunkSize,
                       numChunks,
                       chunkDir);
    inFile.close();
    return chunkDir.string();
}
/// перегруженный метод с дефолтными параметрами
std::string Storage::splitFile(const std::string& filePath)
{
    std::string chunkDir = splitFile(filePath, fs::temp_directory_path().string(), 4096);
    return chunkDir;
}

void mergeFile(const std::string& tempDir,
               const std::string& outDir,
               const size_t chunkSize)
{
    /// первая реалиизация этого метода лежит закоментированная внизу
}


/// Проверяет, достаточно ли места на диске
bool Storage::hasEnoughSpace(size_t fileSize, std::string tempDir)
{
    // Проверяем, достаточно ли места на диске
    boost::filesystem::space_info spaceInfo = boost::filesystem::space(tempDir);
    return spaceInfo.available >= fileSize;
}

void Storage::clearDirectory(const fs::path& dirPath)
{
    fs::remove_all(dirPath);
}

size_t Storage::calculateFileHash(const fs::path& filePath, const size_t& chunkSize)
{
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile.is_open()) {
        throw std::runtime_error("Error opening file: " + filePath.string()); // Обработка ошибки открытия файла
    }

    size_t hash = 0;
    std::unique_ptr<char[]> buffer(new char[chunkSize]);

    while (inFile.read(buffer.get(), chunkSize)) {
        size_t bytesRead = inFile.gcount();
        boost::hash_combine(hash, std::vector<char>(buffer.get(), buffer.get() + bytesRead));
    }

    inFile.close();
    return hash;
}

std::string Storage::base64_encode(const char* data, size_t size) {
    using namespace boost::archive::iterators;

    // Кодируем данные в Base64
    std::string encoded = std::string(
        base64_from_binary<transform_width<const char*, 6, 8>>(data),
        base64_from_binary<transform_width<const char*, 6, 8>>(data + size)
        );

    return encoded;
}

void Storage::createChunkJson(const fs::path& inputFile,
                              const std::string& chunk_name,
                              size_t chunk_number,
                              size_t chunk_size,
                              const std::string& data,
                              const fs::path& output_file)
{
    // Создаем дерево для JSON
    boost::property_tree::ptree root;
    boost::property_tree::ptree chunk;

    // Заполняем данные о части
    boost::property_tree::ptree chunk_data;
    chunk_data.put("file_name", inputFile.filename().string());
    chunk_data.put("chunk_name", chunk_name);
    chunk_data.put("chunk_number", chunk_number);
    chunk_data.put("chunk_size", chunk_size);
    chunk_data.put("encoded_data", data);

    // Добавляем часть в массив
    chunk.push_back(std::make_pair("", chunk_data));

    // Добавляем массив частей в корень
    root.add_child("chunk", chunk);

    // Записываем JSON в файл
    boost::property_tree::write_json(output_file.string(), root);
}

void Storage::createFileInfoJson(const fs::path& inputFile,
                                 size_t file_size,
                                 size_t file_hash,
                                 const std::string& file_type,
                                 size_t chunk_size,
                                 size_t chunk_count,
                                 const fs::path& outputDir)
{
    // Создаем дерево для JSON
    boost::property_tree::ptree root;
    //boost::property_tree::ptree file_info;

    // Заполняем данные о файле
    boost::property_tree::ptree file_data;
    file_data.put("file_name", inputFile.stem().string());
    file_data.put("file_size", file_size);
    file_data.put("file_hash", file_hash);
    file_data.put("file_type", file_type);
    file_data.put("chunk_size", chunk_size);
    file_data.put("chunk_count", chunk_count);
    file_data.put("creation_date", "");
    file_data.put("last_modified_date", "");

    // Добавляем часть в массив
    //file_info.push_back(std::make_pair("", file_data));

    // Добавляем массив частей в корень
    root.add_child("file_info", file_data);

    std::string outFile = outputDir.string() + "/" + inputFile.stem().string() + ".info";

    // Записываем JSON в файл
    boost::property_tree::write_json(outFile, root);
}

struct uploadState Storage::currentState = {-1, 0};

FileData Storage::uploadData(const std::string& splitFileDir,
                             std::unordered_set<std::string> ipList)
{
    fs::path dir(splitFileDir);
    fs::path fileInfoJson;
    std::vector<chunkGroup> chunkGroups;
    FileData fileData = {};

    if (currentState.lastChunkid < 0)
    {
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".info") {
                fileInfoJson = entry;
            }
        }
        currentState.totalChunks = std::stoi(getInfo(fileInfoJson.string(), "chunk_count"));
        chunkGroups = groupChunksByNode(currentState.totalChunks, ipList);
    }

    fs::path chunkPath;
    for (const auto& entry : fs::directory_iterator(dir))
    {
        if  (entry.is_regular_file() && entry.path().extension() == ".part")
        {

        }
    }


    //FileData fileData = chunkToStruct(chunkPath, chunkIP);
    if (currentState.lastChunkid > currentState.totalChunks)
    {
        currentState = {-1, 0};
        memset(fileData.ipAddr, 0, sizeof(fileData.ipAddr));
        memset(fileData.fileName, 0, sizeof(fileData.fileName));
        memset(fileData.data, 0, sizeof(fileData.data));
        fileData.dataSize = 0;
        return fileData;
    } else {
        return chunkToStruct();
    };
    return fileData;

    // currentState.totalChunks
}

FileData chunkToStruct(const fs::path& chunk,
                       const std::string& ipAddr)
{
    FileData fileData;
    fileData.emptyByte = 0;
    std::ifstream inFile(chunk.string(), std::ios::binary);
    if (!inFile.is_open())
    {
        //throw std::runtime_error("Ошибка: Не удалось открыть файл " + chunk.string());
        return fileData; // Возвращаем структуру с нулевыми значениями
    }

    size_t fileSize = chunk.size();
    if (fileSize > 32768)
    {
        return fileData;
    }
    std::unique_ptr<char[]> buffer(new char[32768]);
    inFile.read(buffer.get(), fileSize);
    size_t bytesRead = inFile.gcount();

    strncpy(fileData.ipAddr, ipAddr.c_str(), sizeof(fileData.ipAddr) - 1);
    fileData.ipAddr[sizeof(fileData.ipAddr) - 1] = '\0';

    std::string fileName = chunk.filename().stem().string();

    strncpy(fileData.fileName, fileName.c_str(), sizeof(fileData.fileName) - 1);
    fileData.fileName[sizeof(fileData.fileName) - 1] = '\0';


    memcpy(fileData.data, buffer.get(), bytesRead);
    fileData.dataSize = bytesRead;

    inFile.close();
    return fileData;
    //outFile.write(buffer.get(), inFile.gcount());
}

std::vector<chunkGroup> Storage::groupChunksByNode(size_t totalChunks,
                                                    std::unordered_set<std::string> ipList)
{
    std::vector<chunkGroup> chunkGroups;
    if (ipList.size() < 4)
    { //
        auto it = ipList.begin();
        int count = 0;
        for (auto it = ipList.begin(); it != ipList.end() && count < 2; ++it, ++count) {
            chunkGroup group;
            group.chunkIdBegin = 0;
            group.chunkIdEnd = totalChunks-1;
            group.ipAddr = *it;
            chunkGroups.push_back(group);
        }

    } else if (ipList.size() > 3){
        auto it = ipList.begin();
        int count = 0;
        int firstHalfEnd = std::ceil(static_cast<double>(totalChunks) / 2);
        for (auto it = ipList.begin(); it != ipList.end() && count < 4; ++it, ++count) {
            chunkGroup group;
            if (count < 3){
                group.chunkIdBegin = 0;
                group.chunkIdEnd = firstHalfEnd-1;
                group.ipAddr = *it;
                chunkGroups.push_back(group);
            } else {
                group.chunkIdBegin = firstHalfEnd;
                group.chunkIdEnd = totalChunks-1;
                group.ipAddr = *it;
                chunkGroups.push_back(group);
            }
        }
    };
}


std::string Storage::getInfo(const std::string& file_info_path, const std::string& key)
{
    using namespace boost::property_tree;
    /*
     * Чтение файла .info и получение из него нужных данных по ключу
     */
    ptree read_file_info_path;
    read_json(file_info_path, read_file_info_path);
    auto parent = read_file_info_path.get_child("file_info");
    std::string info = parent.get<std::string>(key);
    return info;
}
