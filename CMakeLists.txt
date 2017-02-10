cmake_minimum_required(VERSION 3.7)

project(main)

include(${CMAKE_SOURCE_DIR}/cmake/cotire.cmake)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()


find_program(CCACHE_FOUND ccache)

if(CCACHE_FOUND)

        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)

        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)

endif(CCACHE_FOUND)

find_package(Boost REQUIRED COMPONENTS system filesystem log)

include_directories(${Boost_INCLUDE_DIR})

link_directories(${Boost_LIBRARY_DIRS})

link_libraries(${Boost_LIBRARIES})

IF (UNIX)  

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


function(lsy_add_directoy dir)

include_directories(${dir})
foreach(type "cpp" "c" "h" "hpp")
file(GLOB_RECURSE FILE_TMP "${dir}/*.${type}")
set(SRC "${SRC};${FILE_TMP}")
endforeach(type)
add_library(${dir} SHARED ${SRC})

cotire(net_api)

endfunction(lsy_add_directoy)

lsy_add_directoy(net_api)
lsy_add_directoy(network net_api)
target_link_libraries(network net_api)

add_subdirectory(net_dll)

find_package(SQLite3 REQUIRED)

lsy_add_directoy(database)

target_link_libraries(database ${SQLITE3_LIBRARIES})

add_library(engine SHARED engine.cpp engine.h)

target_link_libraries(engine database network )



add_executable(main main.cpp)

target_link_libraries(main net_api network database engine )

#set(wxWidgets_FIND_COMPONENTS "core;base;xrc")
#find_package(wxwidgets)
find_package(wxWidgets REQUIRED net gl core base xrc)
include(${wxWidgets_USE_FILE})
add_executable(client client.cpp client.h)
target_link_libraries(client net_api network ${wxWidgets_LIBRARIES})
