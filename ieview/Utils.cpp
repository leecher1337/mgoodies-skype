#include "Utils.h"

void Utils::appendText(char **str, int *sizeAlloced, const char *fmt, ...) {
	va_list vararg;
	char *p;
	int size, len;

	if (str == NULL) return;

	if (*str==NULL || *sizeAlloced<=0) {
		*sizeAlloced = size = 2048;
		*str = (char *) malloc(size);
		len = 0;
	}
	else {
		len = strlen(*str);
		size = *sizeAlloced - strlen(*str);
	}

	p = *str + len;
	va_start(vararg, fmt);
	while (_vsnprintf(p, size, fmt, vararg) == -1) {
		size += 2048;
		(*sizeAlloced) += 2048;
		*str = (char *) realloc(*str, *sizeAlloced);
		p = *str + len;
	}
	va_end(vararg);
}

void Utils::appendText(wchar_t **str, int *sizeAlloced, const wchar_t *fmt, ...) {
	va_list vararg;
	wchar_t *p;
	int size, len;

	if (str == NULL) return;

	if (*str==NULL || *sizeAlloced<=0) {
		*sizeAlloced = size = 2048;
		*str = (wchar_t *) malloc(size);
		len = 0;
	}
	else {
		len = wcslen(*str);
		size = *sizeAlloced - sizeof(wchar_t) * wcslen(*str);
	}

	p = *str + len;
	va_start(vararg, fmt);
	while (_vsnwprintf(p, size / sizeof(wchar_t), fmt, vararg) == -1) {
		size += 2048;
		(*sizeAlloced) += 2048;
		*str = (wchar_t *) realloc(*str, *sizeAlloced);
		p = *str + len;
	}
	va_end(vararg);
}

char *Utils::dupString(const char *a) {
	if (a!=NULL) {
		char *b = new char[strlen(a)+1];
		strcpy(b, a);
		return b;
	}
	return NULL;
}

char *Utils::dupString(const char *a, int l) {
	if (a!=NULL) {
		char *b = new char[l+1];
		strncpy(b, a, l);
		b[l] ='\0';
		return b;
	}
	return NULL;
}

wchar_t *Utils::dupString(const wchar_t *a) {
	if (a!=NULL) {
		wchar_t *b = new wchar_t[wcslen(a)+1];
		wcscpy(b, a);
		return b;
	}
	return NULL;
}

wchar_t *Utils::dupString(const wchar_t *a, int l) {
	if (a!=NULL) {
		wchar_t *b = new wchar_t[l+1];
		wcsncpy(b, a, l);
		b[l] ='\0';
		return b;
	}
	return NULL;
}


wchar_t *Utils::convertToWCS(const char *a) {
	if (a!=NULL) {
		int len = strlen(a)+1;
		wchar_t *b = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, a, len, b, len);
		return b;
	}
	return NULL;
}

wchar_t *Utils::convertToWCS(const char *a, int cp) {
	if (a!=NULL) {
		int len = strlen(a)+1;
		wchar_t *b = new wchar_t[len];
		MultiByteToWideChar(cp, 0, a, len, b, len);
		return b;
	}
	return NULL;
}

char *Utils::convertToString(const wchar_t *a) {
	if (a!=NULL) {
		int len = wcslen(a)+1;
		char *b = new char[len];
		WideCharToMultiByte(CP_ACP, 0, a, len, b, len, NULL, FALSE);
		return b;
	}
	return NULL;
}


void Utils::convertPath(char *path) {
   	for (; *path!='\0'; path++) {
   	    if (*path == '\\') *path = '/';
   	}
}


DWORD Utils::safe_wcslen(wchar_t *msg, DWORD maxLen) {
    DWORD i;
	for (i = 0; i < maxLen; i++) {
		if (msg[i] == (wchar_t)0)
			return i;
	}
	return 0;
}

char * Utils::UTF8Encode(const wchar_t *wtext) {
	unsigned char *szOut;
	int len, i;
	const wchar_t *w;

	if (wtext == NULL) return NULL;
	for (len=0, w=wtext; *w; w++) {
		if (*w < 0x0080) len++;
		else if (*w < 0x0800) len += 2;
		else len += 3;
	}
	szOut = new unsigned char [len+1];
	if (szOut == NULL) return NULL;

	for (i=0, w=wtext; *w; w++) {
		if (*w < 0x0080)
			szOut[i++] = (unsigned char) *w;
		else if (*w < 0x0800) {
			szOut[i++] = 0xc0 | ((*w) >> 6);
			szOut[i++] = 0x80 | ((*w) & 0x3f);
		}
		else {
			szOut[i++] = 0xe0 | ((*w) >> 12);
			szOut[i++] = 0x80 | (((*w) >> 6) & 0x3f);
			szOut[i++] = 0x80 | ((*w) & 0x3f);
		}
	}
	szOut[i] = '\0';
	return (char *) szOut;
}

char *Utils::UTF8Encode(const char *text) {
    wchar_t *wtext = Utils::convertToWCS(text);
    char *atext = UTF8Encode(wtext);
	delete wtext;
	return atext;
}

void Utils::UTF8Encode(const char *text, char *output, int maxLen) {
    wchar_t *wtext = Utils::convertToWCS(text);
    char *atext = UTF8Encode(wtext);
    memcpy(output, atext, min ((int)strlen(atext)+1, maxLen));
	delete atext;
	delete wtext;
}
