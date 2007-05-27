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


#ifndef __MIR_MEMORY_H__
# define __MIR_MEMORY_H__

#include <windows.h>

#ifdef __cplusplus
extern "C" 
{
#endif


// Need to be called on ME_SYSTEM_MODULESLOADED or Load
void init_mir_malloc();


void * mir_alloc0(size_t size);
char *mir_dupToAscii(WCHAR *ptr);
WCHAR *mir_dupToUnicode(char *ptr);
int strcmpnull(char *str1, char *str2);
int strcmpnullW(WCHAR *str1, WCHAR *str2);


#ifdef _UNICODE
# define mir_dupTA mir_dupToUnicode
# define mir_dupTW mir_wstrdup
# define lstrcmpnull strcmpnullW

#define INPLACE_CHAR_TO_TCHAR(_new_var_, _size_, _old_var_)									\
	TCHAR _new_var_[_size_];																\
	MultiByteToWideChar(CP_ACP, 0, _old_var_, -1, _new_var_, _size_)


#define INPLACE_TCHAR_TO_CHAR(_new_var_, _size_, _old_var_)									\
	char _new_var_[_size_];																	\
	WideCharToMultiByte(CP_ACP, 0, _old_var_, -1, _new_var_, _size_, NULL, NULL);

#else

# define mir_dupTA mir_strdup
# define mir_dupTW mir_dupToAscii
# define lstrcmpnull strcmpnull

#define INPLACE_CHAR_TO_TCHAR(_new_var_, _size_, _old_var_)									\
	TCHAR *_new_var_ = _old_var_

#define INPLACE_TCHAR_TO_CHAR(_new_var_, _size_, _old_var_)									\
	char *_new_var_ = _old_var_;

#endif



// Free memory and set to NULL
#define MIR_FREE(_x_) if (_x_ != NULL) { mir_free(_x_); _x_ = NULL; }


#ifdef __cplusplus
}
#endif

#endif // __MIR_MEMORY_H__
