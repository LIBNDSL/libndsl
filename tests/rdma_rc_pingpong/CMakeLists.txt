##
# @file CMakeLists.txt
# @brief 
# Rmda_rc_pingpong的cmake文件
# 
# @author Liu GuangRui
# @email 675040625@qq.com
#

add_executable(rdma_rc_pingpong_server Server.cc)
add_dependencies(rdma_rc_pingpong_server ndsl)
target_link_libraries(rdma_rc_pingpong_server ndsl pthread)

add_executable(rdma_rc_pingpong_client Client.cc)
add_dependencies(rdma_rc_pingpong_client ndsl)
target_link_libraries(rdma_rc_pingpong_client ndsl pthread)
