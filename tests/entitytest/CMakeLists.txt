# 
# @file 
# @brief 
# 使用cmakelists自动生成pb.cc pb.h文件
#
# @author zzt
# @emial 429834658@qq.com
#

find_package(Protobuf REQUIRED)
if(PROTOBUF_FOUND)
   message(STATUS "protobuf library found")
else()
    message(FATAL_ERROR "protobuf library is needed but cant be found\n")
endif()
include_directories(${PROTOBUF_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS Protbload.proto)

message(STATUS ${PROTOBUF_LIBRARIES})
set(SRC_entity EntityTest.cc)
ADD_EXECUTABLE(entity ${SRC_entity} ${PROTO_SRCS} ${PROTO_HDRS})
add_dependencies(entity ndsl)
target_link_libraries(entity ndsl pthread protobuf ${PROTOBUF_LIBRARIES})