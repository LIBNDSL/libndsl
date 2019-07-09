##
# @file CMakeLists.txt
# @brief
# tests目录下cmake文件
#
# @author niexw
# @email xiaowen.nie.cn@gmail.com
#
# find_package(Protobuf REQUIRED)
# include_directories(${PROTOBUF_INCLUDE_DIRS})
# INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_SOURCE_DIR}/build)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(/usr/include/libxml2)
# add_subdirectory(entitytest)
# add_subdirectory(v8test)
# add_subdirectory(rdma_rc_pingpong)
# add_subdirectory(ping-pong)

# catch要求打开异常
string(REGEX REPLACE "-fno-exceptions" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})


# 设置单元测试的文件
set(TEST 
    test.cc 

    #All files
    net/EpollTest.cc
    net/EventLoopTest.cc

    net/PipeAndFifoTest.cc
    net/SignalfdTest.cc
    net/TcpAcceptorTest.cc
    net/TcpConnectionTest.cc

    # net/RdmaTest.cc
    net/DtpTest.cc

    net/TimeWheelTest.cc
    net/UdpEndpointTest.cc
    net/UnixAndTcpSocketAddressTest.cc
    net/UnixConnectionTest.cc
    net/ELThreadpoolTest.cc
    net/SockoptTest.cc
    net/EventfdTest.cc

    utils/EndianTest.cc
    utils/ErrorTest.cc
    utils/GuidTest.cc
    utils/LogTest.cc
    utils/PluginTest.cc
    utils/SemaphoreTest.cc
    utils/TimeStampTest.cc
    utils/XmlTest.cc
)

# net和utils下的测试文件
add_executable(utest ${TEST})
add_dependencies(utest ndsl)
target_link_libraries(utest ndsl pthread)
