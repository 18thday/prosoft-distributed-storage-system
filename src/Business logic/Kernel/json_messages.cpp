#include <string>
#include <iostream>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

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
    std::cout << "to_ptree function argument -> " << src << '\n';
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

//"{ "type" : "node_connected" }"
std::string node_connected_message()
{
    pt::ptree pt;
    pt.add("type","node-connected");

    return to_json(pt);
}