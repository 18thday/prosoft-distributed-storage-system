#ifndef HASHSUMMMANAGER_H
#define HASHSUMMMANAGER_H

#include <fstream>
#include <string>
#include <vector>
#include <boost/functional/hash.hpp>
#include <boost/filesystem.hpp>

class HashSummManager
{
public:
    HashSummManager();

    size_t calculateFileHash(const std::string& filePath);
    bool compareHashes(const size_t hash1, const size_t hash2);
    size_t calculateDirectoryHash(const std::string& directoryPath);//, const std::string& filename);

private:
    void readHashFromJson(const std::string& jsonFilePath);
    const size_t chunkSize = 4096; // заменить на значение из конфиг файла
};

#endif // HASHSUMMMANAGER_H
