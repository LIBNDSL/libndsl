##
# @file CMakeLists.txt
# @brief
# examples目录的cmake文件
#
# @author niexw
# @email xiaowen.nie.cn@gmail.com
#
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/examples/ftp_why/include)
message(STATUS "${CMAKE_SOURCE_DIR}/examples/ftp_why/include")
SET (FTP_WHY

    main.cc
    FtpConnection.cc
    FtpServer.cc
    Session.cc
    FtpProto.cc
    StringUtil.cc
    FtpReply.cc
    SysUtil.cc

)

add_executable(aaa ${FTP_WHY})
add_dependencies(aaa ndsl)
target_link_libraries(aaa ndsl pthread)
