/*#include <iostream>

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}*/
// для проверки работы
 //main.cpp

 // #include <iostream>
 // #include "src/Storage/filesplitter.h"
 // #include "src/Storage/hashsummmanager.h"
 // #include <string>
 // #include <boost/filesystem.hpp>


 // int main() {
 //     FileSplitter splitter;
 //     std::string filePath = "/home/qwuser/Downloads/Periodic cooking of eggs.pdf"; // Замените на реальный путь к файлу
 //     boost::filesystem::path tempDir = boost::filesystem::temp_directory_path();
 //     std::string tempDirStr = tempDir.string();
 //   //      = "/tmp/temp_dir";      // Замените на реальную временную директорию
 //     size_t chunkSize = 4096;

 //     std::cout << "temp dir " + tempDirStr << std::endl;
 //     try {
 //         splitter.split(filePath, tempDirStr, chunkSize);
 //         std::cout << "Файл успешно разделен." << std::endl;
 //     } catch (const std::runtime_error& error) {
 //         std::cerr << "Ошибка при разделении файла: " << error.what() << std::endl;
 //         return 1; // Возвращаем код ошибки
 //     }

 //     return 0;
 // }

#include "src/Storage/hashsummmanager.h"
#include <iostream>
#include <fstream>
#include <string>

 int main() {
     HashSummManager manager;

     // 1. Проверка calculateFileHash
     std::string filePath = "/home/qwuser/Downloads/Periodic cooking of eggs.pdf"; // Создайте этот файл для теста
     // std::ofstream outFile(filePath);
     // outFile << "This is a test file.";
     // outFile.close();

     size_t fileHash1 = manager.calculateFileHash(filePath);
     std::cout << "Хэш файла '" << filePath << "': " << fileHash1 << std::endl;
     size_t fileHash2 = manager.calculateFileHash(filePath);
     std::cout << "Хэш файла '" << filePath << "': " << fileHash2 << std::endl;

     // 2. Проверка compareHashes
     size_t hash1 = fileHash1;
     size_t hash2 = fileHash2;
     size_t hash3 = 12345;

     std::cout << "Сравнение хешей (" << hash1 << ") и (" << hash2 << "): " << (manager.compareHashes(hash1, hash2) ? "Истина" : "Ложь") << std::endl;
     std::cout << "Сравнение хешей (" << hash1 << ") и (" << hash3 << "): " << (manager.compareHashes(hash1, hash3) ? "Истина" : "Ложь") << std::endl;

     // 3. Проверка calculateDirectoryHash
     std::string directoryPath = "/tmp/Periodic-cooking-of-eggs"; // Текущая директория
     std::string filenameContains = "Periodic-cooking-of-eggs";  // Ищем файлы, содержащие "test" в имени

     size_t directoryHash = manager.calculateDirectoryHash(directoryPath);//, filenameContains);
     std::cout << "Хэш директории '" << directoryPath << "' : " << directoryHash << std::endl;

     // Очистка тестового файла
     //std::remove(filePath.c_str());

     return 0;
 }
