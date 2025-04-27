#include <iostream>
#include <fstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/asio.hpp>

namespace net = boost::asio;
using net::ip::tcp;
namespace pt = boost::property_tree;

using namespace std;

enum PacketType
{
    USER_CONNECT,
    NODE_CONNECT,
    FILE_PART,
    KEEP_ALIVE,
};


int send_user_connection_request(tcp::socket socket)
{
    boost::system::error_code ec;
    socket.write_some(net::buffer("Hello, I'm client!\n"sv), ec);
    if (ec) {
        std::cout << "Error sending data"sv << std::endl;
        return 1;
    }
}

void send_node_connection_request()
{
    
}

// Пример парсинга json
int json_parse()
{
    boost::property_tree::ptree jsontree;
    boost::property_tree::read_json("test.json", jsontree);

    int v0 = jsontree.get<int>("a");
    std::cout << v0 << '\n';

    auto v2 = jsontree.get_child("b");
    std::cout << (v2.get<std::string>("d")) << '\n';

    int v1 = jsontree.get<int>("c");
    std::cout << v1 << '\n';
}
