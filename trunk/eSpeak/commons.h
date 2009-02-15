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
#include <sapi.h>
#include <vector>


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
#include <m_idle.h>

#include "../utils/mir_memory.h"
#include "../utils/mir_options.h"
#include "../utils/mir_icons.h"
#include "../utils/mir_buffer.h"
#include "../utils/ContactAsyncQueue.h"
#include "../utils/utf8_helpers.h"

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
#define RELEASE(_A_) if (_A_ != NULL) { _A_->Release(); _A_ = NULL; }


#define ICON_SIZE 16
#define NAME_SIZE 128

#define GENDER_UNKNOWN 0
#define GENDER_MALE 1
#define GENDER_FEMALE 2

int SortVoices(const Voice *voice1, const Voice *voice2);

enum Engine
{
	ENGINE_ESPEAK,
	ENGINE_SAPI
};

class Variant
{
public:
	TCHAR name[NAME_SIZE];
	int gender;
	TCHAR id[NAME_SIZE];

	Variant(const TCHAR *aName, int aGender, const TCHAR *anId)
	{
		lstrcpyn(name, aName, MAX_REGS(name));
		gender = aGender;
		lstrcpyn(id, anId, MAX_REGS(id));
	}
};


class Voice
{
public:
	Engine engine;
	TCHAR name[NAME_SIZE];
	int prio;
	int gender;
	TCHAR age[NAME_SIZE];
	TCHAR id[NAME_SIZE];

	Voice(Engine anEngine, const TCHAR *aName, int aPrio, int aGender, const TCHAR *anAge, const TCHAR *anId)
	{
		engine = anEngine;
		lstrcpyn(name, aName, MAX_REGS(name));
		prio = aPrio;
		gender = aGender;
		lstrcpyn(age, anAge, MAX_REGS(age));
		lstrcpyn(id, anId, MAX_REGS(id));
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

struct RANGE
{
	int min;
	int max;
	int def;
};

static struct {
	espeak_PARAMETER eparam;
	RANGE espeak;
	RANGE sapi;
	char *setting;
	int ctrl;
	int label;
	int type;
} PARAMETERS[] = {
	{ espeakRATE, { 80, 389, 165 }, { -10, 10, 0 }, "Rate", IDC_RATE, IDC_RATE_L, SCROLL }, 
	{ espeakVOLUME, { 10, 190, 100 }, { 0, 100, 100 }, "Volume", IDC_VOLUME, IDC_VOLUME_L, SCROLL }, 
	{ espeakPITCH, { 0, 99, 50 }, { 0, -1, 0 }, "Pitch", IDC_PITCH, IDC_PITCH_L, SCROLL }, 
	{ espeakRANGE, { -100, 99, 50 }, { 0, -1, 0 }, "Range", IDC_RANGE, IDC_RANGE_L, SCROLL },
	{ espeakPUNCTUATION, { espeakPUNCT_NONE, espeakPUNCT_SOME, espeakPUNCT_SOME }, { 0, -1, 0 }, "Punctuation", IDC_PUNCT, IDC_PUNCT_L, COMBO }
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

int SAPI_GetDefaultRateFor(TCHAR *id);

HICON LoadIconEx(Language *lang, BOOL copy = FALSE);

#define SPEAK_NAME "SpeakName"
#define TEMPLATE_ENABLED "Enabled"
#define TEMPLATE_TEXT "Text"


#endif // __COMMONS_H__
