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
      * @brief ������� ����� �� �����
      * @param filePath ���� � �����
      * @param numberOfParts ����� ������
      * @return std::vector<FilePart> ���������� ������ �������� � ������������ ������� ������� 
    */
    static std::vector<FilePart> splitFile(const std::string& filePath, const int numberOfParts);
    /**
      * @brief ����������� ������ ����� � ������
      * @param std::vector<FilePart> ������ �� ���� ���������� � ������
      * @patam string& outputPath ���� ��� ���������� ������������� �����
    */
    static void mergeFile(const std::vector<FilePart>, const std::string& outputPath);
    /**
      * @brief ����������������� ������ ������
      * @param std::vector<FilePart> ������ �� ���� ���������� � ��������� ������ ����� ������ ������������
      * @patam int newNumbersOfParts ����� ����������� ������(�����)
      * @return vector<FilePart> ���������� ������ � ����������� ������������
    */
    static std::vector<FilePart> reconfiguration(const std::vector<FilePart>, const int newNumbersOfParts);

       
private:
    std::string& filePath; // ���� �����
    int numberOfParts; // ����� ������ �� ������� ����� ��������� ����
}

