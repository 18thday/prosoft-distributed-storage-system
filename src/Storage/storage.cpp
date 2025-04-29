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
    } else return nullptr; // решил перестраховаться возможно излишне


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
        std::string chunkFilename = inputFile.stem().string() + "_" + std::to_string(i);
        chunkFilename += ".part";  // Добавляем расширение, чтобы было понятно, что это часть файла

        boost::filesystem::path chunkPath = chunkDir / chunkFilename;

        std::unique_ptr<char[]> buffer(new char[chunkSize]);

        /// самое главное чтение бинарными кусками
        inFile.read(buffer.get(), chunkSize);
        /// преобразуем прочитанное в формат для json
        std::string encodedChunk = base64_encode(buffer.get(), inFile.gcount());

        boost::hash_combine(hash, std::string(buffer.get(), inFile.gcount()));

        /// запись CHUNK в json
        createChunkJson(inputFile.filename().string(),
                        chunkFilename,
                        i,
                        inFile.gcount(),
                        encodedChunk,
                        chunkPath);
    }

    /// создаем FileInfo
    createFileInfoJson(inputFile.stem().string(),
                       fileSize,
                       hash,
                       inputFile.extension().string(),
                       chunkSize,
                       numChunks,
                       chunkDir);

    return chunkDir.string();
}
/// перегруженный метод с дефолтными параметрами
std::string Storage::splitFile(const std::string& filePath)
{
    std::string chunkDir = splitFile(filePath, fs::temp_directory_path().string(), 32768);
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

void clearDirectory(const fs::path& dirPath)
{
    fs::remove_all(dirPath);
}

size_t calculateFileHash(const fs::path& filePath, const size_t& chunkSize)
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

void Storage::createChunkJson(const std::string& file_name,
                              const std::string& chunk_name,
                              int chunk_number,
                              size_t chunk_size,
                              const std::string& data,
                              const fs::path& output_file)
{
    // Создаем дерево для JSON
    boost::property_tree::ptree root;
    boost::property_tree::ptree chunk;

    // Заполняем данные о части
    boost::property_tree::ptree chunk_data;
    chunk_data.put("file_name", file_name);
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

void Storage::createFileInfoJson(const fs::path& file_name,
                                 size_t file_size,
                                 size_t file_hash,
                                 const std::string& file_type,
                                 size_t chunk_size,
                                 size_t chunk_count,
                                 const fs::path& outputDir)
{
    // Создаем дерево для JSON
    boost::property_tree::ptree root;
    boost::property_tree::ptree file_info;

    // Заполняем данные о файле
    boost::property_tree::ptree file_data;
    file_data.put("file_name", file_name);
    file_data.put("file_size", file_size);
    file_data.put("file_hash", file_hash);
    file_data.put("file_type", file_type);
    file_data.put("chunk_size", chunk_size);
    file_data.put("chunk_count", chunk_count);
    file_data.put("creation_date", "");
    file_data.put("last_modified_date", "");

    // Добавляем часть в массив
    file_info.push_back(std::make_pair("", file_data));

    // Добавляем массив частей в корень
    root.add_child("file_info", file_info);

    std::string outFile = output_file.stem().string() + ".info";
    // Записываем JSON в файл
    boost::property_tree::write_json(outFile, root);
}
/*
"file_info": {
    "file_id": "unique_file_identifier",
    "file_name": "example.txt",
    "file_size": 102400,  // размер в байтах
    "file_hash": "abc123hashvalue",  // хеш-сумма файла для проверки целостности
    "creation_date": "2023-10-01T12:00:00Z",  // дата создания файла в формате ISO 8601
    "last_modified_date": "2023-10-10T12:00:00Z",  // дата последнего изменения
    "file_type": "text/plain",  // MIME-тип файла
    "chunk_size": 10240,  // размер каждой части в байтах


*/
/*
void Storage::mergeFile(const std::vector<filePart2> fileParts,
                        const std::string& tempDir,
                        const std::string& outputPath)
{
    Переменная содержащая путь где нужно создать фаил + название файла.
      Как возможная идея, хранить информацию о файле в перой ячейке вектора vector<filePart2>, но это еще надо обсудить.
      В остальных ячейках данные о кусочках. Также вектор должен быть отсортирован.
      В реализации этого метода попробовал использовать эту идею.
      Имя файла предлагаю сохранть при его делении

std::string filePath = outputPath + "/" + fileParts[0].fileName;
//Проверка, достаточно ли места для будущего файла (не тестировал)
size_t fileSize = 0;
for (int i = 0; i < fileParts.size(); i++)
{
    fileSize += boost::filesystem::file_size(fileParts[i].partFilePath);
}
if (!hasEnoughSpace(fileSize, tempDir)) {
    throw std::runtime_error("Not enough space on disk to split the file.");
}

// Создаем фаил с нужным названием в нужном месте, в бинарном режиме
std::ofstream outFile(filePath, std::ios::binary);
if (!outFile)
{
    throw std::runtime_error("Cannot open output file:" + outputPath);
}
// Создаем буфер размера 4096
size_t bufferSize = 4096;
std::vector<char> buffer(bufferSize);
//Обрабатываем каждый входной фаил вытаскиявая информацию из вектора
for (size_t i = 1; i < fileParts.size(); i++)
{
    //Открываем файл в бинарном режиме
    std::ifstream inFile(fileParts[i].partFilePath, std::ios::binary);
    if (!inFile)
    {
        throw std::runtime_error("Cannot open input file: " + fileParts[i].partFilePath);
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
}
*/




/*
    Для теста создал в папке три текстовых файла и рядом папку для объединенного файла

int main() {
    try {
        std::vector <filePart2> testVec;
        filePart2 first = { "test.txt", "0", "0"};
        testVec.push_back(first);
        filePart2 second = { "0","Part1.txt", "C:/Games/test parts/Part1.txt"};
        filePart2 third = { "0","Part2.txt", "C:/Games/test parts/Part2.txt"};
        filePart2 fourth = { "0","Part3.txt", "C:/Games/test parts/Part3.txt"};
        testVec.push_back(second);
        testVec.push_back(third);
        testVec.push_back(fourth);
        Storage::mergeFile(testVec, "C:/Games/test file");



    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}*/
