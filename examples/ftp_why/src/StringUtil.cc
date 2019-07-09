#include <string.h>
#include <ctype.h>
#include "StringUtil.h"

StringUtil::StringUtil(){}
StringUtil::~StringUtil(){}

void StringUtil::trimCtlf(char* str){
	char* p = &str[strlen(str) - 1];
	while(*p == '\r' || *p == '\n'){
		*p-- = '\0';
	}
		
}

int StringUtil::split(const char* str, char* left, char* right, char c){
	char* p = strchr((char*)str, c);
	if(p == NULL){
		strcpy(left, str);
		return 1;
	}else{
		strncpy(left, str, p-str);
		strcpy(right, p+1);
		return 0;
	}
}

int StringUtil::isAllSpace(const char* str){
	while(*str){
		if(!isspace(*str)){
			return 0;
		}
		str++;
	}
	return 1;
}
