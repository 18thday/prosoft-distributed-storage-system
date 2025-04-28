#include "hashsummmanager.h"
#include <iostream>

namespace fs = boost::filesystem;

HashSummManager::HashSummManager() {}

size_t HashSummManager::calculateFileHash(const std::string& filePath)
{
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl; // Обработка ошибки открытия файла
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

size_t HashSummManager::calculateDirectoryHash(const std::string& directoryPath)//,
                                               //const std::string& filename)
{
    size_t totalHash = 0;
    try {
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry))
                //&& entry.path().filename().string().find(filename) != std::string::npos)
            {
                //totalHash ^= calculateFileHash(entry.path().string()); // XOR для суммирования хешей
                boost::hash_combine(totalHash, calculateFileHash(entry.path().string()));
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error processing directory: " << e.what() << std::endl;
    }
    return totalHash;
}

bool HashSummManager::compareHashes(const size_t hash1, const size_t hash2)
{
    if (hash1 > 0 && hash2 > 0) {
        return hash1 == hash2;
    }
    return false;
}
