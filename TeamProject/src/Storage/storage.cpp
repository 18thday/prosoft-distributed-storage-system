#include "storage.h"
#include <fstream>
#include <stdexcept> // Для std::runtime_error
#include <cmath>
#include <memory>

namespace fs = boost::filesystem;
namespace pt = boost::property_tree;

struct uploadState Storage::currentState = {-1};
FileData Storage::uploadData(const std::string& tempDir,
                             const pt::ptree& fileInfo,
                             const std::string& clientIP = "")
{
    FileData fileData = {};

    int secondHalfBegin;
    fs::path chunkPath;

    if (currentState.lastChunkid < 0)
    {
        pt::ptree file_info;
        try {
            file_info = fileInfo.get_child("file_info");
        } catch (const pt::ptree_error e) {
            file_info = fileInfo;
        }
        currentState.totalChunks = std::stoi(file_info.get<std::string>("chunk_count"));
        //currentState.totalChunks = file_info.get<size_t>("chunk_count");
        currentState.fileName = file_info.get<std::string>("file_name");
        currentState.ipList.push_back(file_info.get<std::string>("first_half_ip1"));
        currentState.ipList.push_back(file_info.get<std::string>("first_half_ip2"));
        currentState.ipList.push_back(file_info.get<std::string>("second_half_ip1"));
        currentState.ipList.push_back(file_info.get<std::string>("second_half_ip2"));
        /// проверяем кто мы клиент или узел
        if (clientIP.empty())
        {
            currentState.lastChunkid = 0;
        }

        /// выясняем с какого чанка начинается вторая половина файла
        secondHalfBegin = static_cast<int>(std::ceil(static_cast<double>(currentState.totalChunks) / 2));

        /// проверяем существует ли на узле чанк из второй половины
        fs::path chunkPathAfterHalf = chunksDir / (fileName + "_" + std::to_string(secondHalfBegin) + ".part");
        if (fs::exists(chunkPathAfterHalf))
        {
            currentState.lastChunkid = secondHalfBegin;
        } else {
            currentState.lastChunkid = 0;
        }
    }

    fs::path chunksDir = tempDir + "/PSDSSstorage/" + fileName + "_chunks";
    if (!fs::exists(chunksDir) || !fs::is_directory(chunksDir))
    {
        std::cerr << "Сouldn't find the directory: " << chunksDir.string() << std::endl;
        currentState = {-1};
        return fileData;
    }

    chunkPath = chunksDir / (fileName + "_" + std::to_string(currentState.lastChunkid) + ".part");
    if (!fs::exists(chunkPath))
    {
        currentState = {-1};
        return fileData;
    }

    bool isFirstHalf = currentState.lastChunkid < secondHalfBegin;

    if (isFirstHalf) { /// для первой половины чанков
        if (!currentState.sendToIp2) { /// из-за этого если у нас 1 узел, то файл все равно будет пересылаться в двойном объеме
            currentState.sendToIp2 = true;
            currentState.lastChunkid++;
            return chunkToStruct(chunkPath, currentState.ipList[1]);
        }
        return chunkToStruct(chunkPath, currentState.ipList[0]);
    } else { /// для второй половины чанков
        if (!currentState.sendToIp2) {
            currentState.sendToIp2 = true;
            currentState.lastChunkid++;
            return chunkToStruct(chunkPath, currentState.ipList[3]);
        }
        return chunkToStruct(chunkPath, currentState.ipList[3]);
    }
    currentState = {-1};
    return fileData;
}

std::string reciveData(const std::string& tempDir,
                       const FileData& data)
{
    std::string file_name(data.fileName);
    size_t lastUnderscorePos = file_name.find_last_of('_');
    fs::path chunksDir = tempDir + "/PDSSstorage/" + file_name.substr(0, lastUnderscorePos) + "_chunks";
    if (!fs::exists(chunksDir))
    {
        fs::create_directories(chunksDir);
    }
    std::string outFilePath = chunksDir.string() + "/" + file_name;

    std::ofstream outfile(outFilePath, std::ios::binary);
    if (!outfile.is_open()) {
        std::cerr << "Couldn't open the file for write: " << outFilePath << std::endl;
        return ""; // Возвращаем пустую строку в случае ошибки
    }

    outfile.write(reinterpret_cast<const char*>(data.data), data.dataSize);

    outfile.close();

    return chunksDir.string();
}

pt::ptree Storage::splitFile(const std::string& filePath,
                             const std::string& tempDir,
                             const size_t chunkSize,
                             std::unordered_set<std::string>& ipList)
{
    fs::path inputFile(filePath);
    pt::ptree fileInfoJson;
    if (!fs::exists(inputFile))
    {
        std::cerr << "File not found: " << filePath << std::endl;
        return fileInfoJson;
    }

    fs::path tempDir_(tempDir);
    if (!fs::exists(tempDir_) || !fs::is_directory(tempDir_))
    {
        std::cerr << "Temporary directory is not valid: " << tempDir << std::endl;
        return fileInfoJson;
    }

    fs::path chunkDir = tempDir + "/PDSSstorage/" + inputFile.stem().string() + "_chunks";
    if (!fs::exists(chunkDir))
    {
        fs::create_directories(chunkDir);
    } else if (fs::exists(chunkDir)  && fs::is_directory(chunkDir))
    {
        clearDirectory(chunkDir);
        fs::create_directories(chunkDir);
    } else return fileInfoJson; // решил перестраховаться возможно излишне


    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Could not open input file: " << tempDir << std::endl;
        return fileInfoJson;
    }

    size_t fileSize = fs::file_size(inputFile);
    // if (!hasEnoughSpace(fileSize, tempDir)) {
    //     throw std::runtime_error("Not enough space on disk to split the file.");
    // }
    size_t numChunks = static_cast<size_t>(std::ceil(static_cast<double>(fileSize) / chunkSize)); // Округление вверх
    std::cout << "The file size (byte): " << fileSize << std::endl;
    std::cout << "Chunks will be created: " << numChunks << std::endl;





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

        boost::hash_combine(hash, std::string(buffer.get(), inFile.gcount()));
    }


    chunkGroups distributionIpList = groupChunksByNode(numChunks, ipList);
    /// создаем FileInfo
    fileInfoJson = createFileInfoJson(inputFile,
                                        fileSize,
                                        hash,
                                        inputFile.extension().string(),
                                        chunkSize,
                                        numChunks,
                                        chunkDir,
                                        distributionIpList);
    inFile.close();
    return fileInfoJson;
}
/// перегруженный метод с дефолтными параметрами
// pt::ptree Storage::splitFile(const std::string& filePath)
// {
    // pt::ptree splitResult = splitFile(filePath, fs::temp_directory_path().string(), 4096);
    // return splitResult;
// }


// Объединяет части файла
bool Storage::mergeFile(const std::string& tempDir,
                        const std::string& outDir) {
    /*
 * Ищем в директории tempDir фаил с необходимым расширением .info
 */
    std::string file_info_path;
    for (const auto& entry : fs::directory_iterator(tempDir))
    {
        if (entry.path().filename().string().find(".info") != std::string::npos)
        {
            file_info_path = tempDir + '/' + entry.path().filename().string();
            break;
        }

    }
    /*
  * Достаем из файла .info всю необходимую информацию
  */
    std::string file_name = Storage::getInfo(file_info_path, "file_name");
    std::string file_type = Storage::getInfo(file_info_path, "file_type");
    int chunk_count = std::stoi(Storage::getInfo(file_info_path, "chunk_count"));
    size_t chunk_size = std::stoi(Storage::getInfo(file_info_path, "chunk_size"));
    std::string file_hash = Storage::getInfo(file_info_path, "file_hash");
    std::string filePath = outDir + "/" + file_name + file_type;


    // Создаем фаил с нужным названием в нужном месте, в бинарном режиме
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile)
    {
        throw std::runtime_error("Cannot open output file:" + outDir);
    }
    // Создаем буфер размера chunk_size
    size_t bufferSize = chunk_size;
    std::vector<char> buffer(bufferSize);
    /*
  * Получаем путь к файлам .part и удаляем фрагмент строки с порядковым номером и расширением "0.part"
  * Это необходимо чтобы далее в цикле перебирать все файлы по пордяку
  */
    std::string part_path;
    for (const auto& entry_part : fs::directory_iterator(tempDir))
    {
        if (entry_part.path().filename().string().find(".part") != std::string::npos)
        {
            part_path = tempDir + '/' + entry_part.path().filename().string();
            break;
        }

    }
    /*
 * Удаление порядкрого номера и расширения "0.part" из пути файла
 */
    size_t pos;
    std::string part_type_delete = "0.part";
    while ((pos = part_path.find(part_type_delete)) != std::string::npos) {
        part_path.erase(pos, part_type_delete.length());
    }

    //Обрабатываем каждый входной фаил
    for (size_t i = 0; i < chunk_count; i++)
    {

        //Открываем файл в бинарном режиме
        std::ifstream inFile((part_path + std::to_string(i) + ".part"), std::ios::binary);
        if (!inFile)
        {
            throw std::runtime_error("Cannot open input file: " + (part_path + std::to_string(i) + ".part"));
        }

        //Читаем фаил блоками
        while (inFile.read(buffer.data(), buffer.size()))
        {
            // Записываем в коенчный фаил прочитанный блок
            outFile.write(buffer.data(), inFile.gcount());
        }

        //Записываем оставшиеся данные(меньше размера буфера)
        outFile.write(buffer.data(), inFile.gcount());
        inFile.close();
    }
    outFile.close();
    std::string merge_file_hash = calculateHash(filePath);
    std::cout << "Computed file hash in mergeFile: " << merge_file_hash << ", expected: " << file_hash << "\n";
    if (merge_file_hash != file_hash)
    {
        std::cerr << "Final file hash mismatch: expected " << file_hash << ", got " << merge_file_hash << "\n";
        fs::remove(outDir);
        return false;
    }
    std::cout << "File merged successfully: " << outputPath << "\n";
    return true;
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

pt::ptree Storage::createFileInfoJson(const fs::path& inputFile,
                                 size_t file_size,
                                 size_t file_hash,
                                 const std::string& file_type,
                                 size_t chunk_size,
                                 size_t chunk_count,
                                 const fs::path& outputDir,
                                 const chunkGroups& iplist)
{
    // Создаем дерево для JSON
    pt::ptree root;
    //boost::property_tree::ptree file_info;

    // Заполняем данные о файле
    boost::property_tree::ptree file_data;
    file_data.put("file_name", inputFile.stem().string());
    file_data.put("file_size", file_size);
    //file_data.put_value("file_size", file_size);
    file_data.put("file_hash", file_hash);
    file_data.put("file_type", file_type);
    file_data.put("chunk_size", chunk_size);
    file_data.put("chunk_count", chunk_count);
    file_data.put("creation_date", "");
    file_data.put("last_modified_date", "");
    file_data.put_value("first_half_ip1", "");
    file_data.put_value("first_half_ip2", "");
    file_data.put_value("second_half_ip1", "");
    file_data.put_value("second_half_ip2", "");

    // Добавляем часть в массив
    //file_info.push_back(std::make_pair("", file_data));

    // Добавляем массив частей в корень
    root.add_child("file_info", file_data);

    std::string outFile = outputDir.string() + "/" + inputFile.stem().string() + ".info";

    // Записываем JSON в файл
    boost::property_tree::write_json(outFile, root);
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
}

chunkGroups Storage::groupChunksByNode(std::unordered_set<std::string>& ipList)
{
    chunkGroups chunkGroups;
    //std::string ip1, ip2; // заглушка
    std::vector<std::string> ipVector;
    
    int count = 0;
    for(auto& ipAddr : ipList)
    {
        if (count >= 4) break;
        if (count >= 2 && ipList.size() < 4) break;
        ipVector.push_back(ipAddr);
        count++;
    }
    if (ipVector.size() == 1)
    { // ipAddr 1, 2 - для первой половины файла
      // ipAddr 3, 4 - для второй половины файла
        chunkGroups.ipAddr1 = ipVector[0];
        chunkGroups.ipAddr2 = ipVector[0];
        chunkGroups.ipAddr3 = ipVector[0];
        chunkGroups.ipAddr4 = ipVector[0];
    } else if (ipVector.size() == 2) {
        chunkGroups.ipAddr1 = ipVector[0];
        chunkGroups.ipAddr2 = ipVector[1];
        chunkGroups.ipAddr3 = ipVector[0];
        chunkGroups.ipAddr4 = ipVector[1];
    } else if (ipVector.size() == 4) {
        chunkGroups.ipAddr1 = ipVector[0];
        chunkGroups.ipAddr2 = ipVector[1];
        chunkGroups.ipAddr3 = ipVector[2];
        chunkGroups.ipAddr4 = ipVector[3];
    } else return chunkGroups;

    return chunkGroups;
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
