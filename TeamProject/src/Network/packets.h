#pragma once
#include <string>
#include <memory>
#include <iostream>
#include <boost/asio.hpp>

struct FileData {
    char emptyByte;
    char ipAddr[30];
    char fileName[256];
    unsigned char data[32768];
    size_t dataSize;
};

namespace net = boost::asio;
using net::ip::tcp;

enum PacketType : char 
{
    USER_CONNECTED,
    NODE_CONNECTED,
    FILE_PART,
    FILE_MAP,
    KEEP_ALIVE,
};

struct BasePacket
{
    PacketType type;
};

struct FilePartPacket : BasePacket
{
    int part_index;
    char file_name[100];
    char file_part[4096];
};

struct FileMapPacket : BasePacket
{
};

int get_packet_size(PacketType type)
{
    switch (type)
    {
        case USER_CONNECTED:
        case NODE_CONNECTED:
        case KEEP_ALIVE:
            return sizeof(BasePacket);
        case FILE_PART:
            return sizeof(FilePartPacket);
        case FILE_MAP:
            return sizeof(FileMapPacket);
        default:
            throw std::invalid_argument("no such case");
    }
}

std::shared_ptr<FileData> read_packet(tcp::socket& socket) {
    PacketType type;
    
    boost::system::error_code ec;
    size_t bytes_read = socket.read_some(
        net::buffer(&type, sizeof(type)), ec);

    // std::cout << "Readed packet type is : " << (int)type << '\n';

    if (ec && ec != net::error::would_block) {
        throw boost::system::system_error(ec);
    }

    auto pack = std::make_shared<FileData>();
    pack->emptyByte = type;
    // int rest_size = get_packet_size(type) - sizeof(type);
    int rest_size = sizeof(FileData) - sizeof(type);
    if (rest_size == 0)
        return pack;

    void* rest = (&pack->emptyByte) + sizeof(type);
    char buf[rest_size];

    std::cout << "Rest of packet size -> " << rest_size << '\n';
    
    bytes_read = socket.read_some(
        net::buffer(&buf, rest_size), ec);

    std::memcpy(rest, buf, rest_size);

    std::cout << "Read rest of packet\n";

    if (ec && ec != net::error::would_block) {
        throw boost::system::system_error(ec);
    }

    return pack;
}