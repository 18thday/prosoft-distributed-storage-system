#include <iostream>
#include <fstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/asio.hpp>

namespace net = boost::asio;
using net::ip::tcp;
namespace pt = boost::property_tree;

// Пример парсинга json
void json_parse()
{
    // из файла 
    {
        boost::property_tree::ptree jsontree;
        boost::property_tree::read_json("../temp/test.json", jsontree);

        int v0 = jsontree.get<int>("a");
        std::cout << v0 << '\n';

        auto v2 = jsontree.get_child("b");
        std::cout << (v2.get<std::string>("d")) << '\n';

        int v1 = jsontree.get<int>("c");
        std::cout << v1 << '\n';
    }

    // из строки
    {
        boost::property_tree::ptree jsontree;
        std::stringstream src("{ \"first\" : \"last\" }");
        boost::property_tree::read_json(src, jsontree);

        std::string v0 = jsontree.get<std::string>("first");
        std::cout << v0 << '\n';
    }
}

// Пример создания json
void json_create()
{
    boost::property_tree::ptree jsontree;
    jsontree.add("a","string");
    jsontree.add("b",42);

    boost::property_tree::ptree leaf;
    leaf.add("d","nested object");

    jsontree.add_child("c",leaf);

    std::stringstream str;
    boost::property_tree::json_parser::write_json(str, jsontree);
    std::cout << str.str() << '\n';

    // в таком случае указывается имя файла
    // boost::property_tree::json_parser::write_json("file", jsontree);
}

int main()
{
    json_parse();
    json_create();
}