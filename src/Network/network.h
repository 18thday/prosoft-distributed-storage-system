#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

// Объявляем функции для работы с сетью
std::string sendData(const std::string& nodeIP, const std::string& data); // Отправка данных
std::string receiveData(const std::string& nodeIP); // Получение данных (нема)
std::vector<std::string> getActiveNodes(); // Получение активных узлов (нема)
bool synchronizeData(const nlohmann::json& fileMetadata); // Синхронизация данных
bool checkNodeStatus(const std::string& nodeIP); // Проверка статуса узла (нема)

#endif