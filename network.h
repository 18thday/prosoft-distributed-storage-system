/**
 * @file network.h
 * @brief Интерфейс сетевого взаимодействия для распределенной файловой системы.
 *
 * Данный файл содержит функции для взаимодействия между узлами и клиентской частью
 * системы через протокол .
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

/// === Основные функции === ///

/**
 * @brief Отправить данные на указанный узел.
 * @param nodeIP IP-адрес узла.
 * @param data Данные для отправки.
 * @return true, если данные успешно отправлены, иначе false.
 */
bool sendData(const std::string& nodeIP, const std::string& data);

/**
 * @brief Получить данные от узла.
 * @param nodeIP IP-адрес узла.
 * @return Данные, полученные от узла.
 */
std::string receiveData(const std::string& nodeIP);

/**
 * @brief Получить список активных узлов в сети.
 * @return Вектор строк с IP-адресами активных узлов.
 */
std::vector<std::string> getActiveNodes();

/**
 * @brief Синхронизировать данные с другими узлами.
 * @param fileMetadata Метаданные файла для синхронизации.
 * @return true, если синхронизация прошла успешно, иначе false.
 */
bool synchronizeData(const std::unordered_map<std::string, std::string>& fileMetadata);

/**
 * @brief Проверить состояние соединения с узлом.
 * @param nodeIP IP-адрес узла.
 * @return true, если узел доступен, иначе false.
 */
bool checkNodeStatus(const std::string& nodeIP);

/// === Внутренние функции === ///

/**
 * @brief Установить соединение с узлом.
 * @param nodeIP IP-адрес узла.
 * @return true, если соединение установлено, иначе false.
 */
bool connectToNode(const std::string& nodeIP);

/**
 * @brief Разорвать соединение с узлом.
 * @param nodeIP IP-адрес узла.
 */
void disconnectFromNode(const std::string& nodeIP);

/**
 * @brief Получить состояние узлов.
 * @return Словарь с состоянием узлов (IP -> доступность).
 */
std::unordered_map<std::string, bool> getNodeStatus();