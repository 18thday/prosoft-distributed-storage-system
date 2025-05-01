#include <iostream>
#include <thread>
#include "boost/asio.hpp"

#include "packets.h"

namespace net = boost::asio;
using net::ip::tcp;

class Node {
public:
    const int port;
    Node() = delete;
    Node(int port) : port{port}
    {
        std::cout << "starting serv" << std::endl;
        net::io_context io_context;
        thread = std::thread(server_loop, port);
    }
    ~Node()
    {
        thread.detach();
    }


private:
    std::thread thread;

    static int server_loop(int port)
    {
        net::io_context io_context;
        tcp::endpoint endpoint(tcp::v4(), port);
        tcp::acceptor acceptor(io_context, endpoint, port);
        // acceptor.set_option(tcp::acceptor::reuse_address(true));
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

            // answer to client
            socket.write_some(net::buffer("Hello, I'm server!\n"), ec);

            if (ec) {
                std::cout << "Error sending data" << std::endl;
                return 1;
            }

            socket.close();
        }
    }
};

int main()
{
    Node s1(1337);
    sleep(999999999);
}