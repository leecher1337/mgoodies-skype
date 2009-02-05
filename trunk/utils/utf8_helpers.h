#ifndef __UTF8_HELPERS_H__
# define __UTF8_HELPERS_H__

#include <windows.h>
#include <newpluginapi.h>
#include <m_system.h>


class TcharToUtf8
{
public:
	TcharToUtf8(const char *str) : utf8(NULL)
	{
		int size = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
		if (size <= 0)
			throw _T("Could not convert string to WCHAR");

		WCHAR *tmp = (WCHAR *) mir_alloc(size * sizeof(WCHAR));
		if (tmp == NULL)
			throw _T("mir_alloc returned NULL");

		MultiByteToWideChar(CP_ACP, 0, str, -1, tmp, size);

		init(tmp);

		mir_free(tmp);
	}


	TcharToUtf8(const WCHAR *str) : utf8(NULL)
	{
		init(str);
	}


	~TcharToUtf8()
	{
		if (utf8 != NULL)
			mir_free(utf8);
	}

	operator char *()
	{
		return utf8;
	}

private:
	char *utf8;

	void init(const WCHAR *str)
	{
		int size = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
		if (size <= 0)
			throw _T("Could not convert string to UTF8");

		utf8 = (char *) mir_alloc(size);
		if (utf8 == NULL)
			throw _T("mir_alloc returned NULL");

		WideCharToMultiByte(CP_UTF8, 0, str, -1, utf8, size, NULL, NULL);
	}
};



class Utf8ToTchar
{
public:
	Utf8ToTchar(const char *str) : tchar(NULL)
	{
		if (str == NULL)
			return;

		int size = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
		if (size <= 0)
			throw _T("Could not convert string to WCHAR");

		WCHAR *tmp = (WCHAR *) mir_alloc(size * sizeof(WCHAR));
		if (tmp == NULL)
			throw _T("mir_alloc returned NULL");

		MultiByteToWideChar(CP_UTF8, 0, str, -1, tmp, size);

#ifdef UNICODE

		tchar = tmp;

#else

		size = WideCharToMultiByte(CP_ACP, 0, tmp, -1, NULL, 0, NULL, NULL);
		if (size <= 0)
		{
			mir_free(tmp);
			throw _T("Could not convert string to ACP");
		}

		tchar = (TCHAR *) mir_alloc(size * sizeof(char));
		if (tchar == NULL)
		{
			mir_free(tmp);
			throw _T("mir_alloc returned NULL");
		}

		WideCharToMultiByte(CP_ACP, 0, tmp, -1, tchar, size, NULL, NULL);

		mir_free(tmp);

#endif
	}

	~Utf8ToTchar()
	{
		if (tchar != NULL)
			mir_free(tchar);
	}

	TCHAR *detach()
	{
		TCHAR *ret = tchar;
		tchar = NULL;
		return ret;
	}

	operator TCHAR *()
	{
		return tchar;
	}

private:
	TCHAR *tchar;
};


class CharToTchar
{
public:
	CharToTchar(const char *str) : tchar(NULL)
	{
		if (str == NULL)
			return;

#ifdef UNICODE

		tchar = mir_a2u(str);

#else

		tchar = str;

#endif
	}


	~CharToTchar()
	{
#ifdef UNICODE
		if (tchar != NULL)
			mir_free(tchar);
#endif
	}

	TCHAR *detach()
	{
#ifdef UNICODE
		TCHAR *ret = tchar;
#else
		TCHAR *ret = (tchar == NULL ? NULL : mir_strdup(tchar));
#endif

		tchar = NULL;
		return ret;
	}

	operator const TCHAR *()
	{
		return tchar;
	}

private:
#ifdef UNICODE
	TCHAR *tchar;
#else
	const TCHAR *tchar;
#endif
};


class WcharToTchar
{
public:
	WcharToTchar(const WCHAR *str) : tchar(NULL)
	{
		if (str == NULL)
			return;

#ifdef UNICODE

		tchar = str;

#else

		tchar = mir_u2a(str);

#endif
	}


	~WcharToTchar()
	{
#ifndef UNICODE
		if (tchar != NULL)
			mir_free(tchar);
#endif
	}

	TCHAR *detach()
	{
#ifdef UNICODE
		TCHAR *ret = (tchar == NULL ? NULL : mir_wstrdup(tchar));
#else
		TCHAR *ret = tchar;		
#endif

		tchar = NULL;
		return ret;
	}

	operator const TCHAR *()
	{
		return tchar;
	}

private:
#ifdef UNICODE
	const TCHAR *tchar;
#else
	TCHAR *tchar;
#endif
};




class CharToWchar
{
public:
	CharToWchar(const char *str) : wchar(NULL)
	{
		if (str == NULL)
			return;

		wchar = mir_a2u(str);
	}


	~CharToWchar()
	{
		if (wchar != NULL)
			mir_free(wchar);
	}

	WCHAR *detach()
	{
		WCHAR *ret = wchar;
		wchar = NULL;
		return ret;
	}

	operator const WCHAR *()
	{
		return wchar;
	}

private:
	WCHAR *wchar;
};



class TcharToChar
{
public:
	TcharToChar(const TCHAR *str) : val(NULL)
	{
		if (str == NULL)
			return;

#ifdef UNICODE

		val = mir_u2a(str);

#else

		val = str;

#endif
	}


	~TcharToChar()
	{
#ifdef UNICODE
		if (val != NULL)
			mir_free(val);
#endif
	}

	char *detach()
	{
#ifdef UNICODE
		char *ret = val;
#else
		char *ret = (val == NULL ? NULL : mir_strdup(val));
#endif

		val = NULL;
		return ret;
	}

	operator const char *()
	{
		return val;
	}

private:
#ifdef UNICODE
	char *val;
#else
	const char *val;
#endif
};


class TcharToWchar
{
public:
	TcharToWchar(const TCHAR *str) : val(NULL)
	{
		if (str == NULL)
			return;

#ifdef UNICODE

		val = str;

#else

		val = mir_a2u(str);

#endif
	}


	~TcharToWchar()
	{
#ifndef UNICODE
		if (val != NULL)
			mir_free(val);
#endif
	}

	WCHAR *detach()
	{
#ifdef UNICODE
		WCHAR *ret = (val == NULL ? NULL : mir_wstrdup(val));
#else
		WCHAR *ret = val;
#endif

		val = NULL;
		return ret;
	}

	operator const WCHAR *()
	{
		return val;
	}

private:
#ifdef UNICODE
	const WCHAR *val;
#else
	WCHAR *val;
#endif
};


#endif // __UTF8_HELPERS_H__
