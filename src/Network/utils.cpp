#pragma once
#include <string>
#include <iostream>
#include "boost/asio.hpp"

struct Address
{
    std::string ip;
    int port;
    
    Address(std::string ip, int port)
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
