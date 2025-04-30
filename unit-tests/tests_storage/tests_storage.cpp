#define BOOST_TEST_MODULE My test module
#include <boost/test/included/unit_test.hpp>
#include <string>
#include <storage.h>

BOOST_AUTO_TEST_SUITE(tests_storage)

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
