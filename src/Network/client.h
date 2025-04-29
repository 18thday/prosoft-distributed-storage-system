#pragma once
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "utils.cpp"

class Client {
public:
    Client() = delete;
    Client(Address address);

    void connect(); 
    void close();

    void send(const std::string& message);
    std::string receive();

    static void send_message(Address address, std::string msg);
    static std::string send_message_and_recieve_response(Address address, std::string msg);

private:
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::ip::tcp::socket socket_;
    Address address;
};