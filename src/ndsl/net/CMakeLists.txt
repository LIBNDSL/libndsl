# @file CMakeLists.txt
# @brief
# src/com目录的cmake文件
#
# @author niexw
# @email xiaowen.nie.cn@gmail.com
#
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/ndsl)
include_directories(${PROJECT_SOURCE_DIR}/include/ndsl)
include_directories(${CMAKE_SOURCE_DIR}/include/ndsl/net)
include_directories(${CMAKE_SOURCE_DIR}/include/ndsl/utils)

# set(LIB_NDSL_SRC Log.cc Reactor.cc Timer.cc Dns.cc )
# add_library(ndsl SHARED ${LIB_NDSL_SRC})
# target_link_libraries(ndsl pthread)
