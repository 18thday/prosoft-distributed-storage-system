cmake_minimum_required(VERSION 3.28)
project(prosoft_distributed_storage_system)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system filesystem)

include_directories(${BOOST_INCLUDE_DIRS})

add_executable(prosoft_distributed_storage_system main.cpp
    src/Storage/storageclient.h src/Storage/storageclient.cpp
    src/Storage/filesplitter.h src/Storage/filesplitter.cpp)

target_link_libraries(prosoft_distributed_storage_system ${Boost_LIBRARIES})
