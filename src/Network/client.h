#pragma once
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "utils.cpp"

class Client {
public:
    Client() = delete;
    Client(const std::string& host, const int port);

    void connect(); 
    void close();

    void send(const std::string& message);
    std::string receive();

private:
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::ip::tcp::socket socket_;
    Adress adress;
};