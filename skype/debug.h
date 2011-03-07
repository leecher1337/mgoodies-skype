//#define DEBUG_RELEASE 1

#ifdef DEBUG_RELEASE
  #define _DEBUG 1
#endif

#ifdef _DEBUG
//	#include <crtdbg.h>
	void init_debug(void);
    void end_debug (void);
	void do_log(const char *pszFormat, ...);
	#define DEBUG_OUT(a) OUTPUT(a)
	#define LOG(a) do_log a
#else
	#define DEBUG_OUT(a) 
	#define LOG(a)
#endif

