/**
 * @file tests_net.cpp
 * @brief Unit-тесты сетевого взаимодействия распределенной файловой системы.
 *
 * Данный файл содержит модульные тесты  для тестирования взаимодействия между узлами и клиентской частью
 * системы через протокол.
 */

#define BOOST_TEST_MODULE My test module
#include <boost/test/included/unit_test.hpp>
#include <string>
#include <vector>
#include "network.h"

// Заглушка функции getActiveNodes
std::vector<std::string> getActiveNodes() {
    return {"192.168.1.1", "192.168.1.2", "192.168.1.3"};
}

// Заглушка функции receiveData
std::string receiveData(const std::string& nodeIP) {
    if (nodeIP == "192.168.1.1") return "data from node";
    return "";
}

// Заглушка функции synchronizeData
bool synchronizeData(const nlohmann::json& fileMetadata) {
    // Если есть "filename" и он не пустой - синхронизация успешная
    if (fileMetadata.contains("filename") && !fileMetadata["filename"].get<std::string>().empty()) {
        return true;
    }
    return false;
}

// Заглушка функции checkNodeStatus()
bool checkNodeStatus(const std::string& nodeIP) {
    // Cчитаем, что "192.168.1.1" активен, а остальные нет
    if (nodeIP == "192.168.1.1") return true;
    if (nodeIP.empty()) return false;
    return false;
}

BOOST_AUTO_TEST_SUITE(SendDataTests)

BOOST_AUTO_TEST_CASE(SendData_ValidInput_ReturnsExpectedResponse)
{
    std::string nodeIP = "192.168.1.1";
    std::string data = sendData(nodeIP);

    // Ожидаемый результат
    std::string expectedResponse = "OK";

    std::string actualResponse = sendData(nodeIP, data);

    BOOST_CHECK_EQUAL(actualResponse, expectedResponse);
}

BOOST_AUTO_TEST_CASE(SendData_InvalidIP_ReturnsError)
{
    std::string invalidIP = "256.256.256.256"; // Невалидный IP
    std::string data = sendData(nodeIP);

    std::string expectedResponse = "Error: invalid IP";

    std::string actualResponse = sendData(invalidIP, data);

    BOOST_CHECK_EQUAL(actualResponse, expectedResponse);
}

BOOST_AUTO_TEST_CASE(SendData_EmptyData_ReturnsError)
{
    std::string nodeIP = "192.168.1.1";
    std::string emptyData = "";

    std::string expectedResponse = "empty data";

    std::string actualResponse = sendData(nodeIP, emptyData);

    BOOST_CHECK_EQUAL(actualResponse, expectedResponse);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ReceiveDataTests)

//
BOOST_AUTO_TEST_CASE(ValidNode_ReturnsData)
{
    std::string nodeIP = "192.168.1.1";
    std::string data = receiveData(nodeIP);
    BOOST_CHECK(!data.empty());
    BOOST_CHECK(data.find("test data") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(InvalidNode_ReturnsEmpty)
{
    std::string nodeIP = "10.0.0.1"; // неактивный или недоступный узел
    std::string data = receiveData(nodeIP);
    BOOST_CHECK(data.empty());
}

BOOST_AUTO_TEST_CASE(EmptyIP_ReturnsEmpty)
{
    std::string nodeIP = "";
    std::string data = receiveData(nodeIP);
    BOOST_CHECK(data.empty());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(GetActiveNodesTests)

// Проверяем, что список активных узлов не пустой
BOOST_AUTO_TEST_CASE(ReturnsNonEmptyList)
{
    std::vector<std::string> nodes = getActiveNodes();
    BOOST_CHECK(!nodes.empty());
}

// Проверяем, что в списке есть конкретные ожидаемые IP-адреса
BOOST_AUTO_TEST_CASE(ContainsExpectedNodes)
{
    std::vector<std::string> nodes = getActiveNodes();

    BOOST_CHECK(std::find(nodes.begin(), nodes.end(), "192.168.1.1") != nodes.end());
    BOOST_CHECK(std::find(nodes.begin(), nodes.end(), "192.168.1.2") != nodes.end());
    BOOST_CHECK(std::find(nodes.begin(), nodes.end(), "192.168.1.3") != nodes.end());
}

//Проверяем, что в списке нет повторяющихся элементов
BOOST_AUTO_TEST_CASE(NoDuplicates)
{
    std::vector<std::string> nodes = getActiveNodes();
    std::sort(nodes.begin(), nodes.end());
    auto last = std::unique(nodes.begin(), nodes.end());
    BOOST_CHECK(last == nodes.end());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(SynchronizeDataTests)

BOOST_AUTO_TEST_CASE(ValidMetadata_ReturnsTrue)
{
    nlohmann::json validMetadata = {
        {"filename", "data.txt"},
        {"size", 1024},
        {"checksum", "abc123"}
    };

    bool result = synchronizeData(validMetadata);
    BOOST_CHECK(result);
}

BOOST_AUTO_TEST_CASE(MissingFilename_ReturnsFalse)
{
    nlohmann::json invalidMetadata = {
        {"size", 1024},
        {"checksum", "abc123"}
    };

    bool result = synchronizeData(invalidMetadata);
    BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(EmptyFilename_ReturnsFalse)
{
    nlohmann::json invalidMetadata = {
        {"filename", ""},
        {"size", 1024}
    };

    bool result = synchronizeData(invalidMetadata);
    BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(EmptyJson_ReturnsFalse)
{
    nlohmann::json emptyMetadata = nlohmann::json::object();

    bool result = synchronizeData(emptyMetadata);
    BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(CheckNodeStatusTests)

BOOST_AUTO_TEST_CASE(ActiveNode_ReturnsTrue)
{
    std::string nodeIP = "192.168.1.1";
    bool status = checkNodeStatus(nodeIP);
    BOOST_CHECK(status == true);
}

BOOST_AUTO_TEST_CASE(InactiveNode_ReturnsFalse)
{
    std::string nodeIP = "10.0.0.1";
    bool status = checkNodeStatus(nodeIP);
    BOOST_CHECK(status == false);
}

BOOST_AUTO_TEST_CASE(EmptyIP_ReturnsFalse)
{
    std::string nodeIP = "";
    bool status = checkNodeStatus(nodeIP);
    BOOST_CHECK(status == false);
}

BOOST_AUTO_TEST_SUITE_END()
