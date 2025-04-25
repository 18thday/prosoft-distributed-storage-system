#include "storage.h"

/**
* @brief Интерфейс взаимодействия пользователя с файловой системой.
*/
class ClientToServerAPI {
public:
    explicit ClientToServerAPI(IBLClient* client);
    ~ClientToServerAPI();


    /// === Основные методы === ///


    /**
    * @brief Загрузить файл в распределенное хранилище.
    * @param filePath Путь к файлу.
    */
    void uploadFile(const std::string& filePath);


    /**
    * @brief Скачать файл по его ID.
    * @param fileId Уникальный идентификатор файла.
    * @param outputPath Путь для сохранения.
    */
    void downloadFile(const std::string& fileId, const std::string& outputPath);


    /**
    * @brief Удалить файл из системы.
    * @param fileId Уникальный идентификатор файла.
    */
    void deleteFile(const std::string& fileId);


    /// === Управление узлами === ///






    /**
    * @brief Получить список активных узлов.
    */
    std::vector<std::string> getActiveNodes();


private:
    /// === Внутренние методы === ///
    std::vector<FilePart> splitFile(const std::string& filePath);
    void distributeParts(const std::vector<FilePart>& parts);
    void restoreFile(const std::string& fileId, const std::string& outputPath);


    /// === Поля === ///
    IBLClient* m_client;  // Callback-интерфейс для GUI
    NetworkClient m_network;  // Клиент для сетевого взаимодействия
    std::unordered_map<std::string, FileMetadata> m_fileIndex;  // Метаданные файлов
};
    