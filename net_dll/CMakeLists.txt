add_library(tcp SHARED tcp.cpp)
target_link_libraries(tcp net_api)

add_library(websocket SHARED websocket.cpp)
target_link_libraries(websocket net_api)
