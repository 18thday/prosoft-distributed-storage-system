#include "filesplitter.h"
#include <fstream>
#include <sstream>
#include <cmath>

FileSplitter::FileSplitter() {}
// Создаем временную директорию при создании объекта. В текущей реализации не используется.
//= boost::filesystem::temp_directory_path() / "file_splitter"; // Добавляем поддиректорию для организации
//createTempDirectory();

int FileSplitter::split(const std::string& filePath,
                                const std::string& tempDir,
                                size_t chunkSize)
{
    boost::filesystem::path inputFile(filePath);
    if (!boost::filesystem::exists(inputFile)) {
        throw std::runtime_error("File not found: " + filePath);
    }

    size_t fileSize = boost::filesystem::file_size(inputFile);
    if (!hasEnoughSpace(fileSize, tempDir)) {
        throw std::runtime_error("Not enough space on disk to split the file.");
    }

    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile.is_open()) {
        throw std::runtime_error("Could not open input file: " + filePath);
    }

    size_t numChunks = static_cast<size_t>(std::ceil(static_cast<double>(fileSize) / chunkSize)); // Округление вверх
    std::string sanitizedFilename = sanitizeFilename(inputFile.filename().string());
    boost::filesystem::path chunkDir = tempDir + "/" + sanitizedFilename;
    // создаем директорию для чанков
    if (!boost::filesystem::exists(chunkDir)) {
        boost::filesystem::create_directories(chunkDir);
    } else if (!boost::filesystem::is_directory(chunkDir)) {
        throw std::runtime_error("Path exists but is not a directory: " + chunkDir.string());
    }

    // основная логика
    for (size_t i = 0; i < numChunks; ++i)
    {
        std::string chunkFilename = sanitizedFilename + "_" + std::to_string(i);
        chunkFilename += ".part";  // Добавляем расширение, чтобы было понятно, что это часть файла

        boost::filesystem::path chunkPath = chunkDir / chunkFilename;

        std::ofstream outFile(chunkPath.string(), std::ios::binary);
        if (!outFile.is_open()) {
            throw std::runtime_error("Could not create output file: " + chunkPath.string());
        }

        //char buffer[chunkSize];
        std::unique_ptr<char[]> buffer(new char[chunkSize]);

        inFile.read(buffer.get(), chunkSize);
        outFile.write(buffer.get(), inFile.gcount());

        outFile.close();
    }

    inFile.close();
    return 0;
}

bool FileSplitter::hasEnoughSpace(size_t fileSize, std::string tempDir) const
{
    // Проверяем, достаточно ли места на диске
    boost::filesystem::space_info spaceInfo = boost::filesystem::space(tempDir);
    return spaceInfo.available >= fileSize;
}

std::string FileSplitter::sanitizeFilename(const std::string& filename) const
{
    boost::filesystem::path path(filename);
    path = path.stem(); // отбрасываем расширение
    std::ostringstream oss;
    for (char c : path.string()) {
        if (c == ' ' || c == '_') {
            oss << '-'; // Заменяем пробелы на дефисы
        } else {
            oss << c;
        }
    }
    return oss.str();
}
/* для проверки работы
 * main.cpp
 *
 * #include <iostream>
 * #include "src/Storage/filesplitter.h"
 * #include <string>
 * #include <boost/filesystem.hpp>
 *
 *
 * int main() {
 *     FileSplitter splitter;
 *     std::string filePath = "/home/qwuser/Downloads/nbki-report(cp).pdf"; // Замените на реальный путь к файлу
 *     boost::filesystem::path tempDir = boost::filesystem::temp_directory_path();
 *     std::string tempDirStr = tempDir.string();
 *   //      = "/tmp/temp_dir";      // Замените на реальную временную директорию
 *     size_t chunkSize = 4096;
 *
 *     std::cout << "temp dir " + tempDirStr << std::endl;
 *     try {
 *         splitter.split(filePath, tempDirStr, chunkSize);
 *         std::cout << "Файл успешно разделен." << std::endl;
 *     } catch (const std::runtime_error& error) {
 *         std::cerr << "Ошибка при разделении файла: " << error.what() << std::endl;
 *         return 1; // Возвращаем код ошибки
 *     }
 *
 *     return 0;
 * }
 */

/*
void FileSplitter::createTempDirectory() const
{
    if (!boost::filesystem::exists(tempDir_)) {
        try {
            boost::filesystem::create_directories(tempDir_);
        } catch (const boost::filesystem::filesystem_error& e) {
            throw std::runtime_error("Could not create temporary directory: " + tempDir_.string() + ". Error: " + e.what());
        }
    }
}
*/

