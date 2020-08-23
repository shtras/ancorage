#file(GLOB UTILS_SOURCES Utils/*.cpp)
#file(GLOB UTILS_INCLUDES Utils/*.h)

#file(GLOB ENGINE_SOURCES Job/*.cpp Input/*.cpp Graph/*.cpp)
#file(GLOB ENGINE_INCLUDES Job/*.h Input/*.h Graph/*.h)

#add_library(Utils ${UTILS_SOURCES} ${UTILS_INCLUDES})
#add_library(Engine ${ENGINE_SOURCES} ${ENGINE_INCLUDES})

#target_link_libraries(Engine OpenSSL::SSL ${FFMPEG_LIBRARIES} ${GST_LIBRARIES})
#target_include_directories(Engine PRIVATE ${GST_INCLUDE_DIRS})
#target_compile_options(Engine PRIVATE ${GST_CFLAGS})
#target_link_options(Engine PRIVATE ${GST_LIBRARY_DIRS})

add_executable(Ancorage WIN32 main.cpp)

target_link_libraries(Ancorage Threads::Threads)
