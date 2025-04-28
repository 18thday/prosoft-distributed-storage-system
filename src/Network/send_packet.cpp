#include <boost/asio.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <iostream>
#include <string>
#include <string_view>

#include "packets.h"

namespace net = boost::asio;
using net::ip::tcp;

using namespace std::literals;

struct a{
    int b;  
};

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Usage: "sv << argv[0] << " <server IP> <server Port>"sv << std::endl;
        return 1;
    }

    static const int port = std::stoi(argv[2]);

    // Создадим endpoint - объект с информацией об адресе и порте.
    // Для разбора IP-адреса пользуемся функцией net::ip::make_address.
    boost::system::error_code ec;
    auto endpoint = tcp::endpoint(net::ip::make_address(argv[1], ec), port);

    if (ec) {
        std::cout << "Wrong IP format"sv << std::endl;
        return 1;
    }

    net::io_context io_context;
    tcp::socket socket(io_context);
    socket.connect(endpoint, ec);

    if (ec) {
        std::cout << "Can't connect to server"sv << std::endl;
        return 1;
    }

    // BasePacket pack = {NODE_CONNECTED};
    FilePartPacket pack = {FILE_PART, 0, "myfile.txt", "1010100101010001"};

    socket.write_some(net::buffer(&pack, sizeof(pack)), ec);
    std::cout << "Written bytes -> "sv << sizeof(pack) << std::endl;
    if (ec) {
        std::cout << "Error sending data"sv << std::endl;
        return 1;
    }

    net::streambuf stream_buf;
    net::read_until(socket, stream_buf, '\n', ec);
    std::string server_data{std::istreambuf_iterator<char>(&stream_buf),
                            std::istreambuf_iterator<char>()};

    if (ec) {
        std::cout << "Error reading data"sv << std::endl;
        return 1;
    }

    std::cout << "Server responded: "sv << server_data << std::endl;
} 
