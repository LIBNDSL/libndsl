#ifndef __STRINGUTIL_H__
#define __STRINGUTIL_H__

class StringUtil {
    private:
	  /* data */
	  
    public:
	  StringUtil ();
	  virtual ~StringUtil ();
	 
	public:
	  static void trimCtlf(char* str);
	  static int split(const char* str, char* left, char* right, char c);
	  static int isAllSpace(const char* str);
	  
};

#endif // __STRINGUTIL_H__

