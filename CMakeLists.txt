cmake_minimum_required(VERSION 3.7)
project(main)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sqlite)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

find_package(Boost REQUIRED COMPONENTS system filesystem)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})


IF (UNIX)  
link_libraries(${CMAKE_DL_LIBS})
endif(UNIX)


add_subdirectory(sqlite)

add_library(tool SHARED tool.cpp)
target_link_libraries(tool ${Boost_LIBRARIES})

add_library(tcp SHARED tcp.cpp)
target_link_libraries(tcp ${Boost_LIBRARIES} tool)

add_library(websocket SHARED websocket.cpp)
target_link_libraries(websocket ${Boost_LIBRARIES} tool)

add_library(network SHARED server.cpp listener.cpp port.cpp message.cpp)
target_link_libraries(network tool)

add_library(database SHARED database.cpp)
target_link_libraries(database libsqlite)

add_executable(main main.cpp)
target_link_libraries(main tool network database)

IF (WIN32)  
target_link_libraries(tcp ws2_32.lib mswsock.lib)
target_link_libraries(tool ws2_32.lib mswsock.lib)
endif(WIN32)
