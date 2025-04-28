
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <string>
#include <string_view>

//#include "client.h"

namespace net = boost::asio;
using net::ip::tcp;

using namespace std::literals;

class Client {
    public:
        Client(boost::asio::io_context& io_context, const std::string& host, const std::string& port)
            : resolver_(io_context),
              socket_(io_context),
              host_(host),
              port_(port) {}
    
        void connect() {
            boost::asio::ip::tcp::resolver::results_type endpoints =
                resolver_.resolve(host_, port_);
    
            // Подключаемся к первому доступному эндпоинту
            boost::asio::connect(socket_, endpoints);
            boost::system::error_code ec;
            if (ec) {
                std::cout << "Can't connect to server"sv << std::endl;
            }else
            {
                std::cout << "Connect to server" << std::endl;
            }
    
        }
    
        void send(const std::string& message) {
            // Добавляем терминальный символ
            boost::asio::write(socket_, boost::asio::buffer(message + '\0'));
        }
    
        std::string receive() {
            boost::asio::streambuf buf;
            boost::asio::read_until(socket_, buf, '\0');
    
            // Преобразуем данные в строку
            std::string data;
            std::istream is(&buf);
            std::getline(is, data);
            return data;
        }
        boost::asio::ip::tcp::socket& get_socket() {
            return socket_;
        }
        void close() {
            socket_.close();
        }
    
    private:
        boost::asio::ip::tcp::resolver resolver_;
        boost::asio::ip::tcp::socket socket_;
        std::string host_;
        std::string port_;
    };