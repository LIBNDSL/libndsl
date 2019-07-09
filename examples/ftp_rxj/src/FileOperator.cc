#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include "FileOperator.h"
#include "ndsl/utils/Error.h"
#include "ndsl/utils/Log.h"
#include "ndsl/config.h"
#include "ndsl/net/TcpChannel.h"

using namespace ndsl::ftp;
using namespace ndsl::net;
using namespace std;

FileOperator::FileOperator(){}
FileOperator::~FileOperator(){}

int FileOperator::deleteFile(string path)
{
    if (remove(path.c_str()) < 0)
    {
        cout<< "delete failed"<<endl;
        return S_FALSE;
    }
    return S_OK;
}

int FileOperator::createFile(string path)
{
    ofstream outfile(path.c_str(), ofstream::out|ofstream::app|ofstream::binary);
    if(!outfile)
    {
        cout <<"create file failed"<<endl;
        return S_FALSE;
    }
    outfile.close();
    return S_OK;
}

// error callback and related paramater
static bool isreaderror = false;
void storeFileError(void * pThis, int errnum)
{
    if(errnum != 11) isreaderror = true;
}

// the callback of onrev
void recvCb(void *param)
{
    struct FoParam *pparam = static_cast<struct FoParam *>(param);
    ofstream outfile(pparam -> path.c_str(), ofstream::out|ofstream::app|
            ofstream::binary);
    if(!outfile)
    {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "open file failed");
    }

    outfile.write(pparam -> buffer, *(pparam -> bufflen));

    memset(pparam -> buffer, 0, MAXLINE);
    *(pparam -> bufflen) = MAXLINE;
    outfile.close();

    pparam -> conn -> onRecv(pparam -> buffer, pparam -> bufflen, 0, recvCb, param);

    if (isreaderror || *(pparam -> bufflen) == 0) 
    {
        if(!isreaderror)
            LOG(LOG_DEBUG_LEVEL, LOG_SOURCE_TCPCLIENT, "recv data over");
        delete(pparam -> conn);
        delete(pparam -> buffer);
        delete(pparam -> bufflen);
        delete(pparam);
    }
}

int FileOperator::storeFile(const string& path, TcpConnection *conn)
{
    char *buffer = new char[MAXLINE];
    struct FoParam *param = new FoParam;
    param -> path = path;
    param -> buffer = buffer;
    param -> conn = conn;
    memset(buffer, 0, MAXLINE);
    ssize_t *len = new ssize_t(MAXLINE);
    param -> bufflen = len;

    // sign up callback
    conn -> onError(storeFileError, param -> bufflen);

    if (conn -> onRecv(buffer, param -> bufflen, 0, recvCb, (void *)param) != S_OK)
        return S_FALSE;

    if (isreaderror) return S_FALSE;
    return S_OK; 
}

static bool sendFileError = false; // need be set as static

void isOver(void *p)
{
    delete((char *)p);
}

void isSendError(void *p, int)
{
    sendFileError = true;
    LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "onsend error:%s", strerror(errno));
    delete((char *)p);
}

int FileOperator::getFile(const string &filename, TcpConnection *conn)
{
    ifstream infile(filename.c_str(), ifstream::in|ifstream::binary);
    if (!infile)
    {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "open file failed");
        return S_FALSE;
    }

    infile.seekg(0, std::ios::end);
    int flength = infile.tellg();
    infile.seekg(0, std::ios::beg);
    while(flength > 0)
    {
        char *buffer = new char[MAXLINE];
        infile.read(buffer, MAXLINE);
        if (!infile && !infile.eof())
        {
            LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "read buf failed");
            return S_FALSE;
        }

        flength -=(int)(infile.gcount());
        conn -> onError(isSendError, (void*)buffer);
        if (conn -> onSend(buffer, (int)(infile.gcount()), 0, isOver, 
                    (void *)buffer) != S_OK) return S_FALSE;
    }
    if (sendFileError) return S_FALSE;

    infile.close();
    delete(conn);   // may close before all data be send?
    return S_OK;
}

int FileOperator::getList(const string &directory, TcpConnection *conn)
{
    time_t rawtime;
    struct tm *time;
    struct stat statbuf;
    struct dirent *entry;
    char timebuf[80];
    DIR *dp = opendir(directory.c_str());
    if (!dp) return S_FALSE;

    while((entry = readdir(dp)) != NULL)
    {
        if (lstat((directory+entry -> d_name).c_str(), &statbuf) < 0)
        {
            LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "read file stat failed");
            continue;
        }

        rawtime = statbuf.st_mtime;
        time = localtime(&rawtime);
        strftime(timebuf, 80 ,"%b %d %H:%M", time);
        char buf[MAXLINE];
        memset(buf, 0, MAXLINE);
        snprintf(buf, MAXLINE, "%c %8d %s %s\r\n", (entry->d_type==DT_DIR)?'d':'-',
                (int)statbuf.st_size, timebuf, entry -> d_name);

        ssize_t len = strlen(buf);
        if (conn -> onSend(buf, len, 0, NULL, NULL) != S_OK) 
        {
            LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "send file stat failed");
            return S_FALSE;
        }
    }

    closedir(dp);
    delete(conn);
    return S_OK;
}







    
