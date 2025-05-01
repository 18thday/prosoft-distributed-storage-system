# Добавление подкаталогов с библиотеками
add_subdirectory(Network)
#add_subdirectory(Storage)
#add_subdirectory("Business logic")

add_library(storage
    ${CMAKE_CURRENT_SOURCE_DIR}/Storage/storage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Storage/storage.cpp)

target_include_directories(storage
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
