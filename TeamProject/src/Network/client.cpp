#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <string>
#include <string_view>

#include "client.h"

namespace net = boost::asio;
using net::ip::tcp;

using namespace std::literals;

Client::Client(Address address)
    : io_context_(net::io_context()), 
        resolver_(io_context_),
        socket_(io_context_),
        address(address)
{}

void Client::connect() {
    boost::system::error_code resolve_ec;
    boost::asio::ip::tcp::resolver::results_type endpoints =
        resolver_.resolve(address.ip, std::to_string(address.port), resolve_ec);

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
        std::cout << "Connected to server" << std::endl;
    }

}

void Client::send(const std::string& message) {
    // Добавляем терминальный символ
    boost::asio::write(socket_, boost::asio::buffer(message));
}

std::string Client::receive() {
    boost::asio::streambuf buf;
    boost::system::error_code ec;
    
    // Читаем всё, что есть в сокете
    size_t bytes_read = boost::asio::read(socket_, buf, 
                                       boost::asio::transfer_at_least(1), ec);
    
    if (ec && ec != boost::asio::error::eof) {
        std::cerr << "Receive error: " << ec.message() << '\n';
        return "";
    }

    
    return std::string(boost::asio::buffers_begin(buf.data()),
                     boost::asio::buffers_begin(buf.data()) + buf.size());
}

void Client::close() {
    socket_.close();
}

boost::asio::ip::tcp::socket& Client::get_socket()
{
    return this->socket_;
}

void Client::send_message(Address address, std::string msg)
{
    Client c1(address);

    std::cout << "Connecting to server...\n";

    c1.connect();
    c1.send(msg);

    c1.close();
}

std::string Client::send_message_and_recieve_response(Address address, std::string msg)
{
    Client c1(address);

    std::cout << "Connecting to server...\n";

    c1.connect();
    c1.send(msg);

    std::cout << "Recieving from server...\n";
    std::string response = c1.receive();
    std::cout << "Server responded: " << response << '\n';
    c1.close();
    return response;
}

void Client::brodcast_message(std::unordered_set<std::string> addresses, std::string msg)
{
    for (const std::string& addr_str: addresses) {
        Address address(addr_str);
        // std::thread(send_message, address, msg);
        //TODO:async
        send_message(address, msg);
    }
}

