add_library(tcp SHARED tcp.cpp)
target_link_libraries(tcp ${Boost_LIBRARIES} net_api)

add_library(websocket SHARED websocket.cpp)
target_link_libraries(websocket ${Boost_LIBRARIES} net_api)
