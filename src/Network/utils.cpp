#pragma once
#include <string>
#include <iostream>
#include "boost/asio.hpp"

struct Adress
{
    std::string ip;
    int port;
    
    Adress(std::string ip, int port)
        :ip(ip), port(port)
    {
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
