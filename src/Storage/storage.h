<<<<<<< HEAD
struct FilePart {
    char* firstBytes;
    size_t partSize;
    size_t partNumber;
}

class Storage
{
public:
    Storage(const std::string& filePath); // , size_t partSizeInBytes)
    //
    /*struct FilePart {
        std::string hash;      // Хэш блока (SHA-256)
        std::vector<uint8_t> data;  // Бинарные данные
    };*/
    //структура для возвращения условного разделения

    static std::vector<FilePart> splitFile(const std::string& filePath);
    static void mergeFile(const std::std::vector<FilePart>, const std::string& outputPath);
    std::vector<FilePart> reconfiguration ()

    /*
    
    передали файл, возвращает мапы, объекты кусок файла 
    указатель на кусочек и размер кусочка
    распил реконфигурация
    отправить кусок файла
    принять кусок файла
    соединение файла
    */
}

/*
вычислить хэш файла
записать хэш файла в другой файл
распилить файл по пути
вычислить хэш для куска файла
записать хэш куска файла



*/
=======
#ifndef STORAGE_H
#define STORAGE_H

#pragma once
#include <vector>
#include <string>
#include <boost/filesystem.hpp>

struct FilePart {
    // char* firstBytes;
    size_t partSize;
    size_t partNumber;
    unsigned long filePratHash;
    unsigned long fileHash;
    std::string& hashFilePath;
    std::string& hashFilePartPath;
};
/*
    Эту структуру я создал для реализации метода объединения частей файла, этот момент надо будет обсудить.
    Использовал вектор vector<filePart2> fileParts первая ячейка которого занята информацие о файле (изначальное название файла)
    В остальных ячейках будет содержаться информация о частях файла (имя и путь)

*/
struct filePart2 {
    std::string fileName;
    std::string partFileName;
    std::string partFilePath;
};
class Storage
{
public:
    Storage();
    //Storage(/*const std::string& filePath, const int numberOfParts*/) {} // , size_t partSizeInBytes)

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
    static void mergeFile(const std::vector<filePart2> fileParts, const std::string& tempDir, const std::string& outputPath);
    /**
      * @brief Перераспределение частей файлов
      * @param std::vector<FilePart> Вектор со всей информация о имеющихся частях файла старой конфигурации
      * @patam int newNumbersOfParts Новое колличество частей(узлов)
      * @return vector<FilePart> Возвращаем вектор с актуальными разделениями
    */
    static std::vector<FilePart> reconfiguration(const std::vector<FilePart>, const int newNumbersOfParts);


private:
    /**
     * @brief Проверяет, достаточно ли места на диске для разделения файла.
     * @param fileSize Размер файла в байтах.
     * @return True, если на диске достаточно места; иначе False.
     */
    static bool hasEnoughSpace(size_t fileSize, std::string tempDir);
    /**
     * @brief Очищает имя файла от недопустимых символов (например, пробелов).
     * @param filename Имя файла для очистки.
     * @return Очищенное имя файла.
     */
    std::string sanitizeFilename(const std::string& filename) const; // Обработка пробелов и спецсимволов

    std::string& filePath; // Путь файла
    int numberOfParts; // Число частей на которые нужно разделить фаил

};

#endif // STORAGE_H
>>>>>>> 5232b5a (update storage)
