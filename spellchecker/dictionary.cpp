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

#include "dictionary.h"

#include "hunspell/hunspell.hxx"


#ifdef UNICODE
# define TSTRING wstring
# define _ttoupper towupper
#else
# define TSTRING string
# define _ttoupper toupper
#endif


#include "codepages.cpp"


DWORD WINAPI LoadThread(LPVOID hd);




#define LANGUAGE_NOT_LOADED		 1
#define LANGUAGE_LOADING		-1
#define LANGUAGE_LOADED			 0


class BaseDictionary : public Dictionary {
protected:
	TCHAR path[1024];
	TCHAR userPath[1024];
	map<TSTRING, TSTRING> autoReplaceMap;

	void loadCustomDict()
	{
		TCHAR filename[1024];
		mir_sntprintf(filename, MAX_REGS(filename), _T("%s\\%s.cdic"), userPath, language);

		FILE *file = _tfopen(filename, _T("r"));
		if (file != NULL) 
		{
			TCHAR word[1024];
			while(_fgetts(word, MAX_REGS(word), file)) 
			{
				size_t len = lstrlen(word);
				if (*(word + len - 1) == _T('\n')) 
					*(word + len - 1) = _T('\0');

				addWordInternal(word);
			}
			fclose(file);
		}
	}

	void appendToCustomDict(const TCHAR *word)
	{
		TCHAR filename[1024];
		mir_sntprintf(filename, MAX_REGS(filename), _T("%s\\%s.cdic"), userPath, language);

		FILE *file = _tfopen(filename, _T("a"));
		if (file != NULL) 
		{
			_ftprintf(file, _T("%s\n"), word);
			fclose(file);
		}
	}

	void loadAutoReplaceMap()
	{
		TCHAR filename[1024];
		mir_sntprintf(filename, MAX_REGS(filename), _T("%s\\%s.ar"), userPath, language);

		FILE *file = _tfopen(filename, _T("r"));
		if (file != NULL) 
		{
			TCHAR word[1024];
			while(_fgetts(word, MAX_REGS(word), file)) 
			{
				size_t len = lstrlen(word);
				if (*(word + len - 1) == _T('\n')) 
					*(word + len - 1) = _T('\0');

				// Get from
				TCHAR *p = _tcsstr(word, _T("->"));
				if (p != NULL)
				{
					*p = _T('\0');
					p += 2;

					autoReplaceMap[word] = p;
				}
			}
			fclose(file);
		}
	}

	void appendToAutoReplaceMap(const TCHAR * aFrom, const TCHAR * aTo)
	{
		TCHAR filename[1024];
		mir_sntprintf(filename, MAX_REGS(filename), _T("%s\\%s.ar"), userPath, language);

		TCHAR *from = CharLower(_tcsdup(aFrom));
		TCHAR *to = CharLower(_tcsdup(aTo));

		if (autoReplaceMap.find(from) == autoReplaceMap.end())
		{
			autoReplaceMap[from] = to;

			FILE *file = _tfopen(filename, _T("a"));
			if (file != NULL) 
			{
				_ftprintf(file, _T("%s->%s\n"), from, to);
				fclose(file);
			}
		}
		else
		{
			autoReplaceMap[from] = to;

			// A word changed, so we need to dump all map
			FILE *file = _tfopen(filename, _T("w"));
			if (file != NULL) 
			{
				map<TSTRING,TSTRING>::iterator it = autoReplaceMap.begin();
				while(it != autoReplaceMap.end())
				{
					_ftprintf(file, _T("%s->%s\n"), (it->first).c_str(), (it->second).c_str());
					it++;
				}
				fclose(file);
			}
		}

		free(from);
		free(to);
	}

	virtual void addWordInternal(const TCHAR * word) =0;

public:
	virtual ~BaseDictionary() {}

	// Add a word to the user custom dict
	virtual void addWord(const TCHAR * word)
	{
		addWordInternal(word);
		appendToCustomDict(word);
	}
	
	// Add a word to the list of ignored words
	virtual void ignoreWord(const TCHAR * word)
	{
		addWordInternal(word);
	}

	// Add a word to the list of auto-replaced words
	virtual void addToAutoReplace(const TCHAR * from, const TCHAR * to)
	{
		appendToAutoReplaceMap(from, to);
	}

	// Return a a auto replace to a word
	// You have to free the item
	virtual TCHAR * autoReplace(const TCHAR * word)
	{
		TCHAR *from = _tcslwr(_tcsdup(word));

		if (autoReplaceMap.find(from) == autoReplaceMap.end())
		{
			free(from);
			return NULL;
		}
		else
		{
			TCHAR *to = _tcsdup(autoReplaceMap[from].c_str());

			// Wich case to use?
			size_t len = lstrlen(word);
			size_t i;
			for (i = 0; i < len; i++)
				if (!IsCharUpper(word[i]))
					break;

			if (i <= 0)
			{
				// All lower
				return to;
			}
			else if (i >= len)
			{
				// All upper
				return CharUpper(to);
			}
			else
			{
				// First upper
				TCHAR tmp[2];
				tmp[0] = to[0];
				tmp[1] = _T('\0');
				CharUpper(tmp);
				to[0] = tmp[0];
				return to;
			}
		}
	}
};

class HunspellDictionary : public BaseDictionary {
protected:
	int loaded;
	Hunspell *hunspell;
	TCHAR *wordChars;
	UINT codePage;

	virtual void addWordInternal(const TCHAR * word)
	{
		if (loaded != LANGUAGE_LOADED)
			return;

		char hunspell_word[1024];
		toHunspell(hunspell_word, word, MAX_REGS(hunspell_word));

		hunspell->put_word(hunspell_word);
	}

	void toHunspell(char *hunspellWord, const TCHAR *word, size_t hunspellWordLen)
	{
#ifdef UNICODE
		WideCharToMultiByte(codePage, 0, word, -1, hunspellWord, hunspellWordLen, NULL, NULL);
#else
		// TODO
		strncpy(hunspellWord, word, hunspellWordLen);
#endif
	}

	TCHAR * fromHunspell(const char *hunspellWord)
	{
#ifdef UNICODE
		int len = MultiByteToWideChar(codePage, 0, hunspellWord, -1, NULL, 0);
		WCHAR *ret = (WCHAR *) malloc((len + 1) * sizeof(WCHAR));
		MultiByteToWideChar(codePage, 0, hunspellWord, -1, ret, len + 1);
		return ret;
#else
		// TODO
		return strdup(hunspellWord);
#endif
	}

public:
	HunspellDictionary(TCHAR *aLanguage, TCHAR *aPath, TCHAR *aUserPath, TCHAR *aFlagsPath)
	{
		lstrcpyn(language, aLanguage, MAX_REGS(language));
		lstrcpyn(full_name, aLanguage, MAX_REGS(full_name));
		lstrcpyn(path, aPath, MAX_REGS(path));
		lstrcpyn(userPath, aUserPath, MAX_REGS(userPath));

		loaded = LANGUAGE_NOT_LOADED;
		localized_name[0] = _T('\0');
		english_name[0] = _T('\0');
		hunspell = NULL;
		wordChars = NULL;
		codePage = CP_ACP;
		hFlag = NULL;
	}

	virtual ~HunspellDictionary()
	{
		if (hunspell != NULL)
			delete hunspell;
		if (wordChars != NULL)
			free(wordChars);
	}

	void loadThread()
	{
		char dic[1024];
		char aff[1024];

#ifdef UNICODE
		mir_snprintf(dic, MAX_REGS(dic), "%S\\%S.dic", path, language);
		mir_snprintf(aff, MAX_REGS(aff), "%S\\%S.aff", path, language);
#else
		mir_snprintf(dic, MAX_REGS(dic), "%s\\%s.dic", path, language);
		mir_snprintf(aff, MAX_REGS(aff), "%s\\%s.aff", path, language);
#endif

		hunspell = new Hunspell(aff, dic);

		// Get codepage
		if (hunspell->get_utf8())
		{
			codePage = CP_UTF8;

#ifdef UNICODE
			int len;
			wordChars = _tcsdup(hunspell->get_wordchars_utf16(&len));
#else
			// No option
			wordChars = _tcsdup("qwertzuiopasdfghjklyxcvbnmQWERTZUIOPASDFGHJKLYXCVBNM");
#endif
		}
		else
		{
			for (int i = 0; i < MAX_REGS(codepages); i++)
			{
				if (strcmpi(codepages[i].name, hunspell->get_dic_encoding()) == 0)
				{
					if (IsValidCodePage(codepages[i].codepage))
						codePage = codepages[i].codepage;
					break;
				}
			}
			wordChars = fromHunspell(hunspell->get_wordchars());
		}

		loaded = LANGUAGE_LOADED;

		loadCustomDict();
		loadAutoReplaceMap();
	}

	// Return TRUE if the word is correct
	virtual BOOL spell(const TCHAR *word)
	{
		load();
		if (loaded != LANGUAGE_LOADED)
			return TRUE;

		char hunspell_word[1024];
		toHunspell(hunspell_word, word, MAX_REGS(hunspell_word));

		return hunspell->spell(hunspell_word);
	}

	// Return a list of suggestions to a word
	virtual Suggestions suggest(const TCHAR * word)
	{
		Suggestions ret = {0};

		load();
		if (loaded != LANGUAGE_LOADED)
			return ret;

		char hunspell_word[1024];
		toHunspell(hunspell_word, word, MAX_REGS(hunspell_word));

		char ** words;
		int count = hunspell->suggest(&words, hunspell_word);

		if (count <= 0)
			return ret;

		// Oki, lets make our array
		ret.count = count;
		ret.words = (TCHAR **) malloc(ret.count * sizeof(TCHAR *));
		for (int i = 0; i < count; i++)
		{
			ret.words[i] = fromHunspell(words[i]);
			free(words[i]);
		}
		free(words);

		return ret;
	}

	// Return a list of auto suggestions to a word
	virtual Suggestions autoSuggest(const TCHAR * word)
	{
		Suggestions ret = {0};

		load();
		if (loaded != LANGUAGE_LOADED)
			return ret;

		char hunspell_word[1024];
		toHunspell(hunspell_word, word, MAX_REGS(hunspell_word));

		char ** words;
		int count = hunspell->suggest_auto(&words, hunspell_word);

		if (count <= 0)
			return ret;

		// Oki, lets make our array
		ret.count = count;
		ret.words = (TCHAR **) malloc(ret.count * sizeof(TCHAR *));
		for (int i = 0; i < count; i++)
		{
			ret.words[i] = fromHunspell(words[i]);
			free(words[i]);
		}
		free(words);

		return ret;
	}

	// Return a list of auto suggestions to a word
	// You have to free the list AND each item
	virtual TCHAR * autoSuggestOne(const TCHAR * word)
	{
		load();
		if (loaded != LANGUAGE_LOADED)
			return NULL;

		char hunspell_word[1024];
		toHunspell(hunspell_word, word, MAX_REGS(hunspell_word));

		char ** words;
		int count = hunspell->suggest_auto(&words, hunspell_word);

		if (count <= 0)
			return NULL;

		TCHAR *ret = fromHunspell(words[0]);

		// Oki, lets make our array
		for (int i = 0; i < count; i++)
			free(words[i]);
		free(words);

		return ret;
	}

	// Return TRUE if the char is a word char
	virtual BOOL isWordChar(TCHAR c)
	{
		load();
		if (loaded != LANGUAGE_LOADED)
			return TRUE;

		if (c == _T('-'))
			return !hunspell->get_forbidden_compound();
		else
			return _tcschr(wordChars, (_TINT) c) != NULL;
	}

	// Assert that all needed data is loaded
	virtual void load()	
	{
		if (loaded == LANGUAGE_NOT_LOADED)
		{
			loaded = LANGUAGE_LOADING;

			DWORD thread_id;
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) LoadThread, 
				(LPVOID) this, 0, &thread_id);
		}
	}

	virtual BOOL isLoaded()
	{
		return loaded == LANGUAGE_LOADED;
	}
};


DWORD WINAPI LoadThread(LPVOID hd)
{
	HunspellDictionary *dict = (HunspellDictionary *) hd;
	dict->loadThread();
	return 0;
}



// To use with EnumLocalesProc :(
Dictionaries *tmp_dicts;

// To get the names of the languages
BOOL CALLBACK EnumLocalesProc(LPTSTR lpLocaleString)
{
	TCHAR *stopped = NULL;
	USHORT langID = (USHORT) _tcstol(lpLocaleString, &stopped, 16);

	TCHAR ini[10];
	TCHAR end[10];
	GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SISO639LANGNAME, ini, MAX_REGS(ini));
	GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SISO3166CTRYNAME, end, MAX_REGS(end));

	TCHAR name[10];
	mir_sntprintf(name, MAX_REGS(name), _T("%s_%s"), ini, end);

	for(int i = 0; i < tmp_dicts->count; i++)
	{
		if (lstrcmpi(tmp_dicts->dicts[i]->language, name) == 0)
		{
			GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SLANGUAGE, 
				tmp_dicts->dicts[i]->localized_name, MAX_REGS(tmp_dicts->dicts[i]->localized_name));
			GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SENGLANGUAGE, 
				tmp_dicts->dicts[i]->english_name, MAX_REGS(tmp_dicts->dicts[i]->english_name));

			if (tmp_dicts->dicts[i]->localized_name[0] != _T('\0'))
			{
				mir_sntprintf(tmp_dicts->dicts[i]->full_name, MAX_REGS(tmp_dicts->dicts[i]->full_name), 
					_T("%s [%s]"), tmp_dicts->dicts[i]->localized_name, tmp_dicts->dicts[i]->language);
			}
			break;
		}
	}
	return TRUE;
}


// Return a list of avaible languages
Dictionaries GetAvaibleDictionaries(TCHAR *path, TCHAR *user_path, TCHAR *flags_path)
{
	Dictionaries dicts = {0};

	// Load the language files and create an array with then
	TCHAR file[1024];
	mir_sntprintf(file, MAX_REGS(file), _T("%s\\*.dic"), path);

	// Lets count the files
	WIN32_FIND_DATA ffd = {0};
	HANDLE hFFD = FindFirstFile(file, &ffd);
	if (hFFD != INVALID_HANDLE_VALUE)
	{
		do
		{
			TCHAR tmp[1024];
			mir_sntprintf(tmp, MAX_REGS(tmp), _T("%s\\%s"), path, ffd.cFileName);

			// Check .dic
			DWORD attrib = GetFileAttributes(tmp);
			if (attrib == 0xFFFFFFFF || (attrib & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			// See if .aff exists too
			lstrcpy(&tmp[lstrlen(tmp) - 4], _T(".aff"));
			attrib = GetFileAttributes(tmp);
			if (attrib == 0xFFFFFFFF || (attrib & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			dicts.count++;
		}
		while(FindNextFile(hFFD, &ffd));

		FindClose(hFFD);
	}

	dicts.has_flags = FALSE;
	if (dicts.count > 0)
	{
		// Oki, lets make our cache struct
		dicts.dicts = (Dictionary **) malloc(dicts.count * sizeof(Dictionary *));
		ZeroMemory(dicts.dicts, dicts.count * sizeof(Dictionary *));

		size_t i = 0;
		hFFD = FindFirstFile(file, &ffd);
		if (hFFD != INVALID_HANDLE_VALUE)
		{
			do
			{
				TCHAR tmp[1024];
				mir_sntprintf(tmp, MAX_REGS(tmp), _T("%s\\%s"), path, ffd.cFileName);

				// Check .dic
				DWORD attrib = GetFileAttributes(tmp);
				if (attrib == 0xFFFFFFFF || (attrib & FILE_ATTRIBUTE_DIRECTORY))
					continue;

				// See if .aff exists too
				lstrcpy(&tmp[lstrlen(tmp) - 4], _T(".aff"));
				attrib = GetFileAttributes(tmp);
				if (attrib == 0xFFFFFFFF || (attrib & FILE_ATTRIBUTE_DIRECTORY))
					continue;

				ffd.cFileName[lstrlen(ffd.cFileName)-4] = _T('\0');

				dicts.dicts[i] = new HunspellDictionary(ffd.cFileName, path, user_path, flags_path);
				if (dicts.dicts[i]->hFlag != NULL)
					dicts.has_flags = TRUE;

				i++;
			}
			while(i < dicts.count && FindNextFile(hFFD, &ffd));

			dicts.count = i;

			FindClose(hFFD);
		}
		
		tmp_dicts = &dicts;
		EnumSystemLocales(EnumLocalesProc, LCID_SUPPORTED);
	}

	return dicts;
}


// Free the list returned by GetAvaibleDictionaries
void FreeDictionaries(Dictionaries &dicts)
{
	for (size_t i = 0; i < dicts.count; i++)
	{
		delete dicts.dicts[i];
	}
	free(dicts.dicts);

	dicts.dicts = NULL;
	dicts.count = 0;
}


// Free the list returned by GetAvaibleDictionaries
void FreeSuggestions(Suggestions &suggestions)
{
	for (size_t i = 0; i < suggestions.count; i++)
	{
		free(suggestions.words[i]);
	}
	free(suggestions.words);

	suggestions.words = NULL;
	suggestions.count = 0;
}