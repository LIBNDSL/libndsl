#include <string>
#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <dirent.h>
#include <unistd.h>
#include "Connection.h"
#include "config_ftp.h"
#include "CommandHandler.h"
#include "FileOperator.h"
#include "ndsl/utils/Error.h"
#include "ndsl/utils/Error.h"


using namespace ndsl::ftp;
using namespace ndsl::net;
using namespace std;

CommandHandler::CommandHandler()
{
    userState_.currentWorkingDir = "/home/ranxiangjun/ftpdir/";
    userState_.mode = "PASV";
    userState_.username = "ran";
    userState_.password = "ran";
    userState_.isLogin = false;
    userState_.flag = 0;
}

CommandHandler::~CommandHandler(){}

void CommandHandler::noLeak(void *p)
{
    delete((char *)p);
}

int CommandHandler:: writeState(TcpConnection *contrconn, const string &content)
{
    ssize_t len = strlen(content.c_str());
    char *buf = new char[len+1];
    strcpy(buf, content.c_str());
    cout<<buf<<endl;
    contrconn -> onSend(buf, len, 0, noLeak, buf);
    return S_OK;
}

// get the paramater's hash value
typedef std::uint64_t hash_t;

constexpr hash_t prime = 0x100000001B3ull;
constexpr hash_t basis = 0xCBF29CE484222325ull;

hash_t hash_(char const* str)
{
    hash_t ret{basis};     
    while(*str){
        ret ^= *str;
        ret *= prime;
        str++;
    }    
    return ret;
}

constexpr hash_t hash_compile_time(char const* str, hash_t last_value = basis)
{
    return *str ? hash_compile_time(str+1, (*str ^ last_value) * prime) : last_value;
}

constexpr unsigned long long operator "" _hash(char const* p, size_t)
{
    return hash_compile_time(p);
}

int CommandHandler::parseCommand(const string &content, Param *connparam)
{
    cout<<content<<endl;
    LOG(LOG_DEBUG_LEVEL, LOG_SOURCE_TCPCLIENT,
            (getFirstParam(content) + getSecendParam(content)).c_str());

    // parse the paramater
    switch (hash_(getFirstParam(content).c_str()))
    {
        // parse the user
        case "USER"_hash:
            {
                if (getSecendParam(content).compare("anonymous") == 0)
                {
                    connparam -> cher -> userState_.flag = 1;
                    CommandHandler::writeState(connparam -> pconn,
                            (string)"331 need paasword for anonymous\r\n");
                    break;
                }

                if (getSecendParam(content).compare(connparam -> cher -> userState_.username) == 0)
                {
                    CommandHandler::writeState(connparam -> pconn,(string)"331 username is ok,need password\r\n");
                    break;
                }
                else
                {
                    writeState(connparam -> pconn, (string)"501 invalid username\r\n");
                    break;
                }
            }

        case "XPWD"_hash:
            {
                writeState(connparam -> pconn, (string)"257 " + "\"" 
                        + userState_.currentWorkingDir + "\"" + "\r\n");
                break;
            }

        case "PWD"_hash:
            {
                writeState(connparam -> pconn, (string)"257 " + "\"" 
                        + userState_.currentWorkingDir + "\"" + "\r\n");
                break;
            }

            // check the user's password
        case "PASS"_hash:
            {
                if (getSecendParam(content).compare(userState_.password) == 0)
                {
                    connparam -> cher -> userState_.flag = 0;
                    userState_.isLogin = true;
                    writeState(connparam -> pconn, (string)"230 login successfully\r\n");
                    break;
                }
                writeState(connparam -> pconn, (string)"501 incorrect password\r\n");
                break;
            }

            // change working directory
        case "CWD"_hash:
            {
                if (!userState_.isLogin)
                {
                    writeState(connparam -> pconn, (string)"220 need user account\r\n");
                    break;
                }

                DIR *pdir;
                if (strcmp(getSecendParam(content).substr(0, 1).c_str(), "/") == 0)
                {
                    pdir = opendir(getSecendParam(content).c_str());
                }else pdir = opendir(getFullPath(getSecendParam(content)).c_str());

                if (pdir == NULL)
                {
                    writeState(connparam -> pconn, (string)"501 invalid directory\r\n");
                    break;
                }

                if (strcmp(getSecendParam(content).substr(0, 1).c_str(), "/") == 0)
                    userState_.currentWorkingDir = getSecendParam(content);
                else 
                    userState_.currentWorkingDir = getFullPath(getSecendParam(content));

                writeState(connparam -> pconn, 
                        (string)"250 change directory successfully " + "\"" + 
                        connparam -> cher -> userState_.currentWorkingDir + 
                        "\"" + "is current directory\r\n");
                break;
            }

            // sever listens and change to passive mode
        case "PASV"_hash:
            {
                if (!userState_.isLogin)
                {
                    writeState(connparam -> pconn, (string)"220 need user account\r\n");
                    break;
                }

                userState_.mode = "PASV";
                SocketAddress4 server(IPADDRESS, PORT);
                TcpAcceptor *tAc = new TcpAcceptor(connparam -> loop);
                connparam -> datatac = tAc;
                int port = PORT;

                while(tAc -> start(server) != S_OK && errno == 98)
                {
                    port = port + 1;
                    if(port == 50000) port = 6666;
                    server.setPort(port);
                }

                // receive addr
                struct sockaddr_in rserveraddr;
                bzero(&rserveraddr, sizeof(rserveraddr));
                socklen_t addrlen = sizeof(rserveraddr);

                connparam -> pdataconn = new TcpConnection();
                if (tAc -> onAccept(connparam -> pdataconn, (SA *)&rserveraddr, &addrlen, 
                            NULL, NULL) != S_OK)
                    LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "accept dataconn fail");
                CommandHandler::writeState(connparam -> pconn, 
                        (string)"227 Entering Passive Mode (192,168,1,117,"+
                        to_string( port/256)+","+to_string( port%256)+")\r\n");
                break;
            }

            // tell server port and change to active mode 
        case "PORT"_hash:
            {
                if (!userState_.isLogin)
                {
                    writeState(connparam -> pconn, (string)"220 need user account\r\n");
                    break;
                }

                string subcontent = getSecendParam(content);
                size_t npos0 = subcontent.find(',');

                while(npos0 != string::npos)
                {
                    subcontent.replace(npos0, 1, ".");
                    npos0 = subcontent.find(',', npos0 + 1);
                }

                int npos1 = subcontent.find_last_of('.');
                int npos2 = subcontent.find_last_of('.', npos1-1);
                string ipstr = subcontent.substr(0,npos2);
                unsigned short int port = stoi(subcontent.substr(npos2 + 1,npos1 - npos2 
                            - 1))*256 + stoi(subcontent.substr(npos1 + 1));
                connparam -> paddr -> setAddress(ipstr.c_str(), port);

                userState_.mode = "PORT";
                writeState(connparam -> pconn, (string)"200 mode is PORT now\r\n");
                break;
            }

            // download file from server
        case "RETR"_hash:
            {
                if (!userState_.isLogin)
                {
                    writeState(connparam -> pconn, (string)"220 need user account\r\n");
                    break;
                }

                if (userState_.mode == "PORT")
                {
                    TcpConnection *conn = new TcpConnection();
                    Connection *ftpconn = new Connection;
                    if (ftpconn -> portConn(conn, connparam) != S_OK) break;
                    FileOperator getfile;

                    if (getfile.getFile(getFullPath(getSecendParam(content)), 
                                conn) != S_OK)
                    {
                        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "get file failed");
                        writeState(connparam -> pconn, (string)"451, get file fail\r\n");
                        break;
                    }

                    writeState(connparam -> pconn, (string)"226 get file ok\r\n");
                    break;
                }

                // for PASV mode
                if (userState_.mode == "PASV")
                {
                    writeState(connparam -> pconn, (string)"150 opening connection\r\n");
                    FileOperator getfile;

                    if ( getfile.getFile(getFullPath(getSecendParam(content)), 
                                connparam -> pdataconn) < 0)
                    {
                        writeState(connparam -> pconn, (string)"451 get file fail\r\n");
                        break;
                    }

                    delete(connparam -> datatac);
                    writeState(connparam -> pconn, (string)"250 get file ok\r\n");
                    break;
                }
            }

            // upload file to server
        case "STOR"_hash:
            {
                if (!userState_.isLogin)
                {
                    writeState(connparam -> pconn, (string)"220 need user account\r\n");
                    break;
                }

                if (userState_.mode == "PORT")
                {
                    TcpConnection *conn = new TcpConnection;
                    Connection *ftpconn = new Connection;

                    if (ftpconn -> portConn(conn, connparam) != S_OK) break;

                    FileOperator storefile;
                    if (storefile.storeFile(getFullPath(getSecendParam(content)), 
                                conn) != S_OK)
                    {
                        LOG(LOG_DEBUG_LEVEL, LOG_SOURCE_TCPCLIENT, "store file fail");
                        writeState(connparam -> pconn, (string)"451, get file fail\r\n");
                        break;
                    }

                    delete(ftpconn);
                    writeState(connparam -> pconn, (string)"250 get file ok\r\n");
                    break;
                }

                // for PASV mode
                if (userState_.mode == "PASV")
                {
                    writeState(connparam -> pconn, (string)"125 connection opened\r\n");

                    FileOperator storefile;
                    if ( storefile.storeFile(getFullPath(getSecendParam(content)), 
                                connparam -> pdataconn) < 0)
                    {
                        LOG(LOG_DEBUG_LEVEL, LOG_SOURCE_TCPCLIENT, "store file fail");
                        writeState(connparam -> pconn, (string)"451 store fail\r\n");
                        break;
                    }

                    delete(connparam -> datatac);
                    writeState(connparam -> pconn, (string)"250 store file ok\r\n");
                    break;
                }
            }

            // return the named list,or return working dir list
        case "NLST"_hash:
        case "LIST"_hash:
            {
                if (!userState_.isLogin)
                {
                    writeState(connparam -> pconn, (string)"220 need user account\r\n");
                    break;
                }

                if (userState_.mode == "PORT")
                {
                    TcpConnection *conn = new TcpConnection;
                    Connection *ftpconn = new Connection;

                    if (ftpconn -> portConn(conn, connparam) != S_OK) break;

                    // writeState(connparam -> pconn, (string)"150 ready transmission\r\n");
                    FileOperator getlist;
                    if (getlist.getList(getFullPath(getSecendParam(content)), 
                                conn) != S_OK)
                    {
                        LOG(LOG_DEBUG_LEVEL, LOG_SOURCE_TCPCLIENT, "getlist file fail");
                        writeState(connparam -> pconn, (string)"451, getlist fail\r\n");
                        break;
                    }

                    delete(ftpconn);
                    writeState(connparam -> pconn, (string)"226 get list ok\r\n");
                    break;
                }

                // for PASV mode
                if (userState_.mode == "PASV")
                {
                    writeState(connparam -> pconn, (string)"150 ready transmission\r\n");
                    FileOperator getlist;
                    if (getlist.getList(getFullPath(getSecendParam(content)), 
                                connparam -> pdataconn) != S_OK)
                    {
                        LOG(LOG_DEBUG_LEVEL, LOG_SOURCE_TCPCLIENT, "getlist file fail");
                        writeState(connparam -> pconn, (string)"451 getlist failed\r\n");
                        break;
                    }

                    delete(connparam -> datatac);
                    writeState(connparam -> pconn, (string)"250 get list ok\r\n");
                    break;
                }
            }

            // the type of server sysytem
        case "SYST"_hash:
            {
                if (!userState_.isLogin)
                {
                    writeState(connparam -> pconn, (string)"220 need user account\r\n");
                    break;
                }

                writeState(connparam -> pconn, (string)"215 unix system\r\n");
                break;
            }

            // ascii or binary
        case "TYPE"_hash:
            {
                if (!userState_.isLogin)
                {
                    writeState(connparam -> pconn, (string)"220 need user account\r\n");
                    break;
                }

                writeState(connparam -> pconn, (string)"200 binary mode ok\r\n");
                break;
            }

        case "SIZE"_hash:
            {
                if (!userState_.isLogin)
                {
                    writeState(connparam -> pconn, (string)"220 need user account\r\n");
                    break;
                }

                writeState(connparam -> pconn, (string)"550 file not found\r\n");
                break;
            }

            // disconnect to server
        case "QUIT"_hash:
            {
                userState_.isLogin = false;
                writeState(connparam -> pconn, (string)"221 be unlogin now\r\n");
                if(connparam -> cher ->userState_.flag !=1 )
                {
                    delete(connparam -> cher);
                delete(connparam -> pconn);
                delete(connparam -> pdataconn);
                delete(connparam -> buffer);
                delete(connparam -> buflen);
                delete(connparam -> paddr);
                delete(connparam);
                break;
            }
            else break;
        }

        default:
        {
            writeState(connparam -> pconn, (string)"502 command not inplemented\n");
            return S_FALSE;
            break;
        }
    }
    return S_OK;
}

string CommandHandler::getCurrentWorkingDir()
{
    return userState_.currentWorkingDir;
}

string CommandHandler::getMode()
{
    return userState_.mode;
}

// get the first part of the command
string CommandHandler::getFirstParam(const string &content)
{
    if (strcmp(content.substr(3,1).c_str(), "\r") == 0)
        return content.substr(0, 3);
    if (strcmp(content.substr(4,1).c_str(), "\r") == 0)
        return content.substr(0, 4);
    else return content.substr(0, content.find(" "));
}

// get the secend part of the command
string CommandHandler::getSecendParam(const string &content)
{
    if (strcmp(content.substr(3, 1).c_str(), "\r") == 0 || 
            strcmp(content.substr(4, 1).c_str(), "\r") == 0)
        return "";

    else return content.substr(content.find(" ") + 1, 
            content.find("\r") - content.find(" ") - 1);
}
      
// get the full path
string CommandHandler::getFullPath(const string &filename)
{
    return getCurrentWorkingDir().append(filename);
}






