##
# @file CMakeLists.txt
# @brief
# ndsl目录的cmake文件
#
# @author niexw
# @email xiaowen.nie.cn@gmail.com
#
find_package(Protobuf REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(/usr/include/libxml2)
include_directories(${CMAKE_SOURCE_DIR}/build)
include_directories(${PROTOBUF_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

set(LIB_NDSL_SRC
  #all files
  net/BaseChannel.cc
  net/EventLoop.cc
  net/SignalHandler.cc
  net/TcpAcceptor.cc

  net/TcpConnection.cc
  net/UdpClient.cc
  net/UnixChannel.cc
  net/Multiplexer.cc
  net/SocketAddress.cc

  net/TcpChannel.cc
  net/TimeWheel.cc
  net/UdpEndpoint.cc
  net/UnixConnection.cc
  net/Epoll.cc

  net/PipeAndFifo.cc
  net/SignalChannel.cc
  net/SocketAddressUn.cc
  net/TcpClient.cc
  net/UdpChannel.cc
  net/UnixAcceptor.cc
  net/ELThreadpool.cc
  net/WQThreadpool.cc
  net/SocketOpt.cc
  net/EventfdConnection.cc
  net/EventfdChannel.cc
  net/Dns.cc

  net/RdmaAcceptor.cc
  net/RdmaConnection.cc
  net/RdmaClient.cc
  # net/Sendfile.cc  
  net/DTP.cc
  net/DTPClient.cc
  net/Entity.cc

  utils/Endian.cc
  utils/Plugin.cc
  utils/Thread.cc
  utils/TimeStamp.cc
  utils/Error.cc
  utils/Guid.cc
  utils/Log.cc
  utils/Semaphore.cc
  utils/Xml.cc
 )

#生成protobuf
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ./net/ControlMsg.proto)

add_library(ndsl ${LIB_NDSL_SRC} ${PROTO_SRCS} ${PROTO_HDRS})
# sudo apt-get install uuid-dev libxml2-dev libv8-3.14-dev libssl-dev librdmacm-dev libibverbs-dev
target_link_libraries(ndsl uuid xml2 crypto protobuf ${PROTOBUF_LIBRARIES} rdmacm ibverbs)

#生成libplugin.so
 #set(LIB_SRC utils/Plugin.cc)
 #set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)  

 #add_library(libplugin SHARED ${LIB_SRC})
 #set_target_properties(libplugin PROPERTIES OUTPUT_NAME "plugin")

 #set(CLIENT_SRC utils/Clientplugin.cc)
 #set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)  

 #add_library(clientplugin SHARED ${CLIENT_SRC})
 #set_target_properties(clientplugin PROPERTIES OUTPUT_NAME "clientplugin")


