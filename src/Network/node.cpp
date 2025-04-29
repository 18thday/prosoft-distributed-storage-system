#include <iostream>
#include "boost/asio.hpp"
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "node.h"
#include "client.h"
// #include "../Business logic/Kernel/node_logic.h"
#include "../Business logic/Kernel/node_logic.cpp"

namespace net = boost::asio;
using net::ip::tcp;
using net::ip::udp;

static int server_loop(int port);
static std::string get_local_ip();

Node::Node(int port) : port{port}
{
    cluster_nodes.insert(get_local_ip());
    thread = std::thread([this]() { this->server_loop(); });
}

Node::Node(int port, Address address) : port{port}
{
    cluster_nodes.insert(get_local_ip());
    thread = std::thread([this]() { this->server_loop(); });
    sleep(1);
    connect_to_cluster(address);
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
        Address address(remote_endpoint.address().to_string(), remote_endpoint.port());

        std::string answer = process_message(client_data, address);

        if (answer.length() == 0)
        {
            // answer = "{\"type\":\"\"}";
            socket.close();
            continue;
        }

        // answer to client
        socket.write_some(net::buffer(answer), ec);

        if (ec) {
            std::cout << "Error sending data" << std::endl;
            return 1;
        }

        socket.close();
    }
}

std::string get_local_ip() {
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr)) {
        perror("getifaddrs");
        std::cout << "Local ip not found\n";
        return "";
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        
        int family = ifa->ifa_addr->sa_family;
        if (family == AF_INET) { // IPv4
            char ip[INET_ADDRSTRLEN];
            auto addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, addr, ip, INET_ADDRSTRLEN);
            
            if (!strcmp(ip, "127.0.0.1")) continue; // Пропускаем loopback
            
            std::cout << "Local ip - " << ifa->ifa_name << ": " << ip << "\n";
            freeifaddrs(ifaddr);
            return ip;
        }
    }
    freeifaddrs(ifaddr);
}