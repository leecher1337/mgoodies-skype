class Utils;

#ifndef UTILS_INCLUDED
#define UTILS_INCLUDED

#include "ieview_common.h"

class Utils {
public:
	static void appendText(char **str, int *sizeAlloced, const char *fmt, ...);
	static void appendText(wchar_t **str, int *sizeAlloced, const wchar_t *fmt, ...);
	static void convertPath(char *path);
	static char *dupString(const char *a);
	static char *dupString(const char *a, int l);
	static wchar_t *dupString(const wchar_t *a);
	static wchar_t *dupString(const wchar_t *a, int l);
	static wchar_t *convertToWCS(const char *a);
	static char *convertToString(const wchar_t *a);
	static DWORD safe_wcslen(wchar_t *msg, DWORD maxLen);

}; 

#endif

