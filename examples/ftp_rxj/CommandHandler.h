////
// @file CommandHandler.h
// @brief wrapper command interpretion
//
// @auther ranxiangjun
// @email ranxianshen@gmail.com
//
#ifndef _NDSL_EXAMPLE_COMMANDHANDLER_H
#define _NDSL_EXAMPLE_COMMANDHANDLER_H

namespace ndsl{
namespace ftp{

struct state
{   
    std::string currentWorkingDir;
    std::string mode;
    std::string username;
    std::string password;
    bool isLogin;
};

class CommandHandler
{
  public:
    CommandHandler();
    ~CommandHandler();
    int writeState(int fd, std::string&);
    int parseCommand(std::string&);
    std::string getCurrentWorkingDir();
    std::string getMode();
    std::string getFirstParam(std::string &);
    std::string getSecendParam(std::string &);

  privatel:
    state userState;
    int commFd;

};
}   // ftp
}   // ndsl

#endif  // _NDSL_EXAMPLE_COMMANDHANDLER_H

