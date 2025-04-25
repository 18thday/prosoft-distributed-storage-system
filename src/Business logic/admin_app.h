// =============
// Интерфейс для администратора. Управление сервером, взаимодействие с файловой системой.
// =============

enum StatusEnum {
    stopped,
    working,
    reconfiguring
};

enum Command {
    // основные команды
    start,
    stop,
    get_status,
    // дополнительные команды
    put_file,
    remove_file,
    ...
};

/**
* @brief Обрабатывает команды администратора.
*/
void process_command(Command cmd)
{
    switch (cmd)
    {
        case start:
            startServer()
        break;
        ...
    }
}

/**
* @brief Запустить сервер на данной машине.
*/
void start_server();

/**
* @brief Остановить сервер на данной машине.
*/
void stop_server();

StatusEnum get_server_status();

// /**
// * @brief Загрузить файл в распределенное хранилище.
// * @param filePath Путь к файлу.
// */
// void upload_file(const std::string& filePath);

// /**
// * @brief Скачать файл по его ID.
// * @param fileId Уникальный идентификатор файла.
// * @param outputPath Путь для сохранения.
// */
// void download_file(const std::string& fileId, const std::string& outputPath);

// /**
// * @brief Удалить файл из системы.
// * @param fileId Уникальный идентификатор файла.
// */
// void deleteFile(const std::string& fileId);

// /**
// * @brief Очистить файловую систему кластера.
// * @param fileId Уникальный идентификатор файла.
// */
// void clearAllFiles(const std::string& fileId);