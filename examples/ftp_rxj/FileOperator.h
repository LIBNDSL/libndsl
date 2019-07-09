//
// @file FileOperator.h
// @brief wrapper file operations
//
// @auther ranxangjun
// @email ranxianshen@gamil.com
//
#ifndef _NDSL_EXAMPLE_FILEOPERATOR_H
#define _NDSL_EXAMPLE_FILEOPERATOR_H

namespace ndsl{
namespace ftp{

class FileOperator
{
  public:
    FileOperator();
    ~FileOperator();
    int deleteFile(std::string);
    int createFile(std::string);    
    int storeFile(std::string, char*);     // use append mode
    int getFile(std::string);
    int reNameFile(std::string);
    int deleteDir(std::string);
    int createDir(std::string);
    int getFileAndDirList(std::string);
    std::string getFullPath(std::string &path);
};

}   // ftp
}   // ndsl
#endif  //NDSL_EXAMPLE_FILEOPERATOR_H
