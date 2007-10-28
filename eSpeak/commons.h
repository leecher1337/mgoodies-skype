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


#ifndef __COMMONS_H__
# define __COMMONS_H__


#define OEMRESOURCE 
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <time.h>


// Miranda headers
#define MIRANDA_VER 0x0700
#include <newpluginapi.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_clist.h>
#include <m_contacts.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_updater.h>
#include <m_metacontacts.h>
#include <m_popup.h>
#include <m_history.h>
#include <m_message.h>
#include <m_folders.h>
#include <m_icolib.h>
#include <m_userinfo.h>

#include "../utils/mir_memory.h"
#include "../utils/mir_options.h"
#include "../utils/mir_icons.h"
#include "../utils/ContactAsyncQueue.h"

#include "resource.h"
#include "m_speak.h"
#include "options.h"

#include "eSpeak/speak_lib.h"


#define MODULE_NAME		"eSpeak"


// Global Variables
extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;


#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )


#define ICON_SIZE 16
#define NAME_SIZE 128

#define GENDER_MALE 1
#define GENDER_FEMALE 1

int SortVoices(const Voice *voice1, const Voice *voice2);


class Variant
{
public:
	char name[NAME_SIZE];
	int gender;

	Variant(char *aName, int aGender)
	{
		lstrcpynA(name, aName, MAX_REGS(name));
		gender = aGender;
	}
};


class Voice
{
public:
	char name[NAME_SIZE];
	int prio;
	int gender;

	Voice(char *aName, int aPrio, int aGender)
	{
		lstrcpynA(name, aName, MAX_REGS(name));
		prio = aPrio;
		gender = aGender;
	}
};


class Language 
{
public:
	TCHAR language[NAME_SIZE];
	TCHAR localized_name[NAME_SIZE];
	TCHAR english_name[NAME_SIZE];
	TCHAR full_name[NAME_SIZE];

	LIST<Voice> voices;

	Language(TCHAR *aLanguage)
		: voices(5, SortVoices)
	{
		lstrcpyn(language, aLanguage, MAX_REGS(language));
		localized_name[0] = _T('\0');
		english_name[0] = _T('\0');
		full_name[0] = _T('\0');
	}
};


extern LIST<Language> languages;
extern LIST<Variant> variants;
extern ContactAsyncQueue *queue;


void Speak(Voice *voice, Variant *var, TCHAR *text);
Language *GetLanguage(TCHAR *language, BOOL create = FALSE);
Language *GetContactLanguage(HANDLE hContact);
Voice *GetContactVoice(HANDLE hContact, Language *lang);
Variant *GetContactVariant(HANDLE hContact);
void GetLangPackLanguage(TCHAR *name, size_t len);
HICON LoadIconEx(Language *lang, BOOL copy = FALSE);



#endif // __COMMONS_H__
