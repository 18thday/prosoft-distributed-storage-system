#pragma once
#include <string>
#include <iostream>
#include "boost/asio.hpp"

struct Address
{
    std::string ip;
    int port;
    Address() = delete; 
    Address(std::string ip, int port)
        :ip(ip), port(port)
    {
        boost::system::error_code ec;
        boost::asio::ip::make_address_v4( ip, ec );
        if ( ec )
            std::cerr << ec.message( ) << std::endl;
    }
    Address(std::string ip_and_port)
    {
        size_t colon_pos = ip_and_port.find(':');
        ip = ip_and_port.substr(0, colon_pos);
        port = std::stoi(ip_and_port.substr(colon_pos + 1));

        boost::system::error_code ec;
        boost::asio::ip::make_address_v4( ip, ec );
        if ( ec )
            std::cerr << ec.message( ) << std::endl;
    } 

    std::string to_string()
    {
        return ip + ':' + std::to_string(port);
    }
};
