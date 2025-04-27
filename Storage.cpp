#include "Storage.h"
#include <iostream>
#include <fstream>
#include <stdexcept> // Для std::runtime_error

void Storage::mergeFile(const std::vector<filePart2> fileParts, const std::string& outputPath) 
{
    /*Переменная содержащая путь где нужно создать фаил + название файла.
      Как возможная идея, хранить информацию о файле в перой ячейке вектора vector<filePart2>, но это еще надо обсудить.
      В остальных ячейках данные о кусочках. Также вектор должен быть отсортирован.
      В реализации этого метода попробовал использовать эту идею.
      Имя файла предлагаю сохранть при его делении
    */
    std::string filePath = outputPath + "/" + fileParts[0].fileName;
    //Проверка, достаточно ли места для будущего файла (не тестировал) 
    size_t fileSize = 0;
    for (int i = 0; i < fileParts.size(); i++) 
    {
        fileSize += boost::filesystem::file_size(fileParts[i].partFilePath);
    }
    if (!hasEnoughSpace(fileSize, tempDir)) {
        throw std::runtime_error("Not enough space on disk to split the file.");
    }

    // Создаем фаил с нужным названием в нужном месте, в бинарном режиме
	std::ofstream outFile(filePath, std::ios::binary);
	if (!outFile)
	{
		throw std::runtime_error("Cannot open output file:" + outputPath);
	}
    // Создаем буфер размера 4096
    size_t bufferSize = 4096;
    std::vector<char> buffer(bufferSize);
    //Обрабатываем каждый входной фаил вытаскиявая информацию из вектора
    for (size_t i = 1; i < fileParts.size(); i++)
    {
        //Открываем файл в бинарном режиме
        std::ifstream inFile(fileParts[i].partFilePath, std::ios::binary);
        if (!inFile)
        {
            throw std::runtime_error("Cannot open input file: " + fileParts[i].partFilePath);
        }
        //Читаем фаил блоками
        while (inFile.read(buffer.data(), buffer.size()))
        {
            // Записываем в коенчный фаил прочитанный блок
            outFile.write(buffer.data(), inFile.gcount());
        }
        //Записываем оставшиеся данные(меньше размера буфера)
        outFile.write(buffer.data(), inFile.gcount());
        inFile.close();
    }
    outFile.close();
}

bool Storage::hasEnoughSpace(size_t fileSize, std::string tempDir) const
{
    // Проверяем, достаточно ли места на диске
    boost::filesystem::space_info spaceInfo = boost::filesystem::space(tempDir);
    return spaceInfo.available >= fileSize;
}

/*
    Для теста создал в папке три текстовых файла и рядом папку для объединенного файла
    
int main() {
    try {
        std::vector <filePart2> testVec;
        filePart2 first = { "test.txt", "0", "0"};
        testVec.push_back(first);
        filePart2 second = { "0","Part1.txt", "C:/Games/test parts/Part1.txt"};
        filePart2 third = { "0","Part2.txt", "C:/Games/test parts/Part2.txt"};
        filePart2 fourth = { "0","Part3.txt", "C:/Games/test parts/Part3.txt"};
        testVec.push_back(second);
        testVec.push_back(third);
        testVec.push_back(fourth);
        Storage::mergeFile(testVec, "C:/Games/test file");
      

        
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}*/
