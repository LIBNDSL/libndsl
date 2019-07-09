##
 # @file CMakeList.txt
 # @brief 
 # 
 # @author gyz
 # @email mni_gyz@163.com
 ##

# 设置ping-pong单元测试的文件
set(pingserver server.cc)
set(pingclient nClient.cc)
# include_directories(${CMAKE_SOURCE_DIR}/tests)

# 生成pingpong测试 服务端
add_executable(pserver ${pingserver})
add_dependencies(pserver ndsl)
target_link_libraries(pserver ndsl pthread)

# 生成pingpong测试 客户端
add_executable(pclient ${pingclient})
add_dependencies(pclient ndsl)
target_link_libraries(pclient ndsl pthread)