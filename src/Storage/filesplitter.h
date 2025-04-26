#ifndef FILESPLITTER_H
#define FILESPLITTER_H

#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <stdexcept>
#include <memory>


/**
 * @brief Класс для разделения файла на части (chunks).
 */
class FileSplitter
{
public:

    FileSplitter();
    ~FileSplitter() = default;

    /**
     * @brief Разделяет файл на чанки и сохраняет их во временной директории.
     *
     * @param filePath Путь к файлу, который нужно разделить.  Должен указывать на существующий файл.
     * @param tempDir  Путь к временной директории, где будут храниться чанки. Директория должна существовать и быть доступна для записи.
     * @param chunkSize Размер каждого чанка в байтах. Должен быть положительным значением.
     * @return Путь к созданной временной папке, содержащей чанки файла.
     * @throws std::runtime_error Если файл не найден, недостаточно места на диске для создания чанков или произошла ошибка при создании выходных файлов.
     */
    int split(const std::string& filePath,
                      const std::string& tempDir,
                      size_t chunkSize); // Возвращает путь к временной папке

private:
    /// пока что не используются
    std::string filePath_;
    size_t chunkSize_;
    std::string tempDir_;

private:
    /**
     * @brief Проверяет, достаточно ли места на диске для разделения файла.
     * @param fileSize Размер файла в байтах.
     * @return True, если на диске достаточно места; иначе False.
     */
    bool hasEnoughSpace(size_t fileSize, std::string tempDir) const;
    /**
     * @brief Очищает имя файла от недопустимых символов (например, пробелов).
     * @param filename Имя файла для очистки.
     * @return Очищенное имя файла.
     */
    std::string sanitizeFilename(const std::string& filename) const; // Обработка пробелов и спецсимволов

    //void createTempDirectory() const;
};

#endif // FILESPLITTER_H
