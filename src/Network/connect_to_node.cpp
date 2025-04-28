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

    Client c1(ip, port);

    std::cout << "Connecting to server...\n";

    c1.connect();
    c1.send(msg);

    std::cout << "Recieving from server...\n";
    std::cout << c1.receive() << '\n';
} 
