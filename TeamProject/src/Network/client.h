#pragma once
#include <string>
#include <unordered_set>
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

    boost::asio::ip::tcp::socket& get_socket();

    static void send_message(Address address, std::string msg);
    static std::string send_message_and_recieve_response(Address address, std::string msg);
    static void brodcast_message(std::unordered_set<std::string> addresses, std::string msg);
    static bool is_tcp_connection_possible(Address address);

private:
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::ip::tcp::socket socket_;
    Address address;
};