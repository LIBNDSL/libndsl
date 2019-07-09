##
# @File: CMakeLists.txt
# @Brief 
# NDSL的根CMakeLists.txt
# @Author Liu GuangRui
# @Email 675040625@qq.com
#

cmake_minimum_required(VERSION 2.8)

project(NDSL)

message(STATUS "### Begin to configure project NDSL ###")

# 加入一个配置头文件，用于处理 CMake 对源码的设置
configure_file (
"${PROJECT_SOURCE_DIR}/config.h.in"
"${PROJECT_SOURCE_DIR}/include/ndsl/config.h"
)

# 检测cmake运行操作系统/CPU/编译器
message(STATUS "Generator: ${CMAKE_GENERATOR}")
message(STATUS "Building OS: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Building Processor: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER_ID}")

# 编译模式，Release/Debug，-DCMAKE_BUILD_TYPE=Release|Debug
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug) # Debug Release
endif()
string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)
if (CMAKE_BUILD_TYPE MATCHES "debug")
    set(CMAKE_BUILD_TYPE Debug)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "Building mode: ${CMAKE_BUILD_TYPE}")

# c/c++按照11表展
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED OFF)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置编译结果输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/bin")

# 设置编译参数
set(CMAKE_C_FLAGS                     "-Wall -Werror -Wno-attributes")
set(CMAKE_C_FLAGS_DEBUG               "-g -DDEBUG") #-fstack-protector-all
set(CMAKE_C_FLAGS_RELEASE             "-O3")
set(CMAKE_C_FLAGS_MINSIZEREL          "-Os")
set(CMAKE_C_RELWITHDEBINFO_RELEASE    "-O2")
set(CMAKE_CXX_FLAGS                   "-Wall -Werror -Wno-attributes -ftemplate-backtrace-limit=0 -fno-exceptions -std=\"c++11\"")
set(CMAKE_CXX_FLAGS_DEBUG             "-g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE           "-O3 -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS_MINSIZEREL        "-Os")
set(CMAKE_CXX_RELWITHDEBINFO_RELEASE  "-O2")

add_subdirectory(src)
add_subdirectory(tests)
# add_subdirectory(examples)

message(STATUS "### Done ###")
