# Сбор всех заголовочных и исходных файлов GUI
# file(GLOB GUI_HEADERS *.h)
# file(GLOB GUI_SOURCES *.cpp)

# # Создание исполняемого файла psdss_gui
# add_executable(psdss_gui ${GUI_HEADERS} ${GUI_SOURCES})

# # Установка директорий для включения заголовков
# target_include_directories(psdss_gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Линковка с Qt и бизнес-логикой
# target_link_libraries(psdss_gui
#     PRIVATE Qt5::Widgets
#     PRIVATE business_logic
# )
