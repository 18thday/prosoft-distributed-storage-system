#ifndef CLIENT_APP_H
#define CLIENT_APP_H

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "storage.h"
#include "types.h"
#include <boost/asio.hpp>
#include <QThread>

// Структура для передачи метаданных файла в GUI
struct FileMetadata {
    std::string id; // Идентификатор файла
    size_t size;    // Размер файла
};

// Интерфейс для связи с GUI
class IBLClient {
public:
    // Уведомления о завершении операций
    virtual void onUploadComplete(const std::string& fileId) = 0;
    virtual void onDownloadComplete(const std::string& fileId, const std::string& outputPath) = 0;
    virtual void onDeleteComplete(const std::string& fileId) = 0;
    // Сообщение об ошибке
    virtual void onError(const std::string& message) = 0;
    // Обновление списка узлов
    virtual void onNodesUpdated(const std::vector<std::string>& nodes) = 0;
    // Обновление списка файлов
    virtual void onFilesUpdated(const nlohmann::json& files) = 0;
    // Прогресс загрузки/скачивания
    virtual void onUploadProgress(const std::string& fileId, uint64_t partNumber, uint64_t totalParts) = 0;
    virtual void onDownloadProgress(const std::string& fileId, uint64_t partNumber, uint64_t totalParts) = 0;
    // Результат операций
    virtual void onUploadFinished(const std::string& fileId, bool success, const std::string& message) = 0;
    virtual void onDownloadFinished(const std::string& fileId, bool success, const std::string& message) = 0;
    virtual void onDeleteFinished(const std::string& fileId, bool success, const std::string& message) = 0;
    // Результат запроса списка файлов
    virtual void onListFilesResult(const std::vector<FileMetadata>& files) = 0;

    virtual ~IBLClient() = default;
};

// Класс для управления сетевым потоком
class NetworkThread : public QThread {
    Q_OBJECT
public:
    NetworkThread() : io_context_(), work_(std::make_unique<boost::asio::io_context::work>(io_context_)) {}
    // Запускает сетевой цикл
    void run() override { io_context_.run(); }
    // Возвращает контекст для сетевых операций
    boost::asio::io_context& get_io_context() { return io_context_; }
    // Останавливает поток
    void stop() {
        work_.reset();
        io_context_.stop();
    }

private:
    boost::asio::io_context io_context_;
    std::unique_ptr<boost::asio::io_context::work> work_;
};

// Основной класс для взаимодействия клиента с серверами
class ClientToServerAPI {
public:
    explicit ClientToServerAPI(IBLClient* client);
    ~ClientToServerAPI();

    // Основные операции
    void uploadFile(const std::string& filePath); // Загрузка файла
    void downloadFile(const std::string& fileId, const std::string& outputPath); // Скачивание файла
    void deleteFile(const std::string& fileId); // Удаление файла
    void connect(const std::string& nodeIpPort); // Подключение к узлу
    std::vector<std::string> getActiveNodes(); // Получение активных узлов
    void listFiles(); // Запрос списка файлов

private:
    // Вспомогательные методы
    bool isNodeAvailable(const std::string& nodeIpPort); // Проверка доступности узла
    std::vector<FilePart> splitFile(const std::string& filePath, FileMetaData& metadata, size_t node_count); // Разделение файла
    void distributeParts(const std::vector<FilePart>& parts, const FileMetaData& metadata, const std::vector<std::string>& active_nodes); // Распределение частей
    void restoreFile(const std::string& fileId, const std::string& outputPath, const std::vector<std::string>& active_nodes); // Восстановление файла

    IBLClient* m_client; // Указатель на GUI
    std::string m_node_ip; // Текущий адрес узла
    std::unordered_map<std::string, FileMetaData> file_metadata_; // Локальное хранилище метаданных
    NetworkThread* network_thread_; // Поток для сетевых операций
};

#endif