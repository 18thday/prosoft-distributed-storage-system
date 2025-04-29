# Сбор всех заголовочных и исходных файлов
file(GLOB NETWORK_HEADERS *.h)
file(GLOB NETWORK_SOURCES *.cpp)

# Создание статической библиотеки network
add_library(network STATIC ${NETWORK_HEADERS} ${NETWORK_SOURCES})

# Установка директорий для включения заголовков
target_include_directories(network PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Линковка с Boost
target_link_libraries(network PUBLIC ${Boost_LIBRARIES})
