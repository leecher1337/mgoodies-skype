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

#include "commons.h"


// Prototypes ///////////////////////////////////////////////////////////////////////////

static HANDLE hServices[2] = {0};

int SortTypes(const SPEAK_TYPE *type1, const SPEAK_TYPE *type2);
int RegisterService(WPARAM wParam, LPARAM lParam);
int SpeakExService(WPARAM wParam, LPARAM lParam);

LIST<SPEAK_TYPE> types(20, SortTypes);


// Functions ////////////////////////////////////////////////////////////////////////////


void InitTypes()
{
	hServices[0] = CreateServiceFunction(MS_SPEAK_REGISTER, RegisterService);
	hServices[1] = CreateServiceFunction(MS_SPEAK_SAYEX, SpeakExService);
}

void FreeTypes()
{
	// Destroy services
	int i;
	for(i = 0; i < MAX_REGS(hServices); ++i)
		DestroyServiceFunction(hServices[i]);

	// Free internal structs
	for(i = 0; i < types.getCount(); i++)
	{
		SPEAK_TYPE *type = types[i];
		mir_free((void *) type->module);
		mir_free((void *) type->name);
		mir_free((void *) type->icon);

		if (type->numTemplates > 0)
		{
			for(int i = 0; i < type->numTemplates; i++)
				mir_free((void *) type->templates[i]);
			mir_free(type->templates);
		}

		mir_free(type);
	}

	types.destroy();
}


int SortTypes(const SPEAK_TYPE *type1, const SPEAK_TYPE *type2)
{
	return stricmp(type1->description, type2->description);
}


int RegisterService(WPARAM wParam, LPARAM lParam)
{
	SPEAK_TYPE *orig = (SPEAK_TYPE *) wParam;
	if (orig == NULL || orig->cbSize < sizeof(SPEAK_TYPE) 
		|| orig->name == NULL || orig->description == NULL)
		return -1;

	SPEAK_TYPE *type = (SPEAK_TYPE *) mir_alloc0(sizeof(SPEAK_TYPE));
	type->cbSize = orig->cbSize;
	type->module = mir_strdup(orig->module);
	type->name = mir_strdup(orig->name);
	type->description = Translate(mir_strdup(orig->description));
	type->icon = mir_strdup(orig->icon);
	type->numTemplates = orig->numTemplates;

	if (orig->numTemplates > 0)
	{
		type->templates = (char **) mir_alloc0(orig->numTemplates * sizeof(char *));
		for(int i = 0; i < orig->numTemplates; i++)
			type->templates[i] = mir_strdup(orig->templates[i] == NULL ? "" : orig->templates[i]);
	}

	types.insert(type);

	return 0;
}

SPEAK_TYPE *GetType(const char *name)
{
	for(int i = 0; i < types.getCount(); i++)
	{
		SPEAK_TYPE *type = types[i];
		if (strcmp(type->name, name) == 0)
			return type;
	}

	return NULL;
}

void GetTemplate(Buffer<TCHAR> *buffer, SPEAK_TYPE *type, int templ)
{
	DBVARIANT dbv;

	char setting[128];
	mir_snprintf(setting, MAX_REGS(setting), "%s_%d_" TEMPLATE_TEXT, type->name, templ);

	if (!DBGetContactSettingTString(NULL, type->module == NULL ? MODULE_NAME : type->module, setting, &dbv))
	{
		buffer->append(dbv.ptszVal);
		DBFreeVariant(&dbv);
	}
	else
	{
		// Get default
		const char *tmp = type->templates[templ];
		tmp = strchr(tmp, '\n');
		if (tmp == NULL)
			return;
		tmp++;
		const char *end = strchr(tmp, '\n');
		size_t len = (end == NULL ? strlen(tmp) : end - tmp);

#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, tmp, len, buffer->appender(len), len);
#else
		buffer->append(tmp, len);
#endif
	}
}

static TCHAR** CopyVariablesToUnicode(SPEAK_ITEM *item)
{
	TCHAR **variables = NULL;
	if (item->numVariables > 0)
	{
		variables = (TCHAR **) mir_alloc0(item->numVariables * sizeof(TCHAR *));
		for(int i = 0; i < item->numVariables; i++)
		{
			if (item->flags & SPEAK_CHAR)
				variables[i] = mir_a2t(((char **) item->variables)[i]);
			else
				variables[i] = mir_u2t(((WCHAR **) item->variables)[i]);
		}
	}
	return variables;
}

static void FreeVariablesCopy(SPEAK_ITEM *item, TCHAR **variables)
{
	if (item->numVariables > 0 && variables != NULL)
	{
		for(int i = 0; i < item->numVariables; i++)
		{
			mir_free(variables[i]);
		}
		mir_free(variables);
	}
}

int SpeakExService(WPARAM wParam, LPARAM lParam)
{
	SPEAK_ITEM *item = (SPEAK_ITEM *) wParam;
	if (item == NULL || item->cbSize < sizeof(SPEAK_ITEM))
		return -1;

	SPEAK_TYPE *type = GetType(item->type);
	if (type == NULL)
		return -2;

	// Get the text to speak
	if (item->templateNum < 0)
	{
		if (item->text == NULL)
			return -3;

		if (!GetSettingBool(type, TEMPLATE_ENABLED, FALSE))
			return 1;

		Buffer<TCHAR> buff;
		if (GetSettingBool(type, SPEAK_NAME, TRUE) && item->hContact != NULL && item->hContact != (HANDLE) -1)
		{
			buff.append((TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) item->hContact, GCDNF_TCHAR));
			buff.append(_T(" : "));
		}

		if (item->flags & SPEAK_CHAR)
			buff.append((char *) item->text);
		else
			buff.append((WCHAR *) item->text);

		buff.pack();
		
		return SpeakService(item->hContact, buff.detach());
	}
	else
	{
		if (!GetSettingBool(type, item->templateNum, TEMPLATE_ENABLED, FALSE))
			return 1;

		Buffer<TCHAR> templ;
		GetTemplate(&templ, type, item->templateNum);
		templ.pack();

		Buffer<TCHAR> buffer;
		TCHAR **variables = CopyVariablesToUnicode(item);
		ReplaceTemplate(&buffer, item->hContact, templ.str, variables, item->numVariables);
		FreeVariablesCopy(item, variables);
		buffer.pack();

		if (buffer.str == NULL)
			return -3;

		return SpeakService(item->hContact, buffer.detach());
	}
}



