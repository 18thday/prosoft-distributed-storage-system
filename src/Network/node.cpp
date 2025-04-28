#include <iostream>
#include "boost/asio.hpp"

#include "node.h"
#include "client.h"
// #include "../Business logic/Kernel/node_logic.h"
#include "../Business logic/Kernel/node_logic.cpp"

namespace net = boost::asio;
using net::ip::tcp;
using net::ip::udp;

static int server_loop(int port);

Node::Node(int port) : port{port}
{
    thread = std::thread([this]() { this->server_loop(); });
}

Node::Node(int port, Adress adress) : port{port}
{
    thread = std::thread([this]() { this->server_loop(); });
    connect_to_cluster(adress);
}

void Node::connect_to_cluster(Adress adress) {
    Client client(adress.ip, adress.port);
    client.connect();

    client.send("{\"type\":\"node_connected\"}");
    std::string ans = client.receive();

    std::cout << "Cluster said - " << ans << std::endl;

    client.close();
}

int Node::server_loop()
{
    std::cout << "Opening port" << port << std::endl;
    net::io_context io_context;
    tcp::endpoint endpoint(tcp::v4(), port);
    tcp::acceptor acceptor(io_context, endpoint, port);
    tcp::socket socket = tcp::socket(io_context);

    while (true)
    {
        std::cout << "Waiting for connection..." << std::endl;
        boost::system::error_code ec;
        acceptor.accept(socket, ec);

        if (ec) {
            std::cout << "Can't accept connection: " << ec.message() << std::endl;
            return 1;
        }

        net::streambuf stream_buf;
        net::read_until(socket, stream_buf, '\0', ec);
        std::string client_data{std::istreambuf_iterator<char>(&stream_buf),
                                std::istreambuf_iterator<char>()};

        std::cout << "Client said: " << client_data << std::endl;

        tcp::endpoint remote_endpoint = socket.remote_endpoint();
        Adress address(remote_endpoint.address().to_string(), remote_endpoint.port());

        std::string answer = process_message(client_data, address.to_string());

        // answer to client
        socket.write_some(net::buffer(answer), ec);

        if (ec) {
            std::cout << "Error sending data" << std::endl;
            return 1;
        }

        socket.close();
    }
}