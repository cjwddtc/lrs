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
ADD_DEFINITIONS(-fPIC)
endif(UNIX)

IF (WIN32)  
link_libraries(ws2_32.lib mswsock.lib)
endif(WIN32)

include_directories(net_api)
aux_source_directory(net_api NET_API_SRC)
add_library(net_api ${NET_API_SRC})
target_link_libraries(net_api ${Boost_LIBRARIES})

include_directories(network)
aux_source_directory(network NETWORK_SRC)
add_library(network SHARED ${NETWORK_SRC})
target_link_libraries(network net_api)

add_subdirectory(net_dll)

add_subdirectory(sqlite)

include_directories(database)
aux_source_directory(database DATABASE_SRC)
add_library(database SHARED ${DATABASE_SRC})
target_link_libraries(database libsqlite)

add_library(engine SHARED engine.cpp engine.h)
target_link_libraries(engine database network)

add_executable(main main.cpp)
target_link_libraries(main net_api network database statement)
