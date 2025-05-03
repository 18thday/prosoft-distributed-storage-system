#include <iostream>
#include "boost/asio.hpp"
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "node.h"
#include "client.h"
#include "packets.h"
#include "../BusinessLogic/Kernel/node_logic.cpp"
//TODO: link storage
// #include "../Storage/storage.h"

namespace net = boost::asio;
using net::ip::tcp;
using net::ip::udp;

static int server_loop(int port);
static std::string get_local_ip();
void read_chunk(char first, tcp::socket& socket);

Node::Node(int port) : address(get_local_ip(), port)
{
    cluster_nodes.insert(address.to_string());
    thread = std::thread([this]() { this->server_loop(); });
}

Node::Node(int port, Address cluster_address) : address(get_local_ip(), port)
{
    cluster_nodes.insert(address.to_string());
    thread = std::thread([this]() { this->server_loop(); });

    sleep(0.1);
    connect_to_cluster(cluster_address, address);
}

int Node::server_loop()
{
    std::cout << "Opening port" << address.port << std::endl;
    net::io_context io_context;
    tcp::endpoint endpoint(tcp::v4(), address.port);
    tcp::acceptor acceptor(io_context, endpoint, address.port);
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

        size_t bytes_read = boost::asio::read(socket, stream_buf, 
            boost::asio::transfer_exactly(1), ec);

        char ch = stream_buf.sgetc();
        bool is_json = ch == '{';

        std::cout << "Is this json?: " << is_json << std::endl;

        std::string answer;
        // net::read_until(socket, stream_buf, '\0', ec);
        if (is_json)
        {
            bytes_read = boost::asio::read(socket, stream_buf, 
                boost::asio::transfer_at_least(1), ec);
            std::string client_data{std::istreambuf_iterator<char>(&stream_buf),
                std::istreambuf_iterator<char>()};
            std::cout << "Client said: " << client_data << std::endl;
            answer = process_message(client_data);
        }
        else 
        {
            auto file = read_packet(socket);
            std::cout << "Filename: " << file->fileName << '\n';
            // std::cout << "Content: " << file->data << '\n';
            //TODO: link storage
            // Storage::reciveData("", *file);
        }


        // tcp::endpoint remote_endpoint = socket.remote_endpoint();
        // Address address(remote_endpoint.address().to_string(), remote_endpoint.port());


        std::cout << "Answering to client with: " << answer << std::endl;

        if (answer.length() == 0)
        {
            // answer = "{\"type\":\"\"}";
            socket.close();
            continue;
        }

        // answer to client
        socket.write_some(net::buffer(answer), ec);

        std::cout << "Answered" << std::endl;

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

static std::string to_json_packet(pt::ptree pt)
{
    pt.add("address","node_connected");
    std::stringstream str;
    boost::property_tree::json_parser::write_json(str, pt);
    return str.str();
}
