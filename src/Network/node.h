#pragma once
#include <thread>
#include "utils.cpp"

class Node {
public:
    const int port;
    std::thread thread;
    Node() = delete;
    Node(int port);
    Node(int port, Adress cluster_address);

private:
    int server_loop();
    void connect_to_cluster(Adress);
};