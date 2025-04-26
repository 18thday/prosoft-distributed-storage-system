cmake_minimum_required(VERSION 3.10)
project(psdss_gui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(psdss_gui
    gui/mainwindow.cpp
    gui/main_gui.cpp
)

target_include_directories(psdss_gui PRIVATE ${CMAKE_SOURCE_DIR}/gui)

target_link_libraries(psdss_gui
    Qt5::Widgets
)