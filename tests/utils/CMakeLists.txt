##
# @file CMakeLists.txt
# @brief
# tests目录下cmake文件
#
# @author niexw
# @email xiaowen.nie.cn@gmail.com
#
include_directories(${PROJECT_SOURCE_DIR}/include/ndsl/utils)
include_directories(${PROJECT_SOURCE_DIR}/include/ndsl/net)
link_directories(${PROJECT_SOURCE_DIR}/src/lib)

# catch要求打开异常
string(REGEX REPLACE "-fno-exceptions" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

# 设置单元测试的文件
set(TEST ${TEST} GuidTest.cc)

add_executable(utest ${TEST})

target_link_libraries(utest ndsl)
target_link_libraries(ndsl libuuid.so)

