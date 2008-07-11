#include "Compatibility.h"
#define DB_NOHELPERFUNCTIONS
	#include "m_database.h"
#undef DB_NOHELPERFUNCTIONS

#include <map>

HANDLE gCompServices[31] = {0};
HANDLE gEvents[6] = {0};

HANDLE hEventDeletedEvent, 
       hEventAddedEvent,
			 hEventFilterAddedEvent,
			 hSettingChangeEvent, 
			 hContactDeletedEvent, 
			 hContactAddedEvent;

int CompAddContact(WPARAM wParam, LPARAM lParam)
{
	int res = DBContactCreate(DBContactGetRoot(0, 0), 0);
	if (res == DBT_INVALIDPARAM)
		return 1;

	NotifyEventHooks(hContactAddedEvent, (WPARAM)res, 0);
	return res;
}
int CompDeleteContact(WPARAM hContact, LPARAM lParam)
{
	NotifyEventHooks(hContactDeletedEvent, hContact, 0);

	int res = DBContactDelete(hContact, 0);
	if (res == DBT_INVALIDPARAM)
		return 1;
	
	return res;
}
int CompIsDbContact(WPARAM hContact, LPARAM lParam)
{
	int flags = DBContactGetFlags(hContact, 0);
	return (flags != DBT_INVALIDPARAM) && ((flags & (DBT_CF_IsGroup | DBT_CF_IsVirtual)) == 0);
}
int CompGetContactCount(WPARAM wParam, LPARAM lParam)
{
	TDBTContactIterFilter f = {0};
	f.cbSize = sizeof(f);
	f.fDontHasFlags = DBT_CF_IsGroup | DBT_CF_IsVirtual;

	TDBTContactIterationHandle hiter = DBContactIterInit((WPARAM)&f, 0);
	int c = 0;
	if ((hiter != 0) && (hiter != DBT_INVALIDPARAM))
	{
		TDBTContactHandle con = DBContactIterNext(hiter, 0);

		while ((con != DBT_INVALIDPARAM) && (con != 0))
		{
			if ((con != 0) && (con != DBT_INVALIDPARAM))
				c++;

			con = DBContactIterNext(hiter, 0);
		}
		DBContactIterClose(hiter, 0);
	}
	return c;
}
int CompFindFirstContact(WPARAM wParam, LPARAM lParam)
{
	return gDataBase->getContacts().compFirstContact();
}
int CompFindNextContact(WPARAM hContact, LPARAM lParam)
{
	return gDataBase->getContacts().compNextContact(hContact);
}

int CompGetContactSetting(WPARAM hContact, LPARAM pSetting)
{
	DBCONTACTGETSETTING * dbcgs = (DBCONTACTGETSETTING *) pSetting;

	char namebuf[512];
	namebuf[0] = 0;
	if (dbcgs->szModule)
		strcpy_s(namebuf, dbcgs->szModule);
	strcat_s(namebuf, "/");
	strcat_s(namebuf, dbcgs->szSetting);
	
	TDBTSettingDescriptor desc = {0};
	TDBTSetting set = {0};
	desc.cbSize = sizeof(desc);
	desc.Contact = hContact;
	desc.pszSettingName = namebuf;

	set.cbSize = sizeof(set);
	set.Descriptor = &desc;
	
	if (DBSettingRead((WPARAM)&set, 0) == DBT_INVALIDPARAM)
		return -1;

	switch (set.Type)
	{
		case DBT_ST_ANSI: 
		{			
			dbcgs->pValue->type = DBVT_ASCIIZ;
			dbcgs->pValue->pszVal = set.Value.pAnsi;
			dbcgs->pValue->cchVal = set.Value.Length - 1;
		} break;
		case DBT_ST_UTF8:
		{
			dbcgs->pValue->type = DBVT_ASCIIZ; //DBVT_UTF8;
			dbcgs->pValue->pszVal = mir_utf8decode(set.Value.pUTF8, NULL);
			dbcgs->pValue->cchVal = set.Value.Length - 1;
		} break;
		case DBT_ST_WCHAR:
		{	
			dbcgs->pValue->type = DBVT_ASCIIZ;
			dbcgs->pValue->pszVal = mir_u2a(set.Value.pWide);
			dbcgs->pValue->cchVal = strlen(dbcgs->pValue->pszVal);
			mir_free(set.Value.pWide);
		} break;
		case DBT_ST_BLOB:
		{
			dbcgs->pValue->type = DBVT_BLOB;
			dbcgs->pValue->pbVal = set.Value.pBlob;
			dbcgs->pValue->cpbVal = set.Value.Length;
		} break;
		case DBT_ST_BOOL:
		{
			dbcgs->pValue->type = DBVT_BYTE;
			dbcgs->pValue->bVal = (uint8_t)set.Value.Bool;
		} break;
		case DBT_ST_BYTE: case DBT_ST_CHAR:
		{
			dbcgs->pValue->type = DBVT_BYTE;
			dbcgs->pValue->bVal = set.Value.Byte;
		} break;
		case DBT_ST_SHORT: case DBT_ST_WORD:
		{
			dbcgs->pValue->type = DBVT_WORD;
			dbcgs->pValue->wVal = set.Value.Word;
		} break;
		case DBT_ST_INT: case DBT_ST_DWORD:
		{
			dbcgs->pValue->type = DBVT_DWORD;
			dbcgs->pValue->dVal = set.Value.DWord;
		} break;
		case DBT_ST_INT64: case DBT_ST_QWORD:
		case DBT_ST_DOUBLE: case DBT_ST_FLOAT:
		{
			dbcgs->pValue->type = DBVT_BLOB;
			dbcgs->pValue->cpbVal = sizeof(set.Value);
			dbcgs->pValue->pbVal = (BYTE*)mir_alloc(sizeof(set.Value));
			memcpy(dbcgs->pValue->pbVal, &set.Value, sizeof(set.Value));
		} break;
		default:
		{
			return -1;
		}
	}

	return 0;
}
int CompGetContactSettingStr(WPARAM hContact, LPARAM pSetting)
{
	DBCONTACTGETSETTING * dbcgs = (DBCONTACTGETSETTING *) pSetting;

	if ((dbcgs->pValue->type & DBVTF_VARIABLELENGTH) == 0)
	{
		CompFreeVariant(0, (LPARAM)dbcgs->pValue);
		dbcgs->pValue->type = 0;
	}

	char namebuf[512];
	namebuf[0] = 0;
	if (dbcgs->szModule)
		strcpy_s(namebuf, dbcgs->szModule);
	strcat_s(namebuf, "/");
	strcat_s(namebuf, dbcgs->szSetting);
	
	TDBTSettingDescriptor desc = {0};
	TDBTSetting set = {0};
	desc.cbSize = sizeof(desc);
	desc.Contact = hContact;
	desc.pszSettingName = namebuf;

	set.cbSize = sizeof(set);
	set.Descriptor = &desc;

	
	switch (dbcgs->pValue->type)
	{
		case DBVT_ASCIIZ: set.Type = DBT_ST_ANSI; break;
		case DBVT_BLOB:   set.Type = DBT_ST_BLOB; break;
		case DBVT_UTF8:   set.Type = DBT_ST_UTF8; break;
		case DBVT_WCHAR:  set.Type = DBT_ST_WCHAR; break;
	}
	
	if (DBSettingRead((WPARAM)&set, 0) == DBT_INVALIDPARAM)
		return -1;

	switch (set.Type)
	{
		case DBT_ST_ANSI: 
		{	
			dbcgs->pValue->type = DBVT_ASCIIZ;
			dbcgs->pValue->pszVal = set.Value.pAnsi;
			dbcgs->pValue->cchVal = set.Value.Length - 1;
		} break;
		case DBT_ST_UTF8:
		{	
			dbcgs->pValue->type = DBVT_UTF8;
			dbcgs->pValue->pszVal = set.Value.pUTF8;
			dbcgs->pValue->cchVal = set.Value.Length - 1;
		} break;
		case DBT_ST_WCHAR:
		{	
			if (dbcgs->pValue->type == DBVT_WCHAR)
			{
				dbcgs->pValue->pwszVal = set.Value.pWide;
				dbcgs->pValue->cchVal = set.Value.Length - 1;
			} else {
				dbcgs->pValue->type = DBVT_UTF8;
				dbcgs->pValue->pszVal = mir_utf8encodeW(set.Value.pWide);
				dbcgs->pValue->cchVal = strlen(dbcgs->pValue->pszVal);
				mir_free(set.Value.pWide);
			}
		} break;
		case DBT_ST_BLOB:
		{
			dbcgs->pValue->type = DBVT_BLOB;
			dbcgs->pValue->pbVal = set.Value.pBlob;
			dbcgs->pValue->cpbVal = set.Value.Length;
		} break;
		case DBT_ST_BOOL:
		{
			dbcgs->pValue->type = DBVT_BYTE;
			dbcgs->pValue->bVal = (uint8_t)set.Value.Bool;
		} break;
		case DBT_ST_BYTE: case DBT_ST_CHAR:
		{
			dbcgs->pValue->type = DBVT_BYTE;
			dbcgs->pValue->bVal = set.Value.Byte;
		} break;
		case DBT_ST_SHORT: case DBT_ST_WORD:
		{
			dbcgs->pValue->type = DBVT_WORD;
			dbcgs->pValue->wVal = set.Value.Word;
		} break;
		case DBT_ST_INT: case DBT_ST_DWORD:
		{
			dbcgs->pValue->type = DBVT_DWORD;
			dbcgs->pValue->dVal = set.Value.DWord;
		} break;
		case DBT_ST_INT64: case DBT_ST_QWORD:
		case DBT_ST_DOUBLE: case DBT_ST_FLOAT:
		{
			dbcgs->pValue->type = DBVT_BLOB;
			dbcgs->pValue->cpbVal = sizeof(set.Value);
			dbcgs->pValue->pbVal = (BYTE*)mir_alloc(sizeof(set.Value));
			memcpy(dbcgs->pValue->pbVal, &set.Value, sizeof(set.Value));
		} break;
		default:
		{
			return -1;
		}
	}

	return 0;
}
int CompGetContactSettingStatic(WPARAM hContact, LPARAM pSetting)
{
	DBCONTACTGETSETTING * dbcgs = (DBCONTACTGETSETTING *) pSetting;

	char namebuf[512];
	namebuf[0] = 0;
	if (dbcgs->szModule)
		strcpy_s(namebuf, dbcgs->szModule);
	strcat_s(namebuf, "/");
	strcat_s(namebuf, dbcgs->szSetting);
	
	TDBTSettingDescriptor desc = {0};
	TDBTSetting set = {0};
	desc.cbSize = sizeof(desc);
	desc.Contact = hContact;
	desc.pszSettingName = namebuf;

	set.cbSize = sizeof(set);
	set.Descriptor = &desc;
	
	if (DBSettingRead((WPARAM)&set, 0) == DBT_INVALIDPARAM)
		return -1;

	if ((set.Type & DBT_STF_VariableLength) ^ (dbcgs->pValue->type & DBVTF_VARIABLELENGTH))
	{
		if (set.Type & DBT_STF_VariableLength)
			mir_free(set.Value.pBlob);
		return -1;
	}

	if ((set.Type & DBT_STF_VariableLength) && (dbcgs->pValue->type & DBVTF_VARIABLELENGTH))
	{
		if (!( ((set.Type == DBT_ST_ANSI)   && (dbcgs->pValue->type == DBVT_ASCIIZ)) ||
			     ((set.Type == DBT_ST_UTF8)   && (dbcgs->pValue->type == DBVT_UTF8  )) ||
					 ((set.Type == DBT_ST_WCHAR)  && (dbcgs->pValue->type == DBVT_WCHAR )) ||
					 ((set.Type == DBT_ST_BLOB)   && (dbcgs->pValue->type == DBVT_BLOB  )) ))
		{
			mir_free(set.Value.pBlob);
			return -1;
		}
	}

	switch (set.Type)
	{
		case DBT_ST_ANSI: 
		{			
			if (dbcgs->pValue->cchVal < set.Value.Length)
			{
				memcpy(dbcgs->pValue->pszVal, set.Value.pAnsi, dbcgs->pValue->cchVal);
				dbcgs->pValue->pszVal[dbcgs->pValue->cchVal - 1] = 0;
			} else {
				memcpy(dbcgs->pValue->pszVal, set.Value.pAnsi, set.Value.Length);
			}
			dbcgs->pValue->type = DBVT_ASCIIZ;
			dbcgs->pValue->cchVal = set.Value.Length - 1;

			mir_free(set.Value.pAnsi);
		} break;
		case DBT_ST_UTF8:
		{			
			set.Value.pUTF8 = mir_utf8decode(set.Value.pUTF8, NULL);
			set.Value.Length = strlen(set.Value.pUTF8);

			if (dbcgs->pValue->cchVal < set.Value.Length)
			{
				memcpy(dbcgs->pValue->pszVal, set.Value.pUTF8, dbcgs->pValue->cchVal);
				dbcgs->pValue->pszVal[dbcgs->pValue->cchVal - 1] = 0;
			} else {
				memcpy(dbcgs->pValue->pszVal, set.Value.pUTF8, set.Value.Length);
			}
			dbcgs->pValue->type = DBVT_ASCIIZ;
			dbcgs->pValue->cchVal = set.Value.Length - 1;

			mir_free(set.Value.pUTF8);
		} break;
		case DBT_ST_WCHAR:
		{
			char * tmp = mir_u2a(set.Value.pWide);
			unsigned int l = strlen(tmp);
			mir_free(set.Value.pWide);

			if (dbcgs->pValue->cchVal < l + 1)
			{
				memcpy(dbcgs->pValue->pszVal, tmp, dbcgs->pValue->cchVal);
				dbcgs->pValue->pszVal[l] = 0;
			} else {
				memcpy(dbcgs->pValue->pszVal, tmp, l + 1);
			}
			dbcgs->pValue->type = DBVT_ASCIIZ;
			dbcgs->pValue->cchVal = l;

			mir_free(tmp);
		} break;
		case DBT_ST_BLOB:
		{			
			if (dbcgs->pValue->cchVal < set.Value.Length)
			{
				memcpy(dbcgs->pValue->pbVal, set.Value.pBlob, dbcgs->pValue->cchVal);
			} else {
				memcpy(dbcgs->pValue->pbVal, set.Value.pBlob, set.Value.Length);
			}
			dbcgs->pValue->type = DBVT_BLOB;
			dbcgs->pValue->cchVal = set.Value.Length;

			mir_free(set.Value.pBlob);
		} break;
		case DBT_ST_BOOL:
		{
			dbcgs->pValue->type = DBVT_BYTE;
			dbcgs->pValue->bVal = (uint8_t)set.Value.Bool;
		} break;
		case DBT_ST_BYTE: case DBT_ST_CHAR:
		{
			dbcgs->pValue->type = DBVT_BYTE;
			dbcgs->pValue->bVal = set.Value.Byte;
		} break;
		case DBT_ST_SHORT: case DBT_ST_WORD:
		{
			dbcgs->pValue->type = DBVT_WORD;
			dbcgs->pValue->wVal = set.Value.Word;
		} break;
		case DBT_ST_INT: case DBT_ST_DWORD:
		{
			dbcgs->pValue->type = DBVT_DWORD;
			dbcgs->pValue->dVal = set.Value.DWord;
		} break;
		default:
		{
			return -1;
		}
	}

	return 0;
}
int CompFreeVariant(WPARAM wParam, LPARAM pSetting)
{
	DBVARIANT * dbv = (DBVARIANT *) pSetting;
	
	if ((dbv->type == DBVT_BLOB) && (dbv->pbVal))
	{
		mir_free(dbv->pbVal);
		dbv->pbVal = 0;
	} else if ((dbv->type & DBVTF_VARIABLELENGTH) && (dbv->pszVal))
	{
		mir_free(dbv->pszVal);
		dbv->pszVal = NULL;
	}
	dbv->type = 0;
	return 0;
}
int CompWriteContactSetting(WPARAM hContact, LPARAM pSetting)
{
	DBCONTACTWRITESETTING * dbcws = (DBCONTACTWRITESETTING *)pSetting;

	char namebuf[512];
	namebuf[0] = 0;
	if (dbcws->szModule)
		strcpy_s(namebuf, dbcws->szModule);
	strcat_s(namebuf, "/");
	strcat_s(namebuf, dbcws->szSetting);
	
	TDBTSettingDescriptor desc = {0};
	TDBTSetting set = {0};
	desc.cbSize = sizeof(desc);
	desc.Contact = hContact;
	desc.pszSettingName = namebuf;

	set.cbSize = sizeof(set);
	set.Descriptor = &desc;

	switch (dbcws->value.type)
	{
		case DBVT_ASCIIZ:
		{
			set.Type = DBT_ST_ANSI;
			set.Value.pAnsi = dbcws->value.pszVal;
		} break;
		case DBVT_UTF8:
		{
			set.Type = DBT_ST_UTF8;
			set.Value.pUTF8 = dbcws->value.pszVal;
		} break;
		case DBVT_WCHAR:
		{
			set.Type = DBT_ST_WCHAR;
			set.Value.pWide = dbcws->value.pwszVal;
		} break;
		case DBVT_BLOB:
		{
			set.Type = DBT_ST_BLOB;
			set.Value.pBlob = dbcws->value.pbVal;
		} break;
		case DBVT_BYTE:
		{
			set.Type = DBT_ST_BYTE;
			set.Value.Byte = dbcws->value.bVal;
		} break;
		case DBVT_WORD:
		{
			set.Type = DBT_ST_WORD;
			set.Value.Word = dbcws->value.wVal;
		} break;
		case DBVT_DWORD:
		{
			set.Type = DBT_ST_DWORD;
			set.Value.DWord = dbcws->value.dVal;
		} break;
		default:
		{
			return -1;
		}
	}

	if (DBSettingWrite((WPARAM)&set, 0) == DBT_INVALIDPARAM)
		return -1;

	NotifyEventHooks(ME_DB_CONTACT_SETTINGCHANGED, hContact, pSetting);

	return 0;
}
int CompDeleteContactSetting(WPARAM hContact, LPARAM pSetting)
{
	DBCONTACTGETSETTING * dbcgs = (DBCONTACTGETSETTING *) pSetting;

	char namebuf[512];
	namebuf[0] = 0;
	if (dbcgs->szModule)
		strcpy_s(namebuf, dbcgs->szModule);
	strcat_s(namebuf, "/");
	strcat_s(namebuf, dbcgs->szSetting);
	
	TDBTSettingDescriptor desc = {0};
	desc.cbSize = sizeof(desc);
	desc.Contact = hContact;
	desc.pszSettingName = namebuf;

	if (DBSettingDelete((WPARAM)&desc, 0) == DBT_INVALIDPARAM)
		return -1;

	{
		DBCONTACTWRITESETTING tmp = {0};
		tmp.szModule = dbcgs->szModule;
		tmp.szSetting = dbcgs->szSetting;
		tmp.value.type = 0;
		NotifyEventHooks(hSettingChangeEvent, hContact, (LPARAM)&tmp);
	}

	return 0;
}
int CompEnumContactSettings(WPARAM hContact, LPARAM pEnum)
{
	DBCONTACTENUMSETTINGS * pces = (DBCONTACTENUMSETTINGS *)pEnum;

	TDBTSettingDescriptor desc = {0};
	desc.cbSize = sizeof(desc);
	desc.Contact = hContact;
	
	char namebuf[512];
	namebuf[0] = 0;
	strcpy_s(namebuf, pces->szModule);
	strcat_s(namebuf, "/");

	TDBTSettingIterFilter filter = {0};
	filter.cbSize = sizeof(filter);
	filter.Descriptor = &desc;
	filter.hContact = hContact;
	filter.NameStart = namebuf;

	TDBTSettingIterationHandle hiter = DBSettingIterInit((WPARAM)&filter, 0);
	if ((hiter == 0) || (hiter == DBT_INVALIDPARAM))
		return -1;

	int res = 0;
	TDBTSettingHandle hset = DBSettingIterNext(hiter, 0);
	while (hset != 0)
	{
		char * p = strchr(desc.pszSettingName, '/');
		if (p) {
			++p;
		} else {
			p = desc.pszSettingName;
		}

		res = pces->pfnEnumProc(p, pces->lParam);
		if (res == 0)
		{
			hset = DBSettingIterNext(hiter, 0);
		} else {
			hset = 0;
		}
	}

	DBSettingIterClose(hiter, 0);

	if (desc.pszSettingName)
		mir_free(desc.pszSettingName);

	return res;	
}


int CompGetEventCount(WPARAM hContact, LPARAM lParam)
{
	return DBEventGetCount(hContact, 0);
}
int CompAddEvent(WPARAM hContact, LPARAM pEventInfo)
{
	DBEVENTINFO * dbei = (DBEVENTINFO*) pEventInfo;
	if (dbei->cbSize < sizeof(DBEVENTINFO))
		return -1;

	int tmp = NotifyEventHooks(hEventFilterAddedEvent, hContact, pEventInfo);
	if (tmp != 0)
		return tmp;

	if (hContact == 0)
		hContact = gDataBase->getContacts().getRootContact();


	TDBTEvent ev = {0};
	ev.cbSize = sizeof(ev);
	ev.ModuleName = dbei->szModule;
	ev.Timestamp = dbei->timestamp;
	ev.Flags = dbei->flags;
	ev.EventType = dbei->eventType;
	ev.cbBlob = dbei->cbBlob;
	ev.pBlob = dbei->pBlob;

	int res = DBEventAdd(hContact, (LPARAM)&ev);
	NotifyEventHooks(hEventAddedEvent, hContact, res);
	return res;
}
int CompDeleteEvent(WPARAM hContact, LPARAM hEvent)
{
	int res = NotifyEventHooks(hEventDeletedEvent, hContact, hEvent);

	if (hContact == 0)
		hContact = gDataBase->getContacts().getRootContact();

	if (res == 0)
		return DBEventDelete(hContact, hEvent);

	return res;
}
int CompGetBlobSize(WPARAM hEvent, LPARAM lParam)
{
	return DBEventGetBlobSize(hEvent, 0);
}
int CompGetEvent(WPARAM hEvent, LPARAM pEventInfo)
{
	DBEVENTINFO * dbei = (DBEVENTINFO*) pEventInfo;
	if (dbei->cbSize < sizeof(DBEVENTINFO))
		return -1;

	TDBTEvent ev = {0};
	ev.cbSize = sizeof(ev);
	ev.cbBlob = dbei->cbBlob;
	ev.pBlob = dbei->pBlob;

	int res = DBEventGet(hEvent, (LPARAM)&ev);
	
	dbei->szModule = ev.ModuleName;
	dbei->timestamp = ev.Timestamp;
	dbei->flags = ev.Flags;
	dbei->eventType = ev.EventType;
	dbei->cbBlob = ev.cbBlob;
	dbei->pBlob = ev.pBlob;
	
	return res;
}
int CompMarkEventRead(WPARAM hContact, LPARAM hEvent)
{
	return DBEventMarkRead(hContact, hEvent);
}
int CompGetEventContact(WPARAM hEvent, LPARAM lParam)
{
	TDBTContactHandle res = DBEventGetContact(hEvent, 0);
	if (res == gDataBase->getContacts().getRootContact())
		res = 0;

	return res;
}
int CompFindFirstEvent(WPARAM hContact, LPARAM lParam)
{
	if (hContact == 0)
		hContact = gDataBase->getContacts().getRootContact();

	return gDataBase->getEvents().compFirstEvent(hContact);
}
int CompFindFirstUnreadEvent(WPARAM hContact, LPARAM lParam)
{
	if (hContact == 0)
		hContact = gDataBase->getContacts().getRootContact();
	return gDataBase->getEvents().compFirstUnreadEvent(hContact);
}
int CompFindLastEvent(WPARAM hContact, LPARAM lParam)
{
	if (hContact == 0)
		hContact = gDataBase->getContacts().getRootContact();
	return gDataBase->getEvents().compLastEvent(hContact);
}
int CompFindNextEvent(WPARAM hEvent, LPARAM lParam)
{
	return gDataBase->getEvents().compNextEvent(hEvent);
}
int CompFindPrevEvent(WPARAM hEvent, LPARAM lParam)
{
	return gDataBase->getEvents().compPrevEvent(hEvent);
}

int CompEnumModules(WPARAM wParam, LPARAM pCallback)
{
	if (pCallback == NULL)
		return -1;

	DBMODULEENUMPROC cb = (DBMODULEENUMPROC) pCallback;

	return gDataBase->getSettings().CompEnumModules(cb, wParam);
}

void Encrypt(char*msg, BOOL up)
{
	int i;
	int jump;
	if (up)
	{
		jump = 5;
	}
	else
	{
		jump = -5;
	}

	for (i=0; msg[i]; i++)
	{
		msg[i] = msg[i] + jump;
	}

}

int CompEncodeString(WPARAM wParam, LPARAM lParam)
{
	Encrypt((char*)lParam,TRUE);
	return 0;
}

int CompDecodeString(WPARAM wParam, LPARAM lParam)
{
	Encrypt((char*)lParam,FALSE);
	return 0;
}

int CompGetProfileName(WPARAM cbBytes, LPARAM pszName)
{
	return gDataBase->getProfileName(cbBytes, (char *)pszName);
}

int CompGetProfilePath(WPARAM cbBytes, LPARAM pszName)
{
	return gDataBase->getProfilePath(cbBytes, (char *)pszName);
}

bool CompatibilityRegister()
{
	gCompServices[ 0] = CreateServiceFunction(MS_DB_CONTACT_GETCOUNT,         CompGetContactCount);
	gCompServices[ 1] = CreateServiceFunction(MS_DB_CONTACT_FINDFIRST,        CompFindFirstContact);
	gCompServices[ 2] = CreateServiceFunction(MS_DB_CONTACT_FINDNEXT,         CompFindNextContact);
	gCompServices[ 3] = CreateServiceFunction(MS_DB_CONTACT_DELETE,           CompDeleteContact);
	gCompServices[ 4] = CreateServiceFunction(MS_DB_CONTACT_ADD,              CompAddContact);
	gCompServices[ 5] = CreateServiceFunction(MS_DB_CONTACT_IS,	              CompIsDbContact);

	gCompServices[ 6] = CreateServiceFunction(MS_DB_CONTACT_GETSETTING,       CompGetContactSetting);
	gCompServices[ 7] = CreateServiceFunction(MS_DB_CONTACT_GETSETTING_STR,   CompGetContactSettingStr);
	gCompServices[ 8] = CreateServiceFunction(MS_DB_CONTACT_GETSETTINGSTATIC, CompGetContactSettingStatic);
	gCompServices[ 9] = CreateServiceFunction(MS_DB_CONTACT_FREEVARIANT,      CompFreeVariant);
	gCompServices[10] = CreateServiceFunction(MS_DB_CONTACT_WRITESETTING,     CompWriteContactSetting);
	gCompServices[11] = CreateServiceFunction(MS_DB_CONTACT_DELETESETTING,    CompDeleteContactSetting);
	gCompServices[12] = CreateServiceFunction(MS_DB_CONTACT_ENUMSETTINGS,     CompEnumContactSettings);
	//gCompServices[13] = CreateServiceFunction(MS_DB_SETSETTINGRESIDENT,       CompSetSettingResident);

	gCompServices[14] = CreateServiceFunction(MS_DB_EVENT_GETCOUNT,           CompGetEventCount);
	gCompServices[15] = CreateServiceFunction(MS_DB_EVENT_ADD,                CompAddEvent);
	gCompServices[16] = CreateServiceFunction(MS_DB_EVENT_DELETE,             CompDeleteEvent);
	gCompServices[17] = CreateServiceFunction(MS_DB_EVENT_GETBLOBSIZE,        CompGetBlobSize);
	gCompServices[18] = CreateServiceFunction(MS_DB_EVENT_GET,                CompGetEvent);
	gCompServices[19] = CreateServiceFunction(MS_DB_EVENT_MARKREAD,           CompMarkEventRead);
	gCompServices[20] = CreateServiceFunction(MS_DB_EVENT_GETCONTACT,         CompGetEventContact);
	gCompServices[21] = CreateServiceFunction(MS_DB_EVENT_FINDFIRST,          CompFindFirstEvent);
	gCompServices[22] = CreateServiceFunction(MS_DB_EVENT_FINDFIRSTUNREAD,    CompFindFirstUnreadEvent);
	gCompServices[23] = CreateServiceFunction(MS_DB_EVENT_FINDLAST,           CompFindLastEvent);
	gCompServices[24] = CreateServiceFunction(MS_DB_EVENT_FINDNEXT,           CompFindNextEvent);
	gCompServices[25] = CreateServiceFunction(MS_DB_EVENT_FINDPREV,           CompFindPrevEvent);

	gCompServices[26] = CreateServiceFunction(MS_DB_MODULES_ENUM,             CompEnumModules);

	gCompServices[27] = CreateServiceFunction(MS_DB_CRYPT_ENCODESTRING,       CompEncodeString);
	gCompServices[28] = CreateServiceFunction(MS_DB_CRYPT_DECODESTRING,       CompDecodeString);

	gCompServices[29] = CreateServiceFunction(MS_DB_GETPROFILENAME,           CompGetProfileName);
	gCompServices[30] = CreateServiceFunction(MS_DB_GETPROFILEPATH,           CompGetProfilePath);


	hEventDeletedEvent     = CreateHookableEvent(ME_DB_EVENT_DELETED);
	hEventAddedEvent       = CreateHookableEvent(ME_DB_EVENT_ADDED);
	hEventFilterAddedEvent = CreateHookableEvent(ME_DB_EVENT_FILTER_ADD);
	hSettingChangeEvent    = CreateHookableEvent(ME_DB_CONTACT_SETTINGCHANGED);
	hContactDeletedEvent   = CreateHookableEvent(ME_DB_CONTACT_DELETED);
	hContactAddedEvent     = CreateHookableEvent(ME_DB_CONTACT_ADDED);
	return true;
}
bool CompatibilityUnRegister()
{
	int i;
	for (i = 0; i < sizeof(gCompServices) / sizeof(gCompServices[0]); ++i)
	{
		DestroyServiceFunction(gCompServices[i]);
	}
	return true;
}