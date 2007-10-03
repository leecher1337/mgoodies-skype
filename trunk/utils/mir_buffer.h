/* 
Copyright (C) 2005 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#ifndef __MIR_BUFFER_H__
# define __MIR_BUFFER_H__

#include <windows.h>

#include "mir_memory.h"
#include <m_variables.h>


template<class T>
static int __bvsnprintf(T *str, size_t size, const T *fmt, va_list args)
{
	return 0;
}

template<>
static inline int __bvsnprintf<char>(char *str, size_t size, const char *fmt, va_list args)
{
	return _vsnprintf(str, size, fmt, args);
}

template<>
static inline int __bvsnprintf<wchar_t>(wchar_t *str, size_t size, const wchar_t *fmt, va_list args)
{
	return _vsnwprintf(str, size, fmt, args);
}

template<class T>
static inline int __blen(T *str)
{
	return 0;
}

template<>
static inline int __blen<char>(char *str)
{
	return strlen(str);
}

template<>
static inline int __blen<wchar_t>(wchar_t *str)
{
	return lstrlenW(str);
}

template<class T>
static inline T * __bTranslate(T *str)
{
	return 0;
}

template<>
static inline char * __bTranslate<char>(char *str)
{
	return Translate(str);
}

template<>
static inline wchar_t * __bTranslate<wchar_t>(wchar_t *str)
{
	return TranslateW(str);
}



template<class T>
class Buffer 
{
	public:
		size_t len;
		T *str;

		Buffer() : len(0), str(NULL), size(0) {}

		Buffer(T in) 
		{
			str = in;
			size = len = __blen(in);
		}

		~Buffer()
		{
			free();
		}

		void pack() 
		{
			if (str != NULL)
				str[len] = 0;
		}

		void alloc(size_t total) 
		{
			if (total > size)
			{
				size = total + 256 - total % 256;
				if (str == NULL)
					str = (T *) mir_alloc(size * sizeof(T));
				else
					str = (T *) mir_realloc(str, size * sizeof(T));
			}
		}

		void free()
		{
			if (str != NULL)
			{
				mir_free(str);
				str = NULL;
				len = size = 0;
			}
		}

		void clear() 
		{
			len = 0;
		}

		void append(T app)
		{
			alloc(len + 1);

			str[len] = app;
			len++;
		}

		void append(T *app, size_t appLen = -1)
		{
			if (appLen == -1)
				appLen = __blen(app);

			size_t total = len + appLen + 1;
			alloc(total);

			memmove(&str[len], app, appLen * sizeof(T));
			len += appLen;
		}

		void appendPrintf(T *app, ...)
		{
			size_t total = len + 512;
			alloc(total);

			va_list arg;
			va_start(arg, app);
			total = __bvsnprintf<T>(&str[len], size - len - 1, app, arg);
			if (total < 0)
				total = size - len - 1;
			len += total;
		}

		void insert(size_t pos, T *app, size_t appLen = -1)
		{
			if (pos > len)
				pos = len;
			if (pos < 0)
				pos = 0;

			if (appLen == -1)
				appLen = __blen(app);

			alloc(len + appLen + 1);

			if (pos < len)
				memmove(&str[pos + appLen], &str[pos], (len - pos) * sizeof(T));
			memmove(&str[pos], app, appLen * sizeof(T));

			len += appLen;
		}

		void replace(size_t start, size_t end, T *app, size_t appLen = -1)
		{
			if (start > len)
				start = len;
			if (start < 0)
				start = 0;

			if (end > len)
				end = len;
			if (end < start)
				end = start;

			if (appLen == -1)
				appLen = __blen(app);

			size_t oldLen = end - start;
			if (oldLen < appLen)
				alloc(len + appLen - oldLen + 1);
			
			if (end < len && oldLen != appLen)
				memmove(&str[start + appLen], &str[end], (len - end) * sizeof(T));
			memmove(&str[start], app, appLen * sizeof(T));

			len += appLen - oldLen;
		}

		void translate()
		{
			if (str == NULL || len == 0)
				return;

			str[len] = 0;
			T *tmp = __bTranslate(str);
			len = __blen(tmp);
			alloc(len + 1);
			memmove(str, tmp, len * sizeof(T));
		}

		T *appender(size_t appLen) 
		{
			alloc(len + appLen + 1);
			T *ret = &str[len];
			len += appLen;
			return ret;
		}

		T *lock(size_t maxSize) 
		{
			alloc(len + maxSize + 1);
			return &str[len];
		}

		void release() 
		{
			len += max(__blen(&str[len]), size - len - 1);
		}

		void trimRight() 
		{
			if (str == NULL)
				return;

			int e;
			for(e = len-1; e >= 0 && (str[e] == (T)' ' 
									  || str[e] == (T)'\t' 
									  || str[e] == (T)'\r' 
									  || str[e] == (T)'\n'); e--) ;
			len = e+1;
		}

		void trimLeft() 
		{
			if (str == NULL)
				return;

			int s;
			for(s = 0; str[s] == (T)' ' 
					   || str[s] == (T)'\t' 
					   || str[s] == (T)'\r' 
					   || str[s] == (T)'\n'; s++) ;
			if (s > 0)
			{
				memmove(str, &str[s], (len - s) * sizeof(T));
				len -= s;
			}
		}

		void trim() 
		{
			trimRight();
			trimLeft();
		}


	private:
		size_t size;
};


static void ReplaceVars(Buffer<TCHAR> *buffer, HANDLE hContact, TCHAR **variables, int numVariables)
{
	if (buffer->len < 3)
		return;

	for(size_t i = buffer->len - 1; i > 0; i--)
	{
		if (buffer->str[i] == _T('%'))
		{
			// Find previous
			for(size_t j = i - 1; j > 0 && ((buffer->str[j] >= _T('a') && buffer->str[j] <= _T('z'))
										    || (buffer->str[j] >= _T('A') && buffer->str[j] <= _T('Z'))
											|| buffer->str[j] == _T('-')
											|| buffer->str[j] == _T('_')); j--) ;

			if (buffer->str[j] == _T('%'))
			{
				size_t foundLen = i - j + 1;
				if (foundLen == 9 && _tcsncmp(&buffer->str[j], _T("%contact%"), 9) == 0)
				{
					buffer->replace(j, i + 1, (TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) hContact, GCDNF_TCHAR));
				}
				else if (foundLen == 6 && _tcsncmp(&buffer->str[j], _T("%date%"), 6) == 0)
				{
					TCHAR tmp[128];
					DBTIMETOSTRINGT tst = {0};
					tst.szFormat = _T("d s");
					tst.szDest = tmp;
					tst.cbDest = 128;
					CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM) time(NULL), (LPARAM) &tst);
					buffer->replace(j, i + 1, tmp);
				}
				else
				{
					for(int k = 0; k < numVariables; k += 2)
					{
						size_t len = lstrlen(variables[k]);
						if (foundLen == len + 2 && _tcsncmp(&buffer->str[j]+1, variables[k], len) == 0)
						{
							buffer->replace(j, i + 1, variables[k + 1]);
							break;
						}
					}
				}
			}

			i = j;
			if (i == 0)
				break;
		}
		else if (buffer->str[i] == _T('\\') && i+1 <= buffer->len-1 && buffer->str[i+1] == _T('n')) 
		{
			buffer->str[i] = _T('\r');
			buffer->str[i+1] = _T('\n');
		}
	}
}


static void ReplaceTemplate(Buffer<TCHAR> *out, HANDLE hContact, TCHAR *templ, TCHAR **vars, int numVars)
{
	if (ServiceExists(MS_VARS_FORMATSTRING)) 
	{
		TCHAR *tmp = variables_parse_ex(templ, NULL, hContact, vars, numVars);
		out->append(tmp);
		variables_free(tmp);
	}
	else
	{
		out->append(templ);
		ReplaceVars(out, hContact, vars, numVars);
	}
	out->pack();
}


#endif // __MIR_BUFFER_H__
