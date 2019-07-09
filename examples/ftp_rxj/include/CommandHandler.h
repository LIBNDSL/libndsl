////
// @file CommandHandler.h
// @brief wrapper command interpretion
//
// @auther ranxiangjun
// @email ranxianshen@gmail.com
//
#ifndef _NDSL_EXAMPLE_COMMANDHANDLER_H
#define _NDSL_EXAMPLE_COMMANDHANDLER_H

#include "../../../include/ndsl/net/TcpConnection.h"
#include "../../../include/ndsl/net/EventLoop.h"
#include "Connection.h"

namespace ndsl{
namespace ftp{

struct state
{   
    std::string currentWorkingDir;
    std::string mode;
    std::string username;
    std::string password;
    bool isLogin;
    int flag;
};

class CommandHandler
{
  public:
    CommandHandler();
    ~CommandHandler();
    static int writeState(TcpConnection *, const std::string&);
    static void noLeak(void *);
    int parseCommand(const std::string&, Param *);
    std::string getCurrentWorkingDir();
    std::string getMode();
    std::string getFirstParam(const std::string &);
    std::string getSecendParam(const std::string &);
    std::string getFullPath(const std::string &);

  private:
    state userState_;
    TcpConnection *contrconn_;

};
}   // ftp
}   // ndsl

#endif  // _NDSL_EXAMPLE_COMMANDHANDLER_H

