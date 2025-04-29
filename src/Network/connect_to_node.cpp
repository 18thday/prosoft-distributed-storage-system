#include <iostream>

#include "client.h"

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <server IP> <server Port> <json string>" << std::endl;
        return 1;
    }

    std::string ip = argv[1];
    int port = std::stoi(argv[2]);
    std::string msg = argv[3];

    std::string response = 
        Client::send_message_and_recieve_response(Address(ip, port), msg);
} 
