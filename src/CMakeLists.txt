file(GLOB GUI_SOURCES GUI/*.cpp)
file(GLOB GUI_RESOURCES Resources/*.rc)
file(GLOB GUI_INCLUDES GUI/*.h)
source_group("Resources" FILES ${GUI_RESOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${GUI_SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Header Files" FILES ${GUI_INCLUDES})
add_library(GUI ${GUI_SOURCES} ${GUI_INCLUDES})


file(GLOB ENGINE_SOURCES BLE/*.cpp XInput/*.cpp Utils/*.cpp)
file(GLOB ENGINE_INCLUDES BLE/*.h XInput/*.h Utils/*.h)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${ENGINE_SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Header Files" FILES ${ENGINE_INCLUDES})
add_library(Engine ${ENGINE_SOURCES} ${ENGINE_INCLUDES})

target_link_libraries(Engine Xinput)
add_executable(Ancorage WIN32 main.cpp ${GUI_RESOURCES})
target_link_libraries(Ancorage GUI Engine Threads::Threads)
