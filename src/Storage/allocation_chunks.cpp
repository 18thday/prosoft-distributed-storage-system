#include <iostream>
#include <vector>
#include <string>
//#include <boost/filesystem.hpp>

typedef unsigned int ip_t;

struct Chunk
{
    //boost::filesystem::path chunk_path;
    std::string chunk_path; // путь до части файла
    std::vector<ip_t> ip; // вектор узлов на которых хранится данная часть файла
};
/// @brief распределение частей файла по узлам (только двойное резервирование!)
/// @param all_ip - вектор адресов доступных узлоа
/// @param chunks - вектор частей файла
/// @return вектор распределения частей по узлам
std::vector<Chunk> allocation_chunks(std::vector<ip_t> all_ip, std::vector<std::string> chunks)
{
    int chunks_num = chunks.size();
    int ip_num = all_ip.size();
    int count = chunks_num / ip_num;
    int count_small = chunks_num % ip_num;

    std::vector<Chunk> result;
    int begin = 0;
    int end = 0;
    std::vector<int> parts; // разбивка частей на группы [0][6][12][17][22][27][32]

    parts.push_back(0);
    for(int i = 0; i < ip_num; i++)
    {
        parts.push_back(parts[i] + count + ((count_small) ? 1 : 0));
        if(count_small > 0)
        {
            count_small--;
        }
    }   

    for(int i = 0; i < chunks.size(); i++)
    {
        Chunk che;
        che.chunk_path = chunks[i];
        result.push_back(che);
    }
    for(int i = 0; i < ip_num; i++)
    {   if(i < ip_num - 1)
        {
            begin = parts[i];
            end = parts[i + 2];
            for(int k = begin; k < end; k++)
            {
                result[k].ip.push_back(all_ip[i]);
            }
        }else
        {   //хвост
            int begin_tail = parts[i];
            int end_tail = parts[i + 1];
            for(int k = begin_tail; k < end_tail; k++)
            {
                result[k].ip.push_back(all_ip[i]);
            }
            //голова
            int begin_head = parts[0];
            int end_head = parts[1];
            for(int k = begin_head; k < end_head; k++)
            {
                result[k].ip.push_back(all_ip[i]);
            } 
        }
    }
    return result;
}





int main() {


    std::vector<std::string> chunks_test = {"path1", "path2", "path3", "path4", "path5",
                                            "path6", "path7","path8","path9",
                                            "path10","path11","path12","path13","path14","path15","path16","path17","path18","path19",
                                            "path20","path21","path22","path23","path24","path25","path26","path27","path28","path29",
                                            "path30","path31","path32"};
    std::vector<ip_t> all_ip_test = {10101101, 10101102, 10101103, 10101104, 10101105, 10101106};

    std::vector<Chunk> result_test = allocation_chunks(all_ip_test, chunks_test);

    for(int i = 0; i < result_test.size(); i++)
    {
        std::cout << result_test[i].chunk_path << std::endl;
        std::cout << "-------------------------------" << std::endl;
         for(int j = 0; j < result_test[i].ip.size(); j++)
         {
                std::cout << result_test[i].ip[j] << std::endl;
         }
         std::cout << "______________________________" << std::endl;
    }
    

  return 0;


}