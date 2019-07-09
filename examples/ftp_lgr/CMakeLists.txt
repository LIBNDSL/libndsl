##
# @file CMakeLists.txt
# @brief
# examples目录的cmake文件
#
# @author niexw
# @email xiaowen.nie.cn@gmail.com
#
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(include)

SET (FTP_LGR

    main.cc
    
    src/FTPServer.cc
    src/Session.cc

)

add_executable(ftp_lgr ${FTP_LGR})
add_dependencies(ftp_lgr ndsl)
target_link_libraries(ftp_lgr ndsl pthread crypt)