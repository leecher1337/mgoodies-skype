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
		size = *sizeAlloced - wcslen(*str);
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


void Utils::convertPath(char *path) {
   	for (; *path!='\0'; path++) {
   	    if (*path == '\\') *path = '/';
   	} 
}
    
