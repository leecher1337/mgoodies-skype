/* 
Copyright (C) 2007 Ricardo Pescuma Domenecci

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


#ifndef __M_SPEAK_H__
# define __M_SPEAK_H__


#define MIID_SPEAK { 0x1ef72725, 0x6a83, 0x483b, { 0xaa, 0x50, 0x89, 0x53, 0xe3, 0x59, 0xee, 0xad } }


/*
Speak a text

wParam: (HANDLE) hContact
lParam: (char *) text
return: 0 on success
*/
#define MS_SPEAK_SAY_A	"Speak/Say"


/*
Speak a unicode text

wParam: (HANDLE) hContact
lParam: (WCHAR *) text
return: 0 on success
*/
#define MS_SPEAK_SAY_W	"Speak/SayW"


typedef struct {
	int cbSize;

	const char *module;
	const char *name;					// Internal type name
	const char *description;			// Will be translated
	const char *icon;					// Name off icolib icon

	// Aditional data if wants to use add to history services
	const char **templates; // Each entry is: "Name\nDefault\n%var%\tDescription\n%var%\tDescription\n%var%\tDescription"
	int numTemplates;

} SPEAK_TYPE;


/*
Register and speak type

wParam: (SPEAK_TYPE *) type
lParam: 0
return: 0 on success
*/
#define MS_SPEAK_REGISTER	"Speak/Register"


#define SPEAK_CHAR 1
#define SPEAK_WCHAR 2

typedef struct {
	int cbSize;

	const char *type;		// Internal type name
	HANDLE hContact;
	int flags;				// SPEAK_*
	
	int templateNum;		// -1 to use text
	union
	{
		const void *text;

		struct
		{
			const void *variables;
			int numVariables;
		};
	};

} SPEAK_ITEM;


/*
Speak a text

wParam: (SPEAK_ITEM *) Item
lParam: 0
return: 0 on success
*/
#define MS_SPEAK_SAYEX	"Speak/SayEx"



// Helper functions

static int Speak_SayA(HANDLE hContact, const char *text)
{
	return CallService(MS_SPEAK_SAY_A, (WPARAM) hContact, (LPARAM) text);
}

static int Speak_SayW(HANDLE hContact, const WCHAR *text)
{
	return CallService(MS_SPEAK_SAY_W, (WPARAM) hContact, (LPARAM) text);
}

static int Speak_Register(char *module, char *name, char *description, char *icon)
{
	SPEAK_TYPE type;

	if (!ServiceExists(MS_SPEAK_REGISTER))
		return -1;

	type.cbSize = sizeof(type);
	type.module = module;
	type.name = name;
	type.description = description;
	type.icon = icon;
	type.templates = NULL;
	type.numTemplates = 0;

	return CallService(MS_SPEAK_REGISTER, (WPARAM) &type, 0);
}

static int Speak_RegisterWT(const char *module, const char *name, const char *description, 
							const char *icon, const char **templates, int numTemplates)
{
	SPEAK_TYPE type;

	if (!ServiceExists(MS_SPEAK_REGISTER))
		return -1;

	type.cbSize = sizeof(type);
	type.module = module;
	type.name = name;
	type.description = description;
	type.icon = icon;
	type.templates = NULL;
	type.numTemplates = 0;

	return CallService(MS_SPEAK_REGISTER, (WPARAM) &type, 0);
}

static int Speak_SayExA(char *type, HANDLE hContact, const char *text)
{
	SPEAK_ITEM item;

	if (!ServiceExists(MS_SPEAK_SAYEX))
		// Try old service
		return Speak_SayA(hContact, text);

	item.cbSize = sizeof(item);
	item.flags = SPEAK_CHAR;
	item.type = type;
	item.hContact = hContact;
	item.templateNum = -1;
	item.text = text;

	return CallService(MS_SPEAK_SAYEX, (WPARAM) &item, 0);
}

static int Speak_SayExW(char *type, HANDLE hContact, const WCHAR *text)
{
	SPEAK_ITEM item;

	if (!ServiceExists(MS_SPEAK_SAYEX))
		// Try old service
		return Speak_SayW(hContact, text);

	item.cbSize = sizeof(item);
	item.flags = SPEAK_WCHAR;
	item.type = type;
	item.hContact = hContact;
	item.templateNum = -1;
	item.text = text;

	return CallService(MS_SPEAK_SAYEX, (WPARAM) &item, 0);
}

static int Speak_SayExWTA(char *type, HANDLE hContact, int templateNum, const char **variables, int numVariables)
{
	SPEAK_ITEM item;

	if (!ServiceExists(MS_SPEAK_SAYEX))
		return -1;

	item.cbSize = sizeof(item);
	item.flags = SPEAK_CHAR;
	item.type = type;
	item.hContact = hContact;
	item.templateNum = templateNum;
	item.variables = variables;
	item.numVariables = numVariables;

	return CallService(MS_SPEAK_SAYEX, (WPARAM) &item, 0);
}

static int Speak_SayExWTW(char *type, HANDLE hContact, int templateNum, const WCHAR **variables, int numVariables)
{
	SPEAK_ITEM item;

	if (!ServiceExists(MS_SPEAK_SAYEX))
		return -1;

	item.cbSize = sizeof(item);
	item.flags = SPEAK_CHAR;
	item.type = type;
	item.hContact = hContact;
	item.templateNum = templateNum;
	item.variables = variables;
	item.numVariables = numVariables;

	return CallService(MS_SPEAK_SAYEX, (WPARAM) &item, 0);
}


#ifdef UNICODE
#  define MS_SPEAK_SAY MS_SPEAK_SAY_W
#  define Speak_Say Speak_SayW
#  define Speak_SayEx Speak_SayExW
#  define Speak_SayExWT Speak_SayWTW
#else
#  define MS_SPEAK_SAY MS_SPEAK_SAY_A
#  define Speak_Say Speak_SayA
#  define Speak_SayEx Speak_SayExA
#  define Speak_SayExWT Speak_SayWTA
#endif


#endif // __M_SPEAK_H__
