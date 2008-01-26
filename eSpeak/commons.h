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
#include <commctrl.h>


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
#include "../utils/mir_buffer.h"
#include "../utils/ContactAsyncQueue.h"

#include "resource.h"
#include "m_speak.h"
#include "options.h"
#include "types.h"

#include "eSpeak/speak_lib.h"


#define MODULE_NAME		"meSpeak"


// Global Variables
extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;


#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )


#define ICON_SIZE 16
#define NAME_SIZE 128

#define GENDER_MALE 1
#define GENDER_FEMALE 1

#define STRNCPY(DEST, SRC) strncpy(DEST, SRC, MAX_REGS(DEST)); DEST[MAX_REGS(DEST)-1] = '\0'

int SortVoices(const Voice *voice1, const Voice *voice2);


class Variant
{
public:
	char name[NAME_SIZE];
	int gender;
	char id[NAME_SIZE];

	Variant(const char *aName, int aGender, const char *anId)
	{
		STRNCPY(name, aName);
		gender = aGender;
		STRNCPY(id, anId);
	}
};


class Voice
{
public:
	char name[NAME_SIZE];
	int prio;
	int gender;
	char id[NAME_SIZE];

	Voice(const char *aName, int aPrio, int aGender, const char *anId)
	{
		STRNCPY(name, aName);
		prio = aPrio;
		gender = aGender;
		STRNCPY(id, anId);
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


#define SCROLL 0
#define COMBO 1

static struct {
	espeak_PARAMETER eparam;
	int min;
	int max;
	int def;
	char *setting;
	int ctrl;
	int type;
} PARAMETERS[] = {
	{ espeakRATE, 80, 389, 165, "Rate", IDC_RATE, SCROLL }, 
	{ espeakVOLUME, 10, 190, 100, "Volume", IDC_VOLUME, SCROLL }, 
	{ espeakPITCH, 0, 99, 50, "Pitch", IDC_PITCH, SCROLL }, 
	{ espeakRANGE, -100, 99, 50, "Range", IDC_RANGE, SCROLL },
	{ espeakPUNCTUATION, espeakPUNCT_NONE, espeakPUNCT_SOME, espeakPUNCT_SOME, "Punctuation", IDC_PUNCT, COMBO },
};

#define NUM_PARAMETERS MAX_REGS(PARAMETERS)

class SpeakData
{
public:
	Language *lang;
	Voice *voice;
	Variant *variant;
	TCHAR *text;
	int parameters[NUM_PARAMETERS];

	SpeakData(Language *aLang, Voice *aVoice, Variant *aVariant, TCHAR *aText)
	{
		lang = aLang;
		voice = aVoice;
		variant = aVariant;
		text = aText;
	}

	void setParameter(int param, int value)
	{
		parameters[param] = value;
	}

	int getParameter(int param)
	{
		return parameters[param];
	}
};


extern LIST<Language> languages;
extern LIST<Variant> variants;
extern ContactAsyncQueue *queue;


int SpeakService(HANDLE hContact, TCHAR *text);
void Speak(SpeakData *data);

Language *GetLanguage(TCHAR *language, BOOL create = FALSE);

Language *GetContactLanguage(HANDLE hContact);
Voice *GetContactVoice(HANDLE hContact, Language *lang);
Variant *GetContactVariant(HANDLE hContact);
int GetContactParam(HANDLE hContact, int param);
void SetContactParam(HANDLE hContact, int param, int value);

void GetLangPackLanguage(TCHAR *name, size_t len);

HICON LoadIconEx(Language *lang, BOOL copy = FALSE);

#define SPEAK_NAME "SpeakName"
#define TEMPLATE_ENABLED "Enabled"
#define TEMPLATE_TEXT "Text"


#endif // __COMMONS_H__
