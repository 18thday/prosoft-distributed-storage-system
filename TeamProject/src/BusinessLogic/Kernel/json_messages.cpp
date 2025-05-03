#include <string>
#include <iostream>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "../../Network/utils.cpp"

namespace pt = boost::property_tree;

// === Функции для работы с json/ptree =====================

std::string to_json(pt::ptree pt)
{
    std::stringstream str;
    boost::property_tree::json_parser::write_json(str, pt);
    return str.str();
}

pt::ptree to_ptree(std::string src)
{
    std::stringstream ss(src);
    pt::ptree jsontree;
    boost::property_tree::read_json(ss, jsontree);
    std::cout << "converted to ptree" << '\n';
    return jsontree;
}

// ptree вида ["","",...]
template <typename Container>
pt::ptree container_to_ptree(const Container& strings) {
    boost::property_tree::ptree pt;
    boost::property_tree::ptree children;

    for (const auto& str : strings) {
        children.push_back({"", boost::property_tree::ptree(str)});
    }

    return children;
}

// ptree вида ["","",...]
template <typename Container>
Container ptree_to_container(pt::ptree& pt) {
    Container result;

    for (const auto& item : pt) {
        result.insert(result.end(), item.second.get_value<std::string>());
    }

    return result;
}

// === Функции для формирования сетевых json пакетов =====================

//"{ "type" : "node_connected", "node_adress" : local_ip:port }"
std::string node_connected_message(Address local_address)
{
    pt::ptree pt;
    pt.add("type","node_connected");
    pt.add("node_address", local_address.to_string());

    return to_json(pt);
}