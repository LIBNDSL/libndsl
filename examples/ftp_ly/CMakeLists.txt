##
# @file CMakeLists.txt
# @brief
# ftpserver Cmake文件
#
# @author lanry
# @email luckylanry@163.com
#

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(include)

SET (FTP

    main.cc
    FileManage.cc
    FtpServer.cc
    ServerHandle.cc
)

add_executable(ly ${FTP})
add_dependencies(ly ndsl)
target_link_libraries(ly ndsl pthread)