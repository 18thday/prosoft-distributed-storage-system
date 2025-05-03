cmake_minimum_required(VERSION 3.22.1)
project(psdss_network)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system)

include_directories(${BOOST_INCLUDE_DIRS})

add_executable(start_node start_node.cpp node.cpp client.cpp)
add_executable(connect_to_node connect_to_node.cpp client.cpp)

target_link_libraries(${Boost_LIBRARIES})
