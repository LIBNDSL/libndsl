##
# @file CMakeLists.txt
# @brief
# ndsl目录的cmake文件
#
# @author niexw
# @email xiaowen.nie.cn@gmail.com
#

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/examples/ftp_rxj/include)

set(LIB_NDSL_FTP_SRC
  #all files
  CommandHandler.cc
  Connection.cc
  FileOperator.cc
  FtpSever.cc
 )

#add_library(ndslftp ${LIB_NDSL_FTP_SRC})
add_executable(server ${LIB_NDSL_FTP_SRC})
add_dependencies(server ndsl)
target_link_libraries(server ndsl pthread)

#生成libplugin.so
 #set(LIB_SRC utils/Plugin.cc)
 #set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)  

 #add_library(libplugin SHARED ${LIB_SRC})
 #set_target_properties(libplugin PROPERTIES OUTPUT_NAME "plugin")

 #set(CLIENT_SRC utils/Clientplugin.cc)
 #set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)  

 #add_library(clientplugin SHARED ${CLIENT_SRC})
 #set_target_properties(clientplugin PROPERTIES OUTPUT_NAME "clientplugin")
