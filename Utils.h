class Utils;

#ifndef UTILS_INCLUDED
#define UTILS_INCLUDED

#include "ieview_common.h"

class Utils {
private:
	static wchar_t base_dir[MAX_PATH];
public:
	static const wchar_t *getBaseDir();
	static wchar_t* toAbsolute(wchar_t* relative);
	static void appendText(char **str, int *sizeAlloced, const char *fmt, ...);
	static void appendText(wchar_t **str, int *sizeAlloced, const wchar_t *fmt, ...);
	static void convertPath(char *path);
	static void convertPath(wchar_t *path);
	static char *dupString(const char *a);
	static char *dupString(const char *a, int l);
	static wchar_t *dupString(const wchar_t *a);
	static wchar_t *dupString(const wchar_t *a, int l);
	static wchar_t *convertToWCS(const char *a);
	static wchar_t *convertToWCS(const char *a, int cp);
	static char *convertToString(const wchar_t *a);
	static char *convertToString(const wchar_t *a, int cp);
	static char *escapeString(const char *a);
	static DWORD safe_wcslen(wchar_t *msg, DWORD maxLen);
	static char *UTF8Encode(const wchar_t *wtext);
	static char *UTF8Encode(const char *text);
	static void  UTF8Encode(const char *text, char *output, int maxLen);
	static int   detectURL(const wchar_t *text);
	static unsigned long forkThread(void (__cdecl *threadcode)(void*),unsigned long stacksize,void *arg);

};

#endif

