#include <iostream>
#include <unistd.h>
#include "node.h"

int main(int argc, char** argv)
{
    if (argc != 2 && argc != 4) {
        std::cout << "Usage: " << argv[0] << " <server Port> [<cluster IP> <cluster Port>]" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);

    if (argc == 2)
    {
        Node s1(port);
        s1.thread.join();
        return 0;
    }

    std::string cluster_ip = argv[2];
    int cluster_port = std::stoi(argv[3]);
    {
        Node s1(port, Adress(cluster_ip, cluster_port));
        s1.thread.join();
    }
}