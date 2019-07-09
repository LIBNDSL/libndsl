#include <iostream>
#include <string.h>
#include <stdio.h>
#include "FtpReply.h"

using namespace std;

FtpReply::FtpReply (){}
FtpReply:: ~FtpReply (){}

void FtpReply::reply(FtpConnection* conn, int num, const char* text){
	char buf[1024] = {0};
	sprintf(buf, "%d %s.\r\n", num, text);
	conn->onSend(buf, strlen(buf), 0, NULL, NULL);
}
void FtpReply::lreply(FtpConnection* conn, int num, const char* text){
	char buf[1024] = {0};
	sprintf(buf, "%d-%s.\r\n", num, text);
	conn->onSend(buf, strlen(buf), 0, NULL, NULL);
}
void FtpReply::sreply(FtpConnection* conn, const char* text){
	char buf[1024] = {0};
	sprintf(buf, " %s.\r\n", text);
	conn->onSend(buf, strlen(buf), 0, NULL, NULL);
}
void FtpReply::replyList(FtpConnection* conn, const char* text){
	char buf[1024] = {0};
	sprintf(buf, "%s", text);
	conn->onSend(buf, strlen(buf), 0, NULL, NULL);
}
