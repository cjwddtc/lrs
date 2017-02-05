cmake_minimum_required(VERSION 3.7)

project(main)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sqlite)



set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(cotire)



find_program(CCACHE_FOUND ccache)

if(CCACHE_FOUND)

        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)

        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)

endif(CCACHE_FOUND)

find_package(Threads REQUIRED)

link_libraries(Threads::Threads)
 


find_package(Boost REQUIRED COMPONENTS system filesystem log)

include_directories(${Boost_INCLUDE_DIR})

link_directories(${Boost_LIBRARY_DIRS})

link_libraries(${Boost_LIBRARIES})

message(${Boost_LIBRARIES})

IF (UNIX)  

link_libraries(${CMAKE_DL_LIBS})

ADD_DEFINITIONS(-fPIC)

ADD_DEFINITIONS(-fvisibility=hidden)

endif(UNIX)



IF (WIN32)  

add_compile_options("/EHsc")

link_libraries(ws2_32.lib mswsock.lib)

endif(WIN32)



include_directories(db_tools)

add_subdirectory(db_tools)

#db_gen_file(asd ${CMAKE_SOURCE_DIR})



include_directories(net_api)

aux_source_directory(net_api NET_API_SRC)

add_library(net_api SHARED ${NET_API_SRC})

target_link_libraries(net_api)

cotire(net_api)



include_directories(network)

aux_source_directory(network NETWORK_SRC)

add_library(network SHARED ${NETWORK_SRC})

target_link_libraries(network net_api )

cotire(network)



add_subdirectory(net_dll)



add_subdirectory(sqlite)



include_directories(database)

aux_source_directory(database DATABASE_SRC)

add_library(database SHARED ${DATABASE_SRC})

target_link_libraries(database libsqlite)

cotire(database)



add_library(engine SHARED engine.cpp engine.h)

target_link_libraries(engine database network )



add_executable(main main.cpp)

target_link_libraries(main net_api network database engine )


add_executable(client client.cpp)

target_link_libraries(client net_api network )

IF (UNIX)  

target_link_libraries(client nana X11 pthread rt Xft png asound)

endif(UNIX)