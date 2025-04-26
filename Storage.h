#pragma once
#include <vector>
#include <string>

struct FilePart {
   // char* firstBytes;
    size_t partSize;
    size_t partNumber;
    unsigned long filePratHash;
    unsigned long fileHash;
    std::string& hashFilePath;
    std::string& hashFilePartPath;
};

class Storage
{
public:
    Storage(const std::string& filePath, const int numberOfParts); // , size_t partSizeInBytes)
    
    /**
      * @brief Деление файла на части
      * @param filePath Путь к файлу
      * @param numberOfParts Число частей
      * @return std::vector<FilePart> Возвращаем вектор структур с заполненными данными деления 
    */
    static std::vector<FilePart> splitFile(const std::string& filePath, const int numberOfParts);
    /**
      * @brief Объединение кусков файла в единый
      * @param std::vector<FilePart> Вектор со всей информация о частях
      * @patam string& outputPath Путь для скачивания объединенного файла
    */
    static void mergeFile(const std::vector<FilePart>, const std::string& outputPath);
    /**
      * @brief Перераспределение частей файлов
      * @param std::vector<FilePart> Вектор со всей информация о имеющихся частях файла старой конфигурации
      * @patam int newNumbersOfParts Новое колличество частей(узлов)
      * @return vector<FilePart> Возвращаем вектор с актуальными разделениями
    */
    static std::vector<FilePart> reconfiguration(const std::vector<FilePart>, const int newNumbersOfParts);

       
private:
    std::string& filePath; // Путь файла
    int numberOfParts; // Число частей на которые нужно разделить фаил
}

