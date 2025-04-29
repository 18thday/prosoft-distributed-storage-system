cmake_minimum_required(VERSION 3.10)

project(psdss)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)  # Автоматическая обработка Qt moc
set(CMAKE_AUTORCC ON)  # Автоматическая обработка ресурсов Qt
set(CMAKE_AUTOUIC ON)  # Автоматическая обработка .ui файлов Qt

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Boost 1.71 REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

set(CORE_SOURCES
    src/BusinessLogic/Kernel/node_logic.cpp
    src/BusinessLogic/client_app.cpp
    src/Network/network.cpp
    src/Storage/storage.cpp
)

set(CORE_HEADERS
    src/BusinessLogic/Kernel/node_logic.h
    src/BusinessLogic/client_app.h
    src/Network/network.h
    src/Storage/storage.h
    src/Storage/types.h
)

add_library(core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

target_include_directories(core PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/BusinessLogic
    ${CMAKE_SOURCE_DIR}/src/BusinessLogic/Kernel
    ${CMAKE_SOURCE_DIR}/src/Network
    ${CMAKE_SOURCE_DIR}/src/Storage
)

target_link_libraries(core
    Qt5::Core
    Boost::system
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
    nlohmann_json::nlohmann_json
    stdc++fs
)

set(GUI_SOURCES
    gui/mainwindow.cpp
    gui/main_gui.cpp
)

set(GUI_HEADERS
    gui/mainwindow.hpp
)

add_executable(psdss_gui ${GUI_SOURCES} ${GUI_HEADERS})

target_include_directories(psdss_gui PRIVATE
    ${CMAKE_SOURCE_DIR}/gui
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/BusinessLogic
    ${CMAKE_SOURCE_DIR}/src/BusinessLogic/Kernel
    ${CMAKE_SOURCE_DIR}/src/Network
    ${CMAKE_SOURCE_DIR}/src/Storage
)

target_link_libraries(psdss_gui
    Qt5::Widgets
    core
)

add_executable(psdss_server
    src/Network/main_server.cpp
)

target_include_directories(psdss_server PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/BusinessLogic
    ${CMAKE_SOURCE_DIR}/src/BusinessLogic/Kernel
    ${CMAKE_SOURCE_DIR}/src/Network
    ${CMAKE_SOURCE_DIR}/src/Storage
)

target_link_libraries(psdss_server
    core
)
