#define BOOST_TEST_MODULE My test module
#include <boost/test/included/unit_test.hpp>
#include <string>
#include <cstdio>
#include <storage.h>

BOOST_AUTO_TEST_SUITE(SplitFileTests)

// Вспомогательная функция для создания тестового файла с содержимым
void createTestFile(const std::string& filePath, const std::string& content) {
    std::ofstream ofs(filePath, std::ios::binary);
    ofs.write(content.data(), content.size());
    ofs.close();
}

// Проверяем, что функция возвращает правильное количество частей
BOOST_AUTO_TEST_CASE(SplitFile_CorrectNumberOfParts)
{
    const std::string testFile = "testfile.txt";
    const std::string content = "1234567890abcdef"; // 16 байт
    createTestFile(testFile, content);

    FileMetaData metadata;
    size_t parts_count = 4;

    auto parts = FileSplitter::splitFile(testFile, metadata, parts_count);

    BOOST_REQUIRE_EQUAL(parts.size(), parts_count);
    BOOST_REQUIRE_EQUAL(metadata.part_count, parts_count);
    BOOST_REQUIRE_EQUAL(metadata.size, content.size());

    std::remove(testFile.c_str());
}

// Проверяем, что сумма размеров частей равна размеру исходного файла
BOOST_AUTO_TEST_CASE(SplitFile_PartsSumToOriginalSize)
{
    const std::string testFile = "testfile2.txt";
    const std::string content = "abcdefghijklmnopqrstuvwxyz"; // 26 байт
    createTestFile(testFile, content);

    FileMetaData metadata;
    size_t parts_count = 5;

    auto parts = FileSplitter::splitFile(testFile, metadata, parts_count);

    size_t sumSize = 0;
    for (const auto& part : parts) {
        sumSize += part.data.size();
    }

    BOOST_REQUIRE_EQUAL(sumSize, content.size());
    BOOST_REQUIRE_EQUAL(metadata.size, content.size());

    std::remove(testFile.c_str());
}

// Проверяем корректность обработки остатка при делении файла на части
BOOST_AUTO_TEST_CASE(SplitFile_LastPartContainsRemainder)
{
    const std::string testFile = "testfile3.txt";
    const std::string content = "123456789"; // 9 байт
    createTestFile(testFile, content);

    FileMetaData metadata;
    size_t parts_count = 4;

    auto parts = FileSplitter::splitFile(testFile, metadata, parts_count);

    // Проверяем, что сумма частей равна размеру файла
    size_t sumSize = 0;
    for (const auto& part : parts) {
        sumSize += part.data.size();
    }
    BOOST_REQUIRE_EQUAL(sumSize, content.size());

    // Проверяем, что последняя часть содержит остаток (если есть)
    size_t expectedLastPartSize = content.size() - (content.size() / parts_count) * (parts_count - 1);
    BOOST_REQUIRE_EQUAL(parts.back().data.size(), expectedLastPartSize);

    std::remove(testFile.c_str());
}

// Проверяем поведение при нулевом числе частей
BOOST_AUTO_TEST_CASE(SplitFile_ZeroParts_ReturnsEmpty)
{
    const std::string testFile = "testfile4.txt";
    const std::string content = "data";
    createTestFile(testFile, content);

    FileMetaData metadata;
    size_t parts_count = 0;

    auto parts = FileSplitter::splitFile(testFile, metadata, parts_count);

    BOOST_CHECK(parts.empty());

    std::remove(testFile.c_str());
}


// Проверяем поведение при отсутствии файла
BOOST_AUTO_TEST_CASE(SplitFile_NonExistentFile_ReturnsEmpty)
{
    const std::string testFile = "ghostfile.txt";

    FileMetaData metadata;
    size_t parts_count = 3;

    auto parts = FileSplitter::splitFile(testFile, metadata, parts_count);

    BOOST_CHECK(parts.empty());
}

BOOST_AUTO_TEST_SUITE_END()

FilePart createFilePart(size_t index, const std::string& content) {
    FilePart part;
    part.index = index;
    part.data.assign(content.begin(), content.end());
    return part;
}

BOOST_AUTO_TEST_SUITE(MergeFileTests)

BOOST_AUTO_TEST_CASE(MergeFile_SuccessfulMerge)
{
    std::string outputFile = "merged_test_file.txt";

    // Подготовка частей
    std::vector<FilePart> parts = {
        createFilePart(0, "first "),
        createFilePart(1, "second "),
        createFilePart(2, "third")
    };

    FileMetaData metadata;
    metadata.part_count = parts.size();
    metadata.size = 12; // "first " (6) + "second " (5) + "third" (1)

    bool result = FileMerger::mergeFile(parts, outputFile, metadata);

    BOOST_CHECK(result);

    // Проверяем содержимое итогового файла
    std::ifstream ifs(outputFile, std::ios::binary);
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    BOOST_CHECK_EQUAL(content, "first second third");

    std::remove(outputFile.c_str());
}

BOOST_AUTO_TEST_CASE(MergeFile_PartCountMismatch_ReturnsFalse)
{
    std::string outputFile = "merged_test_file2.txt";

    std::vector<FilePart> parts = {
        createFilePart(0, "first"),
        createFilePart(1, "second")
    };

    FileMetaData metadata;
    metadata.part_count = 3; // Несовпадение с количеством частей
    metadata.size = 10;

    bool result = FileMerger::mergeFile(parts, outputFile, metadata);

    BOOST_CHECK(!result);

    // Файл не должен быть создан или пустой
    std::ifstream ifs(outputFile, std::ios::binary);
    BOOST_CHECK(!ifs.is_open() || ifs.peek() == std::ifstream::traits_type::eof());
    ifs.close();

    std::remove(outputFile.c_str());
}

BOOST_AUTO_TEST_CASE(MergeFile_CannotOpenOutputFile_ReturnsFalse)
{
    // Попытаемся записать в несуществующую директорию
    std::string outputFile = "/nonexistent_dir/output.bin";

    std::vector<FilePart> parts = {
        createFilePart(0, "Data")
    };

    FileMetaData metadata;
    metadata.part_count = 1;
    metadata.size = 4;

    bool result = FileMerger::mergeFile(parts, outputFile, metadata);

    BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(CalculateHashTests)

// проверяеv, что для одного и того же входа хэш всегда одинаковый
BOOST_AUTO_TEST_CASE(Hash_IsDeterministic)
{
    std::string input = "test";
    auto hash1 = calculateHash(input);
    auto hash2 = calculateHash(input);
    BOOST_CHECK_EQUAL(hash1, hash2);
}

//проверяем, что для разных строк хэш отличается
BOOST_AUTO_TEST_CASE(Hash_DifferentForDifferentInputs)
{
    std::string input1 = "test1";
    std::string input2 = "test2";
    BOOST_CHECK_NE(calculateHash(input1), calculateHash(input2));
}

//  проверяем поведение на пустой строке
BOOST_AUTO_TEST_CASE(Hash_EmptyInput)
{
    std::string input = "";
    auto hash = calculateHash(input);
    BOOST_CHECK(hash.empty());
}

BOOST_AUTO_TEST_CASE(Hash_SimilarButDifferentStrings)
{
    std::string input1 = "abc";
    std::string input2 = "abd";
    BOOST_CHECK_NE(calculateHash(input1), calculateHash(input2));
}

BOOST_AUTO_TEST_SUITE_END()
