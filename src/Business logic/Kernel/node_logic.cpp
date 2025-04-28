#pragma once
#include <string>

/**
 * @brief 
 * @param msg json сообщение, пришедшее узлу
 * @param address адрес отправителя, например "127.0.0.1:1337"
 * @return ответ отправителю в виде json строки
 */
std::string process_message(std::string& msg, std::string address)
{
    return "i dont know what to say...";    
}