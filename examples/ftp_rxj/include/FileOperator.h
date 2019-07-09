//
// @file FileOperator.h
// @brief wrapper file operations
//
// @auther ranxangjun
// @email ranxianshen@gamil.com
//
#ifndef _NDSL_EXAMPLE_FILEOPERATOR_H
#define _NDSL_EXAMPLE_FILEOPERATOR_H

#include "../../../include/ndsl/net/TcpConnection.h"

using namespace ndsl::net;

namespace ndsl{
namespace ftp{

struct FoParam
{
    std::string path;
    char *buffer;
    ssize_t *bufflen;
    TcpConnection *conn;
};

class FileOperator
{
  public:
    FileOperator();
    ~FileOperator();
    int deleteFile(std::string);
    int createFile(std::string);    
    int storeFile(const std::string &, TcpConnection *conn);     // use append mode
    int getFile(const std::string &, TcpConnection *conn);
    int getList(const std::string &, TcpConnection *conn);
    int reNameFile(std::string);
    int deleteDir(std::string);
    int createDir(std::string);
    int getFileAndDirList(std::string);
    std::string getFullPath(std::string &path);
};

}   // ftp
}   // ndsl
#endif  //NDSL_EXAMPLE_FILEOPERATOR_H
