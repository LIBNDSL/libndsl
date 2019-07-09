##
# @file CMakeLists.txt
# @brief
# 中继服务器Cmake文件
#
# @author zsq
# @email 1575033031@qq.com
#

include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(Protobuf REQUIRED)
if(PROTOBUF_FOUND)
   message(STATUS "protobuf library found")
else()
    message(FATAL_ERROR "protobuf library is needed but cant be found\n")
endif()
include_directories(${PROTOBUF_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS Protbload.proto)

# 生成client客户端程序
set(SRC_CLIENT1 client1.cc)
ADD_EXECUTABLE(client1 ${SRC_CLIENT1} ${PROTO_SRCS} ${PROTO_HDRS})
add_dependencies(client1 ndsl)
target_link_libraries(client1 ndsl pthread protobuf ${PROTOBUF_LIBRARIES})

# 生成server服务端程序
set(SRC_SERVER1 server1.cc)
ADD_EXECUTABLE(server1 ${SRC_SERVER1} ${PROTO_SRCS} ${PROTO_HDRS})
add_dependencies(server1 ndsl)
target_link_libraries(server1 ndsl pthread protobuf ${PROTOBUF_LIBRARIES})

# 生成Relayserver测试 中继服务器
set(tcprelay tcprelay.cc)
add_executable(tcprelay ${tcprelay} ${PROTO_SRCS} ${PROTO_HDRS})
add_dependencies(tcprelay ndsl)
target_link_libraries(tcprelay ndsl pthread protobuf ${PROTOBUF_LIBRARIES})

# 生成Relayserver测试 中继服务器
set(nTcprelay nTcprelay.cc)
add_executable(nTcprelay ${nTcprelay} ${PROTO_SRCS} ${PROTO_HDRS})
add_dependencies(nTcprelay ndsl)
target_link_libraries(nTcprelay ndsl pthread protobuf ${PROTOBUF_LIBRARIES})

# 生成Relayserver测试 服务端
set(test_server server.cc)
add_executable(test_server ${test_server})
add_dependencies(test_server ndsl)
target_link_libraries(test_server ndsl)


# 生成Relayclient测试 客户端
set(test_client client.cc)
add_executable(test_client ${test_client} ${PROTO_SRCS} ${PROTO_HDRS})
add_dependencies(test_client ndsl)
target_link_libraries(test_client ndsl pthread protobuf ${PROTOBUF_LIBRARIES})




