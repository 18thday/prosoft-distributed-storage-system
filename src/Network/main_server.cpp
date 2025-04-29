#include "node_logic.h"
#include <boost/asio.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <memory>
#include <fstream>
#include <string>

// Подключаем библиотеки для работы с сетью и JSON
using boost::asio::ip::tcp;
using json = nlohmann::json;

// Функция для кодирования данных в base64
std::string base64_encode(const std::string& data) {
    using namespace boost::archive::iterators;
    using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
    std::string encoded(It(data.begin()), It(data.end()));
    size_t padding = (3 - data.size() % 3) % 3;
    encoded.append(padding, '=');
    return encoded;
}

// Функция для декодирования данных из base64
std::string base64_decode(const std::string& data) {
    using namespace boost::archive::iterators;
    using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
    std::string decoded;
    try {
        decoded.assign(It(data.begin()), It(data.end()));
        size_t padding = std::count(data.begin(), data.end(), '=');
        decoded.resize(decoded.size() - padding);
    } catch (const std::exception& e) {
        std::cerr << "Base64 decode error: " << e.what() << "\n";
        return "";
    }
    return decoded;
}

// Класс сервера, который принимает и обрабатывает запросы
class Server {
public:
    // Конструктор сервера
    Server([[maybe_unused]] const std::string& address, short port, NodeLogic& node)
        : acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)), node_(node) {
        startAccept(); // Начинаем принимать подключения
    }

    // Запускает сервер
    void run() {
        io_context_.run();
    }

private:
    // Начинает принимать новые подключения
    void startAccept() {
        auto socket = std::make_shared<tcp::socket>(io_context_);
        acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code& error) {
            if (!error) {
                handleClient(socket); // Обрабатываем клиента
            } else {
                std::cerr << "Accept error: " << error.message() << "\n";
            }
            startAccept(); // Продолжаем принимать новые подключения
        });
    }

    // Обрабатывает запросы от клиента
    void handleClient(std::shared_ptr<tcp::socket> socket) {
        auto buffer = std::make_shared<boost::asio::streambuf>();
        // Читаем данные до символа новой строки
        boost::asio::async_read_until(*socket, *buffer, '\n',
            [this, socket, buffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
                if (error) {
                    std::cerr << "Read error: " << error.message() << "\n";
                    if (error == boost::asio::error::eof) {
                        std::cerr << "Client disconnected unexpectedly\n";
                    }
                    return;
                }

                if (bytes_transferred == 0) {
                    std::cerr << "No data received\n";
                    return;
                }

                // Получаем данные из буфера
                std::string data((std::istreambuf_iterator<char>(&(*buffer))), std::istreambuf_iterator<char>());
                if (data.empty()) {
                    std::cerr << "Received empty data\n";
                    return;
                }
                data = data.substr(0, data.find('\n'));

                std::cout << "Received data: " << data << "\n";

                // Обрабатываем запрос
                try {
                    json request = json::parse(data);
                    if (!request.is_object()) {
                        throw std::runtime_error("Request is not a JSON object");
                    }

                    if (!request.contains("type") || !request["type"].is_string()) {
                        throw std::runtime_error("Invalid request: 'type' field missing or not a string");
                    }

                    std::string type = request["type"].get<std::string>();
                    std::string response;

                    // Обработка разных типов запросов
                    if (type == "list_files") {
                        auto file_list = node_.get_file_list();
                        response = file_list.dump() + "\n"; // Возвращаем список файлов
                    } else if (type == "initiate_upload") {
                        if (!request.contains("metadata")) {
                            throw std::runtime_error("Invalid request: 'metadata' field missing");
                        }
                        node_.sync_metadata(request["metadata"]); // Синхронизируем метаданные
                        response = "OK\n";
                    } else if (type == "upload") {
                        if (!request.contains("file_id") || !request.contains("part_index") ||
                            !request.contains("hash") || !request.contains("content")) {
                            throw std::runtime_error("Invalid upload request: missing required fields");
                        }
                        // Формируем структуру части файла
                        FilePart part{
                            request["file_id"].get<std::string>(),
                            request["part_index"].get<size_t>(),
                            "",
                            request["hash"].get<std::string>()
                        };
                        std::string encoded_content = request["content"].get<std::string>();
                        std::string content = base64_decode(encoded_content);
                        node_.add_file_part(part, content); // Сохраняем часть
                        response = "OK\n";
                    } else if (type == "download") {
                        if (!request.contains("file_id") || !request["file_id"].is_string()) {
                            throw std::runtime_error("Invalid download request: 'file_id' missing or not a string");
                        }
                        std::string file_id = request["file_id"].get<std::string>();
                        auto parts = node_.get_file_parts(file_id); // Получаем части файла
                        std::cout << "Returning " << parts.size() << " parts for file_id: " << file_id << "\n";
                        json parts_json = json::array();
                        for (const auto& part : parts) {
                            std::ifstream part_file(part.file_path, std::ios::binary);
                            if (!part_file) {
                                std::cerr << "Failed to read part file: " << part.file_path << "\n";
                                continue;
                            }
                            std::string content((std::istreambuf_iterator<char>(part_file)), std::istreambuf_iterator<char>());
                            part_file.close();
                            std::string encoded_content = base64_encode(content);
                            std::cout << "Sending part " << part.part_index << " for " << file_id
                                      << ", size: " << content.size() << " bytes, encoded size: " << encoded_content.size() << " bytes\n";
                            json part_json;
                            part_json["part_index"] = part.part_index;
                            part_json["file_path"] = part.file_path;
                            part_json["hash"] = part.hash;
                            part_json["content"] = encoded_content;
                            parts_json.push_back(part_json);
                        }
                        response = parts_json.dump(2) + "\n"; // Возвращаем части
                    } else if (type == "delete") {
                        if (!request.contains("file_id") || !request["file_id"].is_string()) {
                            throw std::runtime_error("Invalid delete request: 'file_id' missing or not a string");
                        }
                        std::string file_id = request["file_id"].get<std::string>();
                        node_.remove_file(file_id); // Удаляем файл
                        response = "OK\n";
                    } else {
                        response = "Unknown request type\n";
                    }

                    // Отправляем ответ клиенту
                    auto response_ptr = std::make_shared<std::string>(response);
                    boost::asio::async_write(*socket, boost::asio::buffer(*response_ptr),
                        [socket, response_ptr](const boost::system::error_code& error, std::size_t bytes_written) {
                            if (error) {
                                std::cerr << "Write error: " << error.message() << "\n";
                            } else {
                                std::cout << "Sent response: " << bytes_written << " bytes\n";
                            }
                        });
                } catch (const std::exception& e) {
                    std::cerr << "Error processing request: " << e.what() << "\n";
                    std::string response = "Error: " + std::string(e.what()) + "\n";
                    auto response_ptr = std::make_shared<std::string>(response);
                    boost::asio::async_write(*socket, boost::asio::buffer(*response_ptr),
                        [socket, response_ptr](const boost::system::error_code& error, std::size_t bytes_written) {
                            if (error) {
                                std::cerr << "Write error: " << error.message() << "\n";
                            } else {
                                std::cout << "Sent error response: " << bytes_written << " bytes\n";
                            }
                        });
                }
            });
    }

    boost::asio::io_context io_context_; // Контекст для сетевых операций
    tcp::acceptor acceptor_; // Объект для приема подключений
    NodeLogic& node_; // Ссылка на логику узла
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./psdss_server start <port>\n";
        return 1;
    }

    std::string command = argv[1];
    if (command != "start") {
        std::cerr << "Unknown command: " << command << "\n";
        return 1;
    }

    // Запускаем сервер
    try {
        short port = std::stoi(argv[2]);
        std::string node_address = "127.0.0.1:" + std::to_string(port);
        NodeLogic node(node_address);
        node.start();
        Server server("127.0.0.1", port, node);
        std::cout << "Server started on 127.0.0.1:" << port << "\n";
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}