cmake_minimum_required(VERSION 3.10)
project(prosoft_distributed_storage_system)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск необходимых пакетов
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(Qt5 REQUIRED COMPONENTS Widgets)

# Добавление подкаталогов
add_subdirectory(src)
add_subdirectory(gui)

# Основной исполняемый файл
add_executable(prosoft_distributed_storage_system main.cpp)
target_link_libraries(prosoft_distributed_storage_system PRIVATE business_logic network storage ${Boost_LIBRARIES})
