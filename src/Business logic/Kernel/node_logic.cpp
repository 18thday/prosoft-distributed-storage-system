#pragma once
#include <string>
#include <unordered_set>
#include <iterator>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

#include "../../Network/client.h"
#include "json_messages.cpp"

std::string process_message(std::string&, Address);

std::unordered_set<std::string> cluster_nodes;

namespace pt = boost::property_tree;



void connect_to_cluster(Address address) {
    std::string resp = 
        Client::send_message_and_recieve_response(address, node_connected_message());

    std::cout << "Cluster said - " << resp << std::endl;
    process_message(resp, address);
}


/**
 * @brief 
 * @param msg json сообщение, пришедшее узлу
 * @param address адрес отправителя, например "127.0.0.1:1337"
 * @return ответ отправителю в виде json строки
 */
std::string process_message(std::string& msg, Address address)
{
    std::cout << "reading json msg :" << msg << "\n"; 
    pt::ptree json = to_ptree(msg);
    std::cout << "readed" << '\n'; 
    std::string type = json.get<std::string>("type");
    std::cout << "type = " << type << '\n'; 
    if (type == "node_connected")
    {
        cluster_nodes.insert(address.to_string());

        // формируем json {nodes : [address1, address2...]}
        pt::ptree pt;
        pt.add("type", "nodes_cluster_list");

        pt::ptree nodes = container_to_ptree(cluster_nodes);
        pt.add_child("nodes", nodes); // {"list": ["str1", "str2", ...]}

        std::ostringstream oss;
        boost::property_tree::write_json(oss, pt);
        return oss.str();
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