cmake_minimum_required(VERSION 3.28)
project(prosoft_distributed_storage_system)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

include_directories(${BOOST_INCLUDE_DIRS})

add_executable(prosoft_distributed_storage_system main.cpp
    src/Storage/storageclient.h src/Storage/storageclient.cpp
    src/Storage/filesplitter.h src/Storage/filesplitter.cpp
    src/Storage/hashsummmanager.h src/Storage/hashsummmanager.cpp
    src/Storage/storage.h src/Storage/storage.cpp)

add_executable(psdss_gui
    gui/mainwindow.cpp
    gui/main_gui.cpp
)

target_link_libraries(prosoft_distributed_storage_system ${Boost_LIBRARIES})

target_include_directories(psdss_gui PRIVATE ${CMAKE_SOURCE_DIR}/gui)

target_link_libraries(psdss_gui
    Qt5::Widgets
)
