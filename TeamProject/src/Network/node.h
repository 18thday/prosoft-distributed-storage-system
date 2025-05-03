#pragma once
#include <thread>
#include "utils.cpp"

class Node {
public:
    Address address;
    std::thread thread;
    Node() = delete;
    Node(int port);
    Node(int port, Address cluster_address);

private:
    int server_loop();
};