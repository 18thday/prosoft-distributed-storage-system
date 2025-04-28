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