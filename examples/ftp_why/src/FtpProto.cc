#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include "FtpProto.h"
#include "FtpReply.h"
#include "StringUtil.h"
#include "FtpConnection.h"
#include "FtpServer.h"
#include "ndsl/net/SocketAddress.h"
#include "SysUtil.h"

FtpProto::FtpProto (){}
FtpProto::~FtpProto (){}

void FtpProto::handle(Session* sess){
	FtpReply::reply(sess->ctrlConn, 220, "miniftp0.1");
	memset(sess->cmdline, 0, sizeof(sess->cmdline));
	ssize_t len = strlen(sess->cmdline);
	sess->ctrlConn->onRecv(sess->cmdline, &len, 0, revcCallback, sess);
	
}

void FtpProto::revcCallback(void* p){
	Session *sess = static_cast<Session *>(p);
	memset(sess->cmd, 0, sizeof(sess->cmd));
	memset(sess->arg, 0, sizeof(sess->arg));
	StringUtil::trimCtlf(sess->cmdline);
	StringUtil::split(sess->cmdline, sess->cmd, sess->arg, ' ');
	cout << "cmd = " << sess->cmd << endl;
	cout << "arg = " << sess->arg << endl;
	if(sess->cmd != NULL){
		if(strcmp("USER", sess->cmd) == 0){
			doUser(sess);
		}else if(strcmp("PASS", sess->cmd) == 0){
			doPass(sess);
		}else if(strcmp("SYST", sess->cmd) == 0){
			doSyst(sess);
		}else if(strcmp("FEAT", sess->cmd) == 0){
			doFeat(sess);
		}else if(strcmp("PWD", sess->cmd) == 0){
			doPwd(sess);
		}else if(strcmp("TYPE", sess->cmd) == 0){
			doType(sess);
		}else if(strcmp("PASV", sess->cmd) == 0){
			doPasv(sess);
		}else if(strcmp("LIST", sess->cmd) == 0){
			doList(sess);
		}else if(strcmp("CWD", sess->cmd) == 0){
			doCwd(sess);
		}else if(strcmp("RETR", sess->cmd) == 0){
			doRetr(sess);
		}else if(strcmp("STOR", sess->cmd) == 0){
			doStor(sess);
		}else{
			FtpReply::reply(sess->ctrlConn, 500, "Unknown command");
		}
	}
	memset(sess->cmdline, 0, sizeof(sess->cmdline));
	ssize_t len = strlen(sess->cmdline); 
	sess->ctrlConn->onRecv(sess->cmdline, &len, 0, revcCallback, sess);
}

void FtpProto::doUser(Session* sess){
	FtpReply::reply(sess->ctrlConn, 331, "please specify the password");
}
void FtpProto::doPass(Session* sess){
	FtpReply::reply(sess->ctrlConn, 230, "Login successful");
}

void FtpProto::doSyst(Session* sess){
	FtpReply::reply(sess->ctrlConn, 215, "UNIX Type:L8");
}
void FtpProto::doFeat(Session* sess){
	FtpReply::lreply(sess->ctrlConn, 211, "Features");
	FtpReply::sreply(sess->ctrlConn, "EPRT");
	FtpReply::sreply(sess->ctrlConn, "EPSV");
	FtpReply::sreply(sess->ctrlConn, "MDTM");
	FtpReply::sreply(sess->ctrlConn, "PASV");
	FtpReply::sreply(sess->ctrlConn, "REST STREAM");
	FtpReply::sreply(sess->ctrlConn, "SIZE");
	FtpReply::sreply(sess->ctrlConn, "TVFS");
	FtpReply::reply(sess->ctrlConn, 211, "End");
}
void FtpProto::doPwd(Session* sess){
	char text[1024] = {0};
	char dir[1024+1] = {0};
	getcwd(dir, 1024);
	sprintf(text, "\"%s\"", dir);
	FtpReply::reply(sess->ctrlConn, 257, text);
}
void FtpProto::doType(Session* sess){
	if(strcmp(sess->arg, "A") == 0){
		sess->isASCII = 1;
		FtpReply::reply(sess->ctrlConn, 200, "Switching to ASCII mode");
	}else if(strcmp(sess->arg, "I") == 0){
		sess->isASCII = 0;
		FtpReply::reply(sess->ctrlConn, 200, "Switching to Binary mode");
	}else{
		FtpReply::reply(sess->ctrlConn, 500, "Unrecognised TYPE command");
	}
}

//void FtpProto::doPort(Session* sess){
//	
//}

void FtpProto::doPasv(Session* sess){
	char ip[16] = "192.168.153.130";
	unsigned short port = 5189;
    
    FtpConnection *con = new FtpConnection();
	con->sess = sess;
    sess->dataTac->onAccept(con, NULL, NULL, FtpServer::onDataConnection, con);
    
    unsigned int v[4];
	sscanf(ip, "%u.%u.%u.%u", &v[0], &v[1], &v[2], &v[3]);
    char text[1024] = {0};
	sprintf(text, "Entering Passive Mode (%u,%u,%u,%u,%u,%u)", v[0], v[1], v[2], v[3], port>>8, port&0xFF);
	sess->isPasv = 1;
    FtpReply::reply(sess->ctrlConn, 227, text);
}

void FtpProto::doList(Session* sess){
	FtpReply::reply(sess->ctrlConn, 150, "Here comes the directory listing");
	FtpConnection::dataConnection(sess);
	listCommon(sess);
	// close
	sess->dataConn->pTcpChannel_->erase();
	close(sess->dataConn->pTcpChannel_->getFd());
	sess->dataConn = NULL;
	FtpReply::reply(sess->ctrlConn, 226, "Directory send OK");
}
int FtpProto::listCommon(Session* sess){
	DIR* dir = opendir(".");
	if(dir == NULL){
		return 0;
	} 
	struct dirent* dt;
	struct stat sbuf;
	while((dt = readdir(dir)) != NULL){
		if(lstat(dt->d_name, &sbuf) < 0){
			continue;
		}
		if(dt->d_name[0] == '.'){
			continue;
		}	
		char perms[] = "----------";
		perms[0] = '?';
		mode_t mode = sbuf.st_mode;
		switch(mode & S_IFMT){
			case S_IFREG:
				perms[0] = '-';
				break;
			case S_IFDIR:
				perms[0] = 'd';
				break;
			case S_IFLNK:
				perms[0] = 'l';
				break;
			case S_IFIFO:
				perms[0] = 'p';
				break;
			case S_IFSOCK:
				perms[0] = 's';
				break;
			case S_IFCHR:
				perms[0] = 'c';
				break;
			case S_IFBLK:
				perms[0] = 'b';
				break;
		}
		
		if(mode & S_IRUSR){
			perms[1] = 'r';
		}
		if(mode & S_IWUSR){
			perms[2] = 'w';
		}
		if(mode & S_IXUSR){
			perms[3] = 'x';
		}
		if(mode & S_IRGRP){
			perms[4] = 'r';
		}
		if(mode & S_IWGRP){
			perms[5] = 'w';
		}
		if(mode & S_IXGRP){
			perms[6] = 'x';
		}
		if(mode & S_IROTH){
			perms[7] = 'r';
		}
		if(mode & S_IWOTH){
			perms[8] = 'w';
		}
		if(mode & S_IXOTH){
			perms[9] = 'x';
		}
		if(mode & S_ISUID){
			perms[3] = (perms[3] == 'x') ? 's' : 'S';
		}
		if(mode & S_ISGID){
			perms[6] = (perms[6] == 'x') ? 's' : 'S';
		}
		if(mode & S_ISVTX){
			perms[9] = (perms[9] == 'x') ? 's' : 'S';
		}
		
		char buf[1024] = {0}; 
		int off = 0;
		off += sprintf(buf, "%s", perms);
		off += sprintf(buf+off, " %3d %-8d %-8d ", (unsigned int)sbuf.st_nlink, sbuf.st_uid, sbuf.st_gid);
		off += sprintf(buf+off, "%8lu ", (unsigned long)sbuf.st_size);
		
		const char* pDateFormat = "%b %e %H:%H";
		struct timeval tv;
		gettimeofday(&tv, NULL);
		time_t localTime = tv.tv_sec;
		if(sbuf.st_mtime > localTime  || (localTime - sbuf.st_mtime) > 60*60*24*182){
			pDateFormat = "%b %e  %y";
		}
		char dateBuf[64] = {0};
		struct tm* pTm = localtime(&localTime);
		strftime(dateBuf, sizeof(dateBuf), pDateFormat, pTm);
		off += sprintf(buf+off, "%s ", dateBuf);
		if(S_ISLNK(sbuf.st_mode)){
			char temp[1024] = {0};
			readlink(dt->d_name, temp, sizeof(temp));
			sprintf(buf+off, "%s -> %s\r\n", dt->d_name, temp);
		}else{
			sprintf(buf+off, "%s\r\n", dt->d_name);
		}
		
//		cout << buf;
		if(sess->dataConn != NULL){
			FtpReply::replyList(sess->dataConn, buf);
		}else{
			cout << "dataConn is NULL!" << endl;
		}
	}
	closedir(dir);
	return 1;
}

void FtpProto::doCwd(Session* sess){
	if(chdir(sess->arg) < 0){
		FtpReply::reply(sess->ctrlConn, 550, "Failed to change director");
		return;
	}
	FtpReply::reply(sess->ctrlConn, 250, "Directory successfully changed");
}

void FtpProto::doRetr(Session* sess){
	FtpConnection::dataConnection(sess);
	// open file
	int fd = open(sess->arg, O_RDONLY);
	if(fd == -1){
		FtpReply::reply(sess->ctrlConn, 550, "Could not create file");
		return;
	}
	// add read lock
	if(SysUtil::lockFileRead(fd) == -1){
		FtpReply::reply(sess->ctrlConn, 550, "Open file failed");
		return;
	}
	struct stat sbuf;
	int ret = fstat(fd, &sbuf);
	if(!S_ISREG(sbuf.st_mode)){
		FtpReply::reply(sess->ctrlConn, 550, "Open file failed");
		return;
	}
	char text[1024] = {0};
	if(sess->isASCII){
		sprintf(text, "Opening ASCLL mode data connection for %s (%lld bytes)", sess->arg, (long long)sbuf.st_size);
	}else{
		sprintf(text, "Opening BINARY mode data connection for %s (%lld bytes)", sess->arg, (long long)sbuf.st_size);
	}
	FtpReply::reply(sess->ctrlConn, 150, text);
	// download
	int flag;
	char buf[4096];
	while(1){
		ret = read(fd, buf, sizeof(buf));
		if(ret == -1){
			if(errno == EINTR){
				continue;
			}else{
				flag = -1;
				break;
			}
		}else if(ret == 0){
			flag = 0;
			break;
		}
		if(write(sess->dataConn->pTcpChannel_->getFd(), buf, ret) != ret){
			flag = 2;
			break;
		}
	}
	// close 
	sess->dataConn->pTcpChannel_->erase();
	close(sess->dataConn->pTcpChannel_->getFd());
	sess->dataConn = NULL;
	close(fd);
	if(flag == 0){
		// 226
		FtpReply::reply(sess->ctrlConn, 226, "Directory send OK");
	}else{
		FtpReply::reply(sess->ctrlConn, 426, "Directory send error");
	}
}

void FtpProto::doStor(Session* sess){
	FtpConnection::dataConnection(sess);
	// open file
	int fd = open(sess->arg, O_CREAT | O_WRONLY, 0666);
	if(fd == -1){
		FtpReply::reply(sess->ctrlConn, 550, "Open file failed");
		return;
	}
	// add write lock
	if(SysUtil::lockFileWrite(fd) == -1){
		FtpReply::reply(sess->ctrlConn, 550, "Lcok file failed");
		return;
	}
	
	sess->localFd = fd;
	FtpReply::reply(sess->ctrlConn, 150, "Ok to send data");
	
	char* buf = new char[16384];
	sess->recvBuf = buf;
	int ret;
	int flag = 0;
	while(1){
		ret = read(sess->dataConn->pTcpChannel_->getFd(), sess->recvBuf, sizeof(sess->recvBuf));
		if(ret == -1){
			if(errno == EAGAIN){
				continue;
			}else{
				flag = -1;
				break;
			}
		}else if(ret == 0){
			break;
		}else{
			if(write(fd, sess->recvBuf, ret) < 0){
				flag = -2;
				break;			
			}
		}
		
	}
	
	sess->dataConn->pTcpChannel_->erase();
	close(sess->dataConn->pTcpChannel_->getFd());
	close(sess->localFd);
	free(sess->recvBuf);
	sess->dataConn = NULL;
	sess->recvBuf = NULL;
	sess->len = -1;
	sess->localFd = -1;
	
	if(flag == 0){
		// 226
		FtpReply::reply(sess->ctrlConn, 226, "Directory send OK");
	}else if(flag == -2){
		FtpReply::reply(sess->ctrlConn, 426, "Failed writting to local file");
	}else if(flag == -1){
		FtpReply::reply(sess->ctrlConn, 451, "Failed reading from network stream");
	}
}





