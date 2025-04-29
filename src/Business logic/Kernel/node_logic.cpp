#pragma once
#include <string>
#include <unordered_set>
#include <iterator>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

#include "../../Network/client.h"
#include "json_messages.cpp"

std::string process_message(std::string&);

std::unordered_set<std::string> cluster_nodes;

namespace pt = boost::property_tree;



void connect_to_cluster(Address cluster_address, Address local_address) {
    Client::send_message(cluster_address, node_connected_message(local_address));
}


/**
 * @brief 
 * @param msg json сообщение, пришедшее узлу
 * @param address адрес отправителя, например "127.0.0.1:1337"
 * @return ответ отправителю в виде json строки
 */
std::string process_message(std::string& msg)
{
    pt::ptree json = to_ptree(msg);
    std::string type = json.get<std::string>("type");
    std::cout << "Packet type = " << type << '\n'; 
    if (type == "node_connected")
    {
        std::string new_address = json.get<std::string>("node_address");
        cluster_nodes.insert(new_address);

        // формируем json {nodes : [address1, address2...]}
        pt::ptree pt;
        pt.add("type", "nodes_cluster_list");

        pt::ptree nodes = container_to_ptree(cluster_nodes);
        pt.add_child("nodes", nodes); // {"list": ["str1", "str2", ...]}

        std::ostringstream oss;
        boost::property_tree::write_json(oss, pt);
        Client::brodcast_message(cluster_nodes, oss.str());
        return "";
    }
    if (type == "nodes_cluster_list")
    {
        pt::ptree nodes = json.get_child("nodes");
        cluster_nodes = ptree_to_container<std::unordered_set<std::string>>(nodes);
    }
    else if (type == "none")
        return "";

    return "{'type':'none'}";    
}