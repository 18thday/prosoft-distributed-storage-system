#include <iostream>
#include <string>
#include <unordered_set>
#include <iterator>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

#include "packets.h"
#include "client.h"
#include "../BusinessLogic/Kernel/json_messages.cpp"
//TODO: link storage
// #include "../Storage/storage.h"

namespace pt = boost::property_tree;

void send_file_struct(Address address, FileData pack)
{
    Client c1(address);
    std::cout << "Connecting to server...\n";
    c1.connect();

    // FileData pack;
    // memcpy(pack.ipAddr, "127.0.0.1:1337", sizeof(pack.ipAddr));
    // memcpy(pack.ipAddr, "myfile.txt", sizeof(pack.fileName));
    // memcpy(pack.ipAddr, "127.0.0.1:1337", sizeof(pack.data));
    // pack.dataSize = 20;

    tcp::socket& socket = c1.get_socket();
    boost::system::error_code ec; 
    socket.write_some(net::buffer(&pack, sizeof(pack)), ec);
    std::cout << "Written bytes -> " << sizeof(pack) << std::endl;

    if (ec) {
        std::cout << "Error sending data" << std::endl;
        return;
    }

    socket.close();
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <server IP> <server Port> <json string or file_path (to upload file)>" << std::endl;
        return 1;
    }

    std::string ip = argv[1];
    int port = std::stoi(argv[2]);
    std::string msg = argv[3];

    if (msg[0] == '{')
        std::string response = 
            Client::send_message_and_recieve_response(Address(ip, port), msg);
    else
    {
        pt::ptree request;
        request.add("type","get_nodes_cluster_list");
        std::ostringstream oss;
        boost::property_tree::write_json(oss, request);
        std::string response = 
            Client::send_message_and_recieve_response(Address(ip, port), oss.str());

        pt::ptree json = to_ptree(response);
        pt::ptree nodes = json.get_child("nodes");
        std::unordered_set<std::string> cluster = ptree_to_container<std::unordered_set<std::string>>(nodes);

        std::string file_path = argv[3];

        //TODO: link storage
        // std::string file_info = Storage::splitFile(cluster, file_path, "", 32768);
        // FileData fd = Storage::uploadData(file_info);
        // while (fd.fileName != "")
        // {
        //     send_file_struct(Address(ip,port), fd);
        //     fd = Storage::uploadData(file_info);
        // }
        
    }
} 
