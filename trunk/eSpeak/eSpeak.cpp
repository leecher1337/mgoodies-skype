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

#include "commons.h"


// Prototypes ///////////////////////////////////////////////////////////////////////////


PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
#ifdef UNICODE
	"meSpeak (Unicode)",
#else
	"meSpeak",
#endif
	PLUGIN_MAKE_VERSION(0,0,0,9),
	"Speaker plugin based on eSpeak engine (%s)",
	"Ricardo Pescuma Domenecci",
	"",
	"© 2007-2008 Ricardo Pescuma Domenecci",
	"http://pescuma.org/miranda/meSpeak",
	UNICODE_AWARE,
	0,		//doesn't replace anything built-in
#ifdef UNICODE
	{ 0x9c1fad62, 0x8f94, 0x484b, { 0x8e, 0x96, 0xff, 0x6d, 0xa4, 0xcd, 0x98, 0x95 } } // {9C1FAD62-8F94-484b-8E96-FF6DA4CD9895}
#else
	{ 0xb8b98db2, 0xa8e5, 0x4501, { 0x91, 0xf5, 0x6f, 0x3b, 0x44, 0x34, 0x81, 0xa8 } } // {B8B98DB2-A8E5-4501-91F5-6F3B443481A8}
#endif
};


HINSTANCE hInst;
PLUGINLINK *pluginLink;
LIST_INTERFACE li;

static HANDLE hHooks[3] = {0};
static HANDLE hServices[2] = {0};

LIST<Language> languages(20);
LIST<Variant> variants(20);
ContactAsyncQueue *queue;

HANDLE hDictionariesFolder = NULL;
TCHAR dictionariesFolder[1024];

HANDLE hFlagsDllFolder = NULL;
TCHAR flagsDllFolder[1024];

HBITMAP hCheckedBmp;
BITMAP bmpChecked;

char *metacontacts_proto = NULL;
BOOL loaded = FALSE;

int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int PreShutdown(WPARAM wParam, LPARAM lParam);

int SpeakAService(WPARAM wParam, LPARAM lParam);
int SpeakWService(WPARAM wParam, LPARAM lParam);
TCHAR * VariablesSpeak(ARGUMENTSINFO *ai);

LRESULT CALLBACK MenuWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void LoadESpeak();

Language *GetClosestLanguage(TCHAR *lang_name);

void Speak(HANDLE hContact, void *param);

TCHAR *aditionalLanguages[] = {
	_T("en_R"), _T("English (Rhotic)"),
	_T("en_SC"), _T("English (Scottish)"),
	_T("en_UK"), _T("English - UK"),
	_T("en_UK_NORTH"), _T("English - UK (Northern)"),
	_T("en_UK_RP"), _T("English - UK (Received Pronunciation)"),
	_T("en_UK_WMIDS"), _T("English - UK (West Midlands)"),
	_T("en_WI"), _T("English - Westindies"),
	_T("es_LA"), _T("Spanish - Latin American"),
	_T("eo"), _T("Esperanto"),
	_T("la"), _T("Latin"),
	_T("no"), _T("Norwegian"),
	_T("jbo"), _T("Lojban"),
	_T("sr"), _T("Serbian"),
	_T("grc"), _T("Ancient Greek"),
	_T("yue"), _T("Cantonese"),
	_T("ku"), _T("Kurdish"),
};


BOOL shutDown = FALSE;


// Functions ////////////////////////////////////////////////////////////////////////////


extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) 
{
	hInst = hinstDLL;
	return TRUE;
}

static void FixPluginDescription()
{
	static char description[128];
	_snprintf(description, MAX_REGS(description), "Speaker plugin based on eSpeak engine (%s)", espeak_Info(NULL));
	description[MAX_REGS(description)-1] = '\0';
	pluginInfo.description = description;
}

extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion) 
{
	pluginInfo.cbSize = sizeof(PLUGININFO);
	FixPluginDescription();
	return (PLUGININFO*) &pluginInfo;
}


extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	pluginInfo.cbSize = sizeof(PLUGININFOEX);
	FixPluginDescription();
	return &pluginInfo;
}


static const MUUID interfaces[] = { MIID_SPEAK, MIID_LAST };
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}


extern "C" int __declspec(dllexport) Load(PLUGINLINK *link) 
{
	pluginLink = link;

	CHECK_VERSION("meSpeak")

	init_mir_malloc();
	mir_getLI(&li);

	// hooks
	hHooks[0] = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hHooks[1] = HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);

	hCheckedBmp = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_CHECK));
	if (GetObject(hCheckedBmp, sizeof(bmpChecked), &bmpChecked) == 0)
		bmpChecked.bmHeight = bmpChecked.bmWidth = 10;

	InitTypes();

	TCHAR mirandaFolder[1024];
	GetModuleFileName(GetModuleHandle(NULL), mirandaFolder, MAX_REGS(mirandaFolder));
	TCHAR *p = _tcsrchr(mirandaFolder, _T('\\'));
	if (p != NULL)
		*p = _T('\0');

    // Folders plugin support
	if (ServiceExists(MS_FOLDERS_REGISTER_PATH))
	{
		hDictionariesFolder = (HANDLE) FoldersRegisterCustomPathT(Translate("meSpeak"), 
					Translate("Languages"), 
					_T(MIRANDA_PATH) _T("\\Dictionaries\\Voice"));

		FoldersGetCustomPathT(hDictionariesFolder, dictionariesFolder, MAX_REGS(dictionariesFolder), _T("."));

		hFlagsDllFolder = (HANDLE) FoldersRegisterCustomPathT(Translate("meSpeak"), 
					Translate("Flags DLL"), 
					_T(MIRANDA_PATH) _T("\\Icons"));

		FoldersGetCustomPathT(hFlagsDllFolder, flagsDllFolder, MAX_REGS(flagsDllFolder), _T("."));
	}
	else
	{
		mir_sntprintf(dictionariesFolder, MAX_REGS(dictionariesFolder), _T("%s\\Dictionaries\\Voice"), mirandaFolder);

		mir_sntprintf(flagsDllFolder, MAX_REGS(flagsDllFolder), _T("%s\\Icons"), mirandaFolder);
	}

	LoadESpeak();

	hServices[0] = CreateServiceFunction(MS_SPEAK_SAY_A, SpeakAService);
	hServices[1] = CreateServiceFunction(MS_SPEAK_SAY_W, SpeakWService);

	queue = new ContactAsyncQueue(&Speak);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void) 
{
	FreeTypes();

	DeleteObject(hCheckedBmp);

	for(unsigned i = 0; i < MAX_REGS(hServices); ++i)
		DestroyServiceFunction(hServices[i]);

	return 0;
}


HICON LoadIconEx(Language *lang, BOOL copy)
{
#ifdef UNICODE
	char tmp[NAME_SIZE];
	WideCharToMultiByte(CP_ACP, 0, lang->language, -1, tmp, MAX_REGS(tmp), NULL, NULL);
	return LoadIconEx(tmp, copy);
#else
	return LoadIconEx(lang->language, copy);
#endif
}


// Called when all the modules are loaded
int ModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
	if (ServiceExists(MS_MC_GETPROTOCOLNAME))
		metacontacts_proto = (char *) CallService(MS_MC_GETPROTOCOLNAME, 0, 0);

	// add our modules to the KnownModules list
	CallService("DBEditorpp/RegisterSingleModule", (WPARAM) MODULE_NAME, 0);

    // updater plugin support
    if(ServiceExists(MS_UPDATE_REGISTER))
	{
		Update upd = {0};
		char szCurrentVersion[30];

		upd.cbSize = sizeof(upd);
		upd.szComponentName = pluginInfo.shortName;

		upd.szUpdateURL = UPDATER_AUTOREGISTER;

		upd.szBetaVersionURL = "http://pescuma.org/miranda/meSpeak_version.txt";
		upd.szBetaChangelogURL = "http://pescuma.org/miranda/mespeak#Changelog";
		upd.pbBetaVersionPrefix = (BYTE *)"meSpeak ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
#ifdef UNICODE
		upd.szBetaUpdateURL = "http://pescuma.org/miranda/meSpeakW.zip";
#else
		upd.szBetaUpdateURL = "http://pescuma.org/miranda/meSpeak.zip";
#endif

		upd.pbVersion = (BYTE *)CreateVersionStringPlugin((PLUGININFO*) &pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);

        CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}

	InitOptions();

	if (opts.use_flags)
	{
		// Load flags dll
		TCHAR flag_file[1024];
		_sntprintf(flag_file, MAX_REGS(flag_file), _T("%s\\flags.dll"), flagsDllFolder);
		flag_file[1023] = 0;
		HMODULE hFlagsDll = LoadLibrary(flag_file);

		char path[1024];
		GetModuleFileNameA(hInst, path, MAX_REGS(path));

		SKINICONDESC sid = {0};
		sid.cbSize = sizeof(SKINICONDESC);
		sid.pszDefaultFile = path;
		sid.flags = SIDF_TCHAR | SIDF_SORTED;
		sid.ptszSection = TranslateT("Languages/Flags");

		// Get language flags
		for (int i = 0; i < languages.getCount(); i++)
		{
			sid.ptszDescription = languages[i]->full_name;
#ifdef UNICODE
			char lang[32];
			mir_snprintf(lang, MAX_REGS(lang), "%S", languages[i]->language);
			sid.pszName = lang;
#else
			sid.pszName = languages[i]->language;
#endif

			HICON hFlag = LoadIconEx(sid.pszName);
			if (hFlag != NULL)
			{
				// Already registered
				ReleaseIconEx(hFlag);
				continue;
			}
			
			if (hFlagsDll != NULL)
			{
				hFlag = (HICON) LoadImage(hFlagsDll, languages[i]->language, IMAGE_ICON, 16, 16, 0);

				if (hFlag == NULL)
				{
					TCHAR tmp[NAME_SIZE];
					lstrcpyn(tmp, languages[i]->language, MAX_REGS(tmp));
					do
					{
						TCHAR *p = _tcsrchr(tmp, _T('_'));
						if (p == NULL)
							break;

						*p = _T('\0');
						hFlag = (HICON) LoadImage(hFlagsDll, tmp, IMAGE_ICON, 16, 16, 0);
					}
					while(hFlag == NULL);
				}
			}
			else
				hFlag = NULL;

			if (hFlag != NULL)
			{
				sid.hDefaultIcon = hFlag;
				sid.pszDefaultFile = NULL;
				sid.iDefaultIndex = 0;
			}
			else
			{
				sid.hDefaultIcon = NULL;
				sid.pszDefaultFile = path;
				sid.iDefaultIndex = - IDI_UNKNOWN_FLAG;
			}

			// Oki, lets add to IcoLib, then
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
			
			if (hFlag != NULL)
				DestroyIcon(hFlag);
		}
		FreeLibrary(hFlagsDll);
	}

	// Variables support
	if (ServiceExists(MS_VARS_REGISTERTOKEN))
	{
		TOKENREGISTER tr = {0};
		tr.cbSize = sizeof(TOKENREGISTER);
		tr.memType = TR_MEM_MIRANDA;
		tr.flags = TRF_FREEMEM | TRF_PARSEFUNC | TRF_FUNCTION | TRF_TCHAR;

		tr.tszTokenString = _T("speak");
		tr.parseFunctionT = VariablesSpeak;
		tr.szHelpText = "Speak\t(x,[y])\tSpeak the text x using the y contact voice (y is optional)";
		CallService(MS_VARS_REGISTERTOKEN, 0, (LPARAM) &tr);
	}

	loaded = TRUE;

	return 0;
}


Language *GetLanguage(TCHAR *language, BOOL create)
{
	for (int i = 0; i < languages.getCount(); i++)
	{
		Language *lang = languages[i];
		if (lstrcmpi(lang->language, language) == 0)
			return lang;
	}

	if (create)
	{
		Language *lang = new Language(language);
		languages.insert(lang);
		return lang;
	}

	return NULL;
}


int SortVoices(const Voice *voice1, const Voice *voice2)
{
	return (int) voice1->prio - (int) voice2->prio;
}


// To get the names of the languages
BOOL CALLBACK EnumLocalesProc(LPTSTR lpLocaleString)
{
	TCHAR *stopped = NULL;
	USHORT langID = (USHORT) _tcstol(lpLocaleString, &stopped, 16);

	TCHAR ini[32];
	TCHAR end[32];
	GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SISO639LANGNAME, ini, MAX_REGS(ini));
	GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SISO3166CTRYNAME, end, MAX_REGS(end));

	TCHAR name[64];
	mir_sntprintf(name, MAX_REGS(name), _T("%s_%s"), ini, end);
/*
OutputDebugString(name);
OutputDebugStringA(" : ");
TCHAR tmp[128];
GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SENGLANGUAGE, tmp, MAX_REGS(tmp));
OutputDebugString(tmp);
OutputDebugStringA(" | ");
GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SLANGUAGE, tmp, MAX_REGS(tmp));
OutputDebugString(tmp);
OutputDebugStringA(" | ");
GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SNATIVELANGNAME , tmp, MAX_REGS(tmp));
OutputDebugString(tmp);
OutputDebugStringA("\n");
*/
	for (int i = 0; i < languages.getCount(); i++)
	{
		size_t len = lstrlen(languages[i]->language);
		if (len > 2 && lstrcmpi(languages[i]->language, name) == 0)
		{
			GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SLANGUAGE, 
				languages[i]->localized_name, MAX_REGS(languages[i]->localized_name));
			GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SENGLANGUAGE, 
				languages[i]->english_name, MAX_REGS(languages[i]->english_name));

			if (languages[i]->localized_name[0] != _T('\0'))
			{
				mir_sntprintf(languages[i]->full_name, MAX_REGS(languages[i]->full_name), 
					_T("%s [%s]"), TranslateTS(languages[i]->localized_name), languages[i]->language);
			}
		}
		else if (len == 2 && lstrcmpi(languages[i]->language, ini) == 0 && lstrcmpi(languages[i]->language, end) == 0)
		{
			GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SENGLANGUAGE, 
				languages[i]->localized_name, MAX_REGS(languages[i]->localized_name));
			GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SENGLANGUAGE, 
				languages[i]->english_name, MAX_REGS(languages[i]->english_name));

			if (languages[i]->localized_name[0] != _T('\0'))
			{
				mir_sntprintf(languages[i]->full_name, MAX_REGS(languages[i]->full_name), 
					_T("%s [%s]"), TranslateTS(languages[i]->localized_name), languages[i]->language);
			}
		}
		else if (len == 2 && lstrcmpi(languages[i]->language, ini) == 0 && languages[i]->localized_name[0] == _T('\0'))
		{
			GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SENGLANGUAGE, 
				languages[i]->localized_name, MAX_REGS(languages[i]->localized_name));
			GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SENGLANGUAGE, 
				languages[i]->english_name, MAX_REGS(languages[i]->english_name));

			if (languages[i]->localized_name[0] != _T('\0'))
			{
				mir_sntprintf(languages[i]->full_name, MAX_REGS(languages[i]->full_name), 
					_T("%s [%s]"), TranslateTS(languages[i]->localized_name), languages[i]->language);
			}
		}
	}

	return TRUE;
}


int SynthCallback(short*, int, espeak_EVENT*)
{
	return shutDown;
}


void LoadESpeak()
{
	char *tmp = mir_t2a(dictionariesFolder);

	if (espeak_Initialize(AUDIO_OUTPUT_SYNCH_PLAYBACK, 0, tmp, 0) == EE_INTERNAL_ERROR)
	{
		MessageBox(NULL, _T("Error initializing eSpeak engine"), _T("meSpeak"), MB_OK | MB_ICONERROR);
		mir_free(tmp);
		return;
	}

	espeak_SetSynthCallback(SynthCallback);

	mir_free(tmp);

	const espeak_VOICE **voices = espeak_ListVoices(NULL);
	const espeak_VOICE *voice;
	int i;
	for (i = 0; (voice = voices[i]) != NULL; i++)
	{
		const char *p = voice->languages;
		while(*p != '\0')
		{
			size_t len = strlen(p+1);

#ifdef UNICODE
			TCHAR *language = mir_utf8decodeW(p+1);
#else
			char language[NAME_SIZE];
			lstrcpyn(language, p+1, MAX_REGS(language));
			mir_utf8decode(language, NULL);
#endif
			TCHAR *tmp = language;
			while((tmp = _tcschr(tmp, _T('-'))) != NULL)
			{
				*tmp = _T('_');
				CharUpper(tmp);
			}


			Language *lang = GetLanguage(language, TRUE);
			lang->voices.insert(new Voice(voice->name, *p, voice->gender, voice->identifier));

#ifdef UNICODE
			mir_free(language);
#endif

			p += len+2;
		}
	}

	if (languages.getCount() <= 0)
		return;

	espeak_VOICE voice_select;
	voice_select.languages = "variant";
	voice_select.age = 0;
	voice_select.gender = 0;
	voice_select.name = NULL;

	voices = espeak_ListVoices(&voice_select);
	for (i = 0; (voice = voices[i]) != NULL; i++)
	{
		variants.insert(new Variant(voice->name, voice->gender, &voice->identifier[3]));
	}

	EnumSystemLocales(EnumLocalesProc, LCID_SUPPORTED);

	// Try to get name from DB
	for(i = 0; i < languages.getCount(); i++)
	{
		Language *lang = languages[i];
		if (lang->full_name[0] == _T('\0'))
		{
			DBVARIANT dbv;
#ifdef UNICODE
			char tmp[NAME_SIZE];
			WideCharToMultiByte(CP_ACP, 0, lang->language, -1, tmp, MAX_REGS(tmp), NULL, NULL);
			if (!DBGetContactSettingTString(NULL, MODULE_NAME, tmp, &dbv))
#else
			if (!DBGetContactSettingTString(NULL, MODULE_NAME, lang->language, &dbv))
#endif
			{
				lstrcpyn(lang->localized_name, dbv.ptszVal, MAX_REGS(lang->localized_name));
				DBFreeVariant(&dbv);
			}

			if (lang->localized_name[0] == _T('\0'))
			{
				for(size_t j = 0; j < MAX_REGS(aditionalLanguages); j+=2)
				{
					if (lstrcmp(aditionalLanguages[j], lang->language) == 0)
					{
						lstrcpyn(lang->localized_name, aditionalLanguages[j+1], MAX_REGS(lang->localized_name));
						break;
					}
				}
			}

			if (lang->localized_name[0] != _T('\0'))
			{
				mir_sntprintf(lang->full_name, MAX_REGS(lang->full_name), 
					_T("%s [%s]"), TranslateTS(lang->localized_name), lang->language);
			}
			else
			{
				lstrcpyn(lang->full_name, TranslateTS(lang->language), MAX_REGS(lang->full_name));
			}
		}
	}
/*
	for (i = 0; i < languages.getCount(); i++)
	{
		Language *lang = languages[i];
		OutputDebugString(lang->language);
		OutputDebugStringA(" (");
		OutputDebugString(lang->full_name);
		OutputDebugStringA(") ");
		OutputDebugStringA(":\n");
		for (int j = 0; j < lang->voices.getCount(); j++)
		{
			char tmp[128];
			Voice *voice = lang->voices[j];

			OutputDebugStringA("   - ");
			OutputDebugStringA(itoa(voice->prio, tmp, 10));
			OutputDebugStringA(" : ");
			OutputDebugStringA(voice->name);
			OutputDebugStringA(" - ");
			tmp[0] = voice->gender;
			tmp[1] = 0;
			OutputDebugStringA(tmp);
			OutputDebugStringA("\n");
		}
	}
*/
}


int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	shutDown = TRUE;

	delete queue;

	DeInitOptions();

	if (ServiceExists(MS_MSG_REMOVEICON))
	{
		StatusIconData sid = {0};
		sid.cbSize = sizeof(sid);
		sid.szModule = MODULE_NAME;
		CallService(MS_MSG_REMOVEICON, 0, (LPARAM) &sid);
	}

	for(unsigned i=0; i<MAX_REGS(hHooks); ++i)
		UnhookEvent(hHooks[i]);

	return 0;
}


void ToLocaleID(TCHAR *szKLName, size_t size)
{
	TCHAR *stopped = NULL;
	USHORT langID = (USHORT) _tcstol(szKLName, &stopped, 16);

	TCHAR ini[32];
	TCHAR end[32];
	GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SISO639LANGNAME, ini, MAX_REGS(ini));
	GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SISO3166CTRYNAME, end, MAX_REGS(end));

	mir_sntprintf(szKLName, size, _T("%s_%s"), ini, end);
}


Language * GetClosestLanguage(TCHAR *lang_name) 
{
	// Search the language by name
	for (int i = 0; i < languages.getCount(); i++)
	{
		if (lstrcmpi(languages[i]->language, lang_name) == 0)
		{
			return languages[i];
		}
	}

	// Try searching by the prefix only
	TCHAR *p = _tcschr(lang_name, _T('_'));
	if (p != NULL)
	{
		*p = _T('\0');
		for (int i = 0; i < languages.getCount(); i++)
		{
			if (lstrcmpi(languages[i]->language, lang_name) == 0)
			{
				*p = '_';
				return languages[i];
			}
		}
		*p = _T('_');
	}

	// Try any suffix, if one not provided
	if (p == NULL)
	{
		size_t len = lstrlen(lang_name);
		for (int i = 0; i < languages.getCount(); i++)
		{
			if (_tcsnicmp(languages[i]->language, lang_name, len) == 0 
				&& languages[i]->language[len] == _T('_'))
			{
				return languages[i];
			}
		}
	}

	return NULL;
}

void GetUserProtoLanguageSetting(TCHAR *lang_name, HANDLE hContact, char *proto, char *setting)
{
	DBVARIANT dbv = {0};

	if (!DBGetContactSettingTString(hContact, proto, setting, &dbv))
	{
		for (int i = 0; i < languages.getCount(); i++)
		{
			if (lstrcmpi(languages[i]->localized_name, dbv.ptszVal) == 0)
			{
				lstrcpyn(lang_name, languages[i]->language, NAME_SIZE);
				break;
			}
			if (lstrcmpi(languages[i]->english_name, dbv.ptszVal) == 0)
			{
				lstrcpyn(lang_name, languages[i]->language, NAME_SIZE);
				break;
			}
		}
		DBFreeVariant(&dbv);
	}
}

void GetUserLanguageSetting(TCHAR *lang_name, HANDLE hContact, char *setting)
{
	DBVARIANT dbv = {0};

	char *proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (proto == NULL)
		return;

	GetUserProtoLanguageSetting(lang_name, hContact, proto, setting);

	// If not found and is inside meta, try to get from the meta
	if (lang_name[0] != _T('\0'))
		return;
	
	// Is a subcontact?
	if (!ServiceExists(MS_MC_GETMETACONTACT)) 
		return;

	HANDLE hMetaContact = (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM) hContact, 0);
	if (hMetaContact == NULL)
		return;

	GetUserProtoLanguageSetting(lang_name, hMetaContact, metacontacts_proto, setting);
}


void GetLangPackLanguage(TCHAR *name, size_t len)
{
	LCID localeID = CallService(MS_LANGPACK_GETLOCALE, 0, 0);
	TCHAR ini[32];
	TCHAR end[32];
	GetLocaleInfo(localeID, LOCALE_SISO639LANGNAME, ini, MAX_REGS(ini));
	GetLocaleInfo(localeID, LOCALE_SISO3166CTRYNAME, end, MAX_REGS(end));

	mir_sntprintf(name, len, _T("%s_%s"), ini, end);
}


Language *GetContactLanguage(HANDLE hContact)
{
	if (hContact == NULL)
	{
		// System language

		// First try the db setting
		TCHAR lang_name[NAME_SIZE] = _T("");
		DBVARIANT dbv;
		if (!DBGetContactSettingTString(NULL, MODULE_NAME, "TalkLanguage", &dbv))
		{
			lstrcpyn(lang_name, dbv.ptszVal, MAX_REGS(lang_name));
			DBFreeVariant(&dbv);
		}

		// Then the langpack language
		if (lang_name[0] == _T('\0'))
			GetLangPackLanguage(lang_name, MAX_REGS(lang_name));

		Language *lang = GetLanguage(lang_name);
		if (lang != NULL)
			return lang;

		// Then english
		lang = GetLanguage(_T("en"));
		if (lang != NULL)
			return lang;

		// Last shot: first avaiable language
		return languages[0];
	}
	else
	{
		// Contact language
		TCHAR lang_name[NAME_SIZE] = _T("");

		DBVARIANT dbv = {0};
		if (!DBGetContactSettingTString(hContact, MODULE_NAME, "TalkLanguage", &dbv))
		{
			lstrcpyn(lang_name, dbv.ptszVal, NAME_SIZE);
			DBFreeVariant(&dbv);
		}

		if (lang_name[0] == _T('\0') && !DBGetContactSettingTString(hContact, "SpellChecker", "TalkLanguage", &dbv))
		{
			lstrcpyn(lang_name, dbv.ptszVal, NAME_SIZE);
			DBFreeVariant(&dbv);
		}

		// Try from metacontact
		if (lang_name[0] == _T('\0') && ServiceExists(MS_MC_GETMETACONTACT)) 
		{
			HANDLE hMetaContact = (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM) hContact, 0);
			if (hMetaContact != NULL)
			{
				if (!DBGetContactSettingTString(hMetaContact, MODULE_NAME, "TalkLanguage", &dbv))
				{
					lstrcpyn(lang_name, dbv.ptszVal, NAME_SIZE);
					DBFreeVariant(&dbv);
				}
								
				if (lang_name[0] == _T('\0') && !DBGetContactSettingTString(hMetaContact, "SpellChecker", "TalkLanguage", &dbv))
				{
					lstrcpyn(lang_name, dbv.ptszVal, NAME_SIZE);
					DBFreeVariant(&dbv);
				}
			}
		}

		// Try to get from Language info
		if (lang_name[0] == _T('\0'))
			GetUserLanguageSetting(lang_name, hContact, "Language");
		if (lang_name[0] == _T('\0'))
			GetUserLanguageSetting(lang_name, hContact, "Language1");
		if (lang_name[0] == _T('\0'))
			GetUserLanguageSetting(lang_name, hContact, "Language2");
		if (lang_name[0] == _T('\0'))
			GetUserLanguageSetting(lang_name, hContact, "Language3");

		if (lang_name[0] == _T('\0'))
			// Use default lang
			return opts.default_language;

		Language *ret = GetClosestLanguage(lang_name);
		if(ret == NULL)
			// Lost a lang?
			return opts.default_language;

		return ret;
	}
}


Voice *GetContactVoice(HANDLE hContact, Language *lang)
{
	int i;
	DBVARIANT dbv;
	if (DBGetContactSettingString(hContact, MODULE_NAME, "Voice", &dbv))
		goto DEFAULT;

	char name[NAME_SIZE];
	strncpy(name, dbv.pszVal, MAX_REGS(name));
	DBFreeVariant(&dbv);
	
	if (name[0] == '\0')
		goto DEFAULT;

	for (i = 0; i < lang->voices.getCount(); i++)
		if (stricmp(name, lang->voices[i]->name) == 0)
			return lang->voices[i];
	
DEFAULT:
	if (lang == opts.default_language)
		return opts.default_voice;
	else
		return lang->voices[0];
}


Variant *GetVariant(char *name)
{
	for (int i = 0; i < variants.getCount(); i++)
		if (stricmp(name, variants[i]->name) == 0)
			return variants[i];
	return NULL;
}


Variant *GetContactVariant(HANDLE hContact)
{
	Variant *ret;

	DBVARIANT dbv;
	if (DBGetContactSettingString(hContact, MODULE_NAME, "Variant", &dbv))
		goto DEFAULT;

	char name[NAME_SIZE];
	strncpy(name, dbv.pszVal, MAX_REGS(name));
	DBFreeVariant(&dbv);

	if (name[0] == '\0')
		goto DEFAULT;

	ret = GetVariant(name);
	if (ret != NULL)
		return ret;
	
DEFAULT:

	if (hContact != NULL)
	{
		CONTACTINFO ci = {0};
		ci.cbSize = sizeof(ci);
		ci.hContact = hContact;
		ci.dwFlag = CNF_GENDER;
		CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) &ci);

		if (ci.bVal == 'M')
		{
			ret = GetVariant("male1");
			if (ret != NULL)
				return ret;
		}
		else if (ci.bVal == 'F')
		{
			ret = GetVariant("female1");
			if (ret != NULL)
				return ret;
		}
	}	

	return opts.default_variant;
}


int GetContactParam(HANDLE hContact, int param)
{
	int ret = DBGetContactSettingDword(NULL, MODULE_NAME, PARAMETERS[param].setting, PARAMETERS[param].def);
	if (hContact != NULL)
		ret = DBGetContactSettingDword(hContact, MODULE_NAME, PARAMETERS[param].setting, ret);
	return ret;
}


void SetContactParam(HANDLE hContact, int param, int value)
{
	DBWriteContactSettingDword(hContact, MODULE_NAME, PARAMETERS[param].setting, value);
}


BOOL StatusEnabled(int status)
{
	switch(status) {
		case ID_STATUS_OFFLINE: return !opts.disable_offline;
		case ID_STATUS_ONLINE: return !opts.disable_online;
		case ID_STATUS_AWAY: return !opts.disable_away;
		case ID_STATUS_DND: return !opts.disable_dnd;
		case ID_STATUS_NA: return !opts.disable_na;
		case ID_STATUS_OCCUPIED: return !opts.disable_occupied;
		case ID_STATUS_FREECHAT: return !opts.disable_freechat;
		case ID_STATUS_INVISIBLE: return !opts.disable_invisible;
		case ID_STATUS_ONTHEPHONE: return !opts.disable_onthephone;
		case ID_STATUS_OUTTOLUNCH: return !opts.disable_outtolunch;
	}

	return !opts.disable_offline;
}


int SpeakService(HANDLE hContact, TCHAR *text)
{
	Language *lang;
	Voice *voice;
	SpeakData *data;
	int status, i;

	// Enabled?
	if (hContact != NULL && !DBGetContactSettingByte(hContact, MODULE_NAME, "Enabled", TRUE))
		goto RETURN;

	// Check status
	status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
	if (!StatusEnabled(status))
		goto RETURN;

	if (opts.enable_only_idle)
	{
		MIRANDA_IDLE_INFO idle = {0};
		CallService(MS_IDLE_GETIDLEINFO, 0, (LPARAM) &idle);
		if (idle.idleType == 0)
			goto RETURN;
	}

	// Check language
	lang = GetContactLanguage(hContact);
	if (lang == NULL)
		goto RETURN;

	voice = GetContactVoice(hContact, lang);
	if (voice == NULL)
		goto RETURN;

	data = new SpeakData(lang, voice, GetContactVariant(hContact), text);
	for (i = 0; i < NUM_PARAMETERS; i++)
		data->setParameter(i, GetContactParam(hContact, i));
	queue->Add(0, hContact, data);

	return 0;

RETURN:
	mir_free(text);
	return -1;
}


int SpeakAService(WPARAM wParam, LPARAM lParam)
{
	char *text = (char *) lParam;
	if (text == NULL)
		return -1;

	return SpeakService((HANDLE) wParam, mir_a2t(text));
}


int SpeakWService(WPARAM wParam, LPARAM lParam)
{
	WCHAR *text = (WCHAR *) lParam;
	if (text == NULL)
		return -1;

	return SpeakService((HANDLE) wParam, mir_u2t(text));
}


void Speak(HANDLE hContact, void *param)
{
	int status;

	SpeakData *data = (SpeakData *) param;
	if (data == NULL)
		return;

	if (languages.getCount() < 1)
		goto RETURN;

	if (hContact != (HANDLE) -1)
	{
		if (opts.respect_sndvol_mute && !DBGetContactSettingByte(NULL, "Skin", "UseSound", 1))
			goto RETURN;

		if (hContact != NULL && !DBGetContactSettingByte(hContact, MODULE_NAME, "Enabled", TRUE))
			goto RETURN;

		status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
		if (!StatusEnabled(status))
			goto RETURN;

		if (opts.enable_only_idle)
		{
			MIRANDA_IDLE_INFO idle = {0};
			CallService(MS_IDLE_GETIDLEINFO, 0, (LPARAM) &idle);
			if (idle.idleType == 0)
				goto RETURN;
		}
	}

	Speak(data);

RETURN:
	mir_free(data->text);
	delete data;
}


void Speak(SpeakData *data)
{
	if (data->variant != NULL)
	{
		char name[NAME_SIZE];
		mir_snprintf(name, MAX_REGS(name), "%s+%s", data->voice->id, data->variant->id);
		espeak_SetVoiceByName(name);
	}
	else
		espeak_SetVoiceByName(data->voice->id);

	for (int i = 0; i < NUM_PARAMETERS; i++)
		espeak_SetParameter(PARAMETERS[i].eparam, data->getParameter(i), 0);
	
	espeak_Synth(data->text, (lstrlen(data->text) + 1) * sizeof(TCHAR), 0, POS_CHARACTER, 
				 0, espeakCHARS_TCHAR, NULL, NULL);
}

TCHAR * VariablesSpeak(ARGUMENTSINFO *ai)
{
	if (ai->cbSize < sizeof(ARGUMENTSINFO))
		return NULL;

	ai->flags = AIF_FALSE;
	if (ai->argc < 2 || ai->argc > 3)
		return NULL;

	TCHAR *text = ai->targv[1];
	if (text == NULL)
		return NULL;
	
	HANDLE hContact = NULL;
	if (ai->argc >= 3)
	{
		CONTACTSINFO ci = {0};
		ci.cbSize = sizeof(ci);
		ci.tszContact = ai->targv[2];
		ci.flags = 0xFF | CI_UNICODE;
		int count = CallService(MS_VARS_GETCONTACTFROMSTRING, (WPARAM)&ci, 0);
		if (count == 1 && ci.hContacts != NULL) 
		{
			hContact = ci.hContacts[0];
		}
		else 
		{
			if (ci.hContacts != NULL) 
				CallService(MS_VARS_FREEMEMORY, (WPARAM) ci.hContacts, 0);
			
			return NULL;
		}
	}

	SpeakService(hContact, mir_tstrdup(text));
	ai->flags = 0;
	return mir_tstrdup(_T(""));
}
