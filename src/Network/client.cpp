
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <string>
#include <string_view>

#include "client.h"

namespace net = boost::asio;
using net::ip::tcp;

using namespace std::literals;

Client::Client(const std::string& host, const int port)
    : io_context_(net::io_context()), 
        resolver_(io_context_),
        socket_(io_context_),
        adress(host, port)
{}

void Client::connect() {
    boost::system::error_code resolve_ec;
    boost::asio::ip::tcp::resolver::results_type endpoints =
        resolver_.resolve(adress.ip, std::to_string(adress.port), resolve_ec);

    if (resolve_ec) {
        std::cout << "Resolution failed: " << resolve_ec.message() << std::endl;
        return;
    }

    // Подключаемся к первому доступному эндпоинту
    boost::asio::connect(socket_, endpoints);
    boost::system::error_code ec;
    if (ec) {
        std::cout << "Can't connect to server"sv << std::endl;
    }else
    {
        std::cout << "Connect to server" << std::endl;
    }

}

void Client::send(const std::string& message) {
    // Добавляем терминальный символ
    boost::asio::write(socket_, boost::asio::buffer(message + '\0'));
}

std::string Client::receive() {
    boost::asio::streambuf buf;
    boost::system::error_code ec;
    boost::asio::read_until(socket_, buf, '\0', ec);

    if (ec.failed())
        std::cerr << ec.message() << '\n';

    // Преобразуем данные в строку
    std::string data;
    std::istream is(&buf);
    std::getline(is, data);
    return data;
}

void Client::close() {
    socket_.close();
}