/*
#include "storageclient.h"
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>

StorageClient::StorageClient(NetworkClient* networkClient, FileMetadataManager* metadataManager) :
    networkClient(networkClient), metadataManager(metadataManager)
{
    if (!networkClient || !metadataManager) {
        throw std::runtime_error("Некорректные указатели на NetworkClient и MetadataManager");
    }
}

void StorageClient::splitFileAndUpload(const std::string& filePath, const std::vector<std::string>& serverAddresses) {
    try {
        // 1. Разделение файла на фрагменты
        std::vector<std::pair<int, std::vector<char>>> fragments = networkClient->splitFile(filePath);

        // 2. Создание метаданных
        FileMetadata metadata;
        metadata.filename = filePath;
        metadata.fileSize = 0; // Заполнить размер файла позже (если нужно)

        for (const auto& fragment : fragments) {
            metadata.fragments.push_back({fragment.first, "server1" }); // Заменить на логику выбора сервера
        }

        // 3. Сохранение метаданных
        metadataManager->saveMetadata(metadata);

        // 4. Отправка фрагментов на серверы
        for (size_t i = 0; i < fragments.size(); ++i) {
            networkClient->uploadFragment(fragments[i].second, serverAddresses[i % serverAddresses.size()]); // Распределение по серверам
        }

        std::cout << "Файл успешно разделен и загружен на серверы." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Ошибка при разделении и загрузке файла: " << e.what() << std::endl;
    }
}

std::vector<char> StorageClient::assembleFile(const std::vector<std::pair<int, std::string>>& fragments, const std::string& destinationPath) {
    try {
        // 1. Загрузка фрагментов с серверов
        std::vector<std::vector<char>> downloadedFragments;
        for (const auto& fragment : fragments) {
            downloadedFragments.push_back(networkClient->downloadFragment(fragment.first));
        }

        // 2. Сборка фрагментов в единый файл
        std::vector<char> assembledFileContent;
        for (const auto& fragment : downloadedFragments) {
            assembledFileContent.insert(assembledFileContent.end(), fragment.begin(), fragment.end());
        }

        // 3. Сохранение собранного файла на диск
        std::ofstream outFile(destinationPath, std::ios::binary);
        if (!outFile.is_open()) {
            throw std::runtime_error("Ошибка открытия файла для записи: " + destinationPath);
        }

        outFile.write(reinterpret_cast<const char*>(assembledFileContent.data()), assembledFileContent.size());
        outFile.close();

        return assembledFileContent; // Возвращаем содержимое собранного файла (полезно для тестирования)

    } catch (const std::exception& e) {
        std::cerr << "Ошибка при сборке файла: " << e.what() << std::endl;
        return {}; // Возвращаем пустой вектор в случае ошибки
    }
}
*/
