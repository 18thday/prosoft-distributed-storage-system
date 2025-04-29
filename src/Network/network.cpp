#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

// Подключаем библиотеки для работы с сетью и JSON
using boost::asio::ip::tcp;
using json = nlohmann::json;

// Функция для отправки данных на сервер
std::string sendData(const std::string& nodeIpPort, const std::string& data) {
    boost::asio::io_context io_context;
    tcp::resolver resolver(io_context);
    tcp::socket socket(io_context);
    bool connected = false;

    try {
        // Разрешаем адрес узла (IP и порт)
        auto endpoints = resolver.resolve(nodeIpPort.substr(0, nodeIpPort.find(':')), nodeIpPort.substr(nodeIpPort.find(':') + 1));
        boost::asio::connect(socket, endpoints);
        connected = true;

        // Включаем keep-alive для стабильного соединения
        socket.set_option(boost::asio::socket_base::keep_alive(true));

        // Отправляем данные с добавлением новой строки
        std::string request = data + "\n";
        boost::asio::write(socket, boost::asio::buffer(request));

        // Читаем ответ
        boost::asio::streambuf response;
        response.prepare(1024 * 1024 * 4); // Резервируем 4 МБ для ответа
        boost::system::error_code ec;
        std::string response_str;

        // Читаем данные до конца ответа
        while (!ec && socket.is_open()) {
            boost::asio::read_until(socket, response, '\n', ec);
            if (ec && ec != boost::asio::error::eof) {
                std::cerr << "Read error: " << ec.message() << "\n";
                socket.close();
                return "";
            }

            std::string chunk((std::istreambuf_iterator<char>(&response)), std::istreambuf_iterator<char>());
            if (!chunk.empty() && chunk.back() == '\n') {
                chunk.pop_back();
            }
            response_str += chunk;

            if (ec == boost::asio::error::eof) {
                break; // Конец данных
            }
        }

        std::cout << "Client received response from " << nodeIpPort << ": " << response_str.size() << " bytes\n";

        socket.close();
        return response_str;
    } catch (const std::exception& e) {
        std::cerr << "Error sending data to " << nodeIpPort << ": " << e.what() << "\n";
        if (connected) {
            socket.close();
        }
        return "";
    }
}

// Функция-заглушка для синхронизации данных
bool synchronizeData(const nlohmann::json& /*fileMetadata*/) {
    std::cout << "Synchronizing data (not implemented)\n";
    return true;
}