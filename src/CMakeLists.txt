file(GLOB GUI_SOURCES GUI/*.cpp)
file(GLOB GUI_RESOURCES Resources/*.rc)
file(GLOB GUI_INCLUDES GUI/*.h)

file(GLOB ENGINE_SOURCES XInput/*.cpp Engine/*.cpp BLE/*.cpp Utils/*.cpp)
file(GLOB ENGINE_INCLUDES XInput/*.h Enigne/*.h BLE/*.h Utils/*.h)

add_library(GUI ${GUI_SOURCES} ${GUI_INCLUDES})
add_library(Engine ${ENGINE_SOURCES} ${ENGINE_INCLUDES})

#set_source_files_properties(${GUI_RESOURCES} PROPERTIES LANGUAGE RC)
source_group("Resources" FILES ${GUI_RESOURCES})
#target_link_libraries(Engine OpenSSL::SSL ${FFMPEG_LIBRARIES} ${GST_LIBRARIES})
#target_include_directories(Engine PRIVATE ${GST_INCLUDE_DIRS})
#target_compile_options(Engine PRIVATE ${GST_CFLAGS})
#target_link_options(Engine PRIVATE ${GST_LIBRARY_DIRS})

target_link_libraries(Engine Xinput)

add_executable(Ancorage WIN32 main.cpp ${GUI_RESOURCES})

target_link_libraries(Ancorage GUI Engine Threads::Threads)
