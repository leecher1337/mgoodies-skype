//#define DEBUG_RELEASE 1

#ifdef DEBUG_RELEASE
  #define _DEBUG 1
#endif

#ifdef _DEBUG
//	#include <crtdbg.h>
	void log_write(char *prfx, char *text);
	void log_long(char *prfx, long text);
	void init_debug(void);
	#define DEBUG_OUT(a) OUTPUT(a)
	#define LOG(a, b) log_write(a, b)
	#define LOGL(a, b) log_long(a, b)
#else
	#define DEBUG_OUT(a) 
	#define LOG(a,b )
    #define LOGL(a, b)
#endif
