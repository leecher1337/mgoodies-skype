/* 
Copyright (C) 2006 Ricardo Pescuma Domenecci

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
#include <richedit.h>
#include <tom.h>
#include <richole.h>


// Disable "...truncated to '255' characters in the debug information" warnings
#pragma warning(disable: 4786)

#include <map>
#include <vector>
#include <string>
using namespace std;


#ifdef __cplusplus
extern "C" 
{
#endif

// Miranda headers
#include <newpluginapi.h>
#include <m_system.h>
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
#include <IcoLib.h>

#include "../utils/mir_memory.h"
#include "../utils/mir_options.h"

#include "resource.h"
#include "m_spellchecker.h"
#include "options.h"

#ifdef __cplusplus
}
#endif


#include "dictionary.h"


#define MODULE_NAME		"SpellChecker"


// Global Variables
extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;


#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )


#define ICON_SIZE 16


extern Dictionaries languages;

struct WrongWordPopupMenuData {
	Suggestions suggestions;
	TCHAR *word;
	CHARRANGE pos;
	HMENU hMeSubMenu;
	HMENU hCorrectSubMenu;
	HMENU hReplaceSubMenu;
};

struct Dialog {
	HWND hwnd;
	HWND hwnd_owner;
	HANDLE hContact;
	char name[64];
	Dictionary *lang;
	TCHAR lang_name[10];
	WNDPROC old_edit_proc;
	BOOL enabled;
	BOOL srmm;

	IRichEditOle *ole;
	ITextDocument *textDocument;

	HWND hwnd_menu_owner;
	WNDPROC old_menu_proc;

	BOOL changed;
	int old_text_len;

	// Popup data
	HMENU hLanguageSubMenu;
	HMENU hWrongWordsSubMenu;
	vector<WrongWordPopupMenuData> *wrong_words;
};



extern HICON hEnabledIcon;
extern HICON hDisabledIcon;
extern HICON hUnknownFlag;

extern HINSTANCE hFlagsDll;

#endif // __COMMONS_H__
