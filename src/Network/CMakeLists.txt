cmake_minimum_required(VERSION 3.22.1)
project(psdss_network)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system)

include_directories(${BOOST_INCLUDE_DIRS})

add_executable(node node.cpp)
add_executable(send_packet send_packet.cpp)

target_link_libraries(${Boost_LIBRARIES})
