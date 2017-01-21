add_library(tcp SHARED tool.h socket.h tcp.cpp)
target_link_libraries(tcp ${Boost_LIBRARIES} tool)

add_library(websocket SHARED tool.h socket.h websocket.cpp)
target_link_libraries(websocket ${Boost_LIBRARIES} tool)