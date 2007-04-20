/*
 * Contactlist management functions
 */

#include "skype.h"
#include "skypeapi.h"
#include "debug.h"
#include "pthread.h"
#include "voiceservice.h"
#include "../../include/m_langpack.h"

// #include <shlwapi.h>


// Imported Globals
extern char pszSkypeProtoName[MAX_PATH+30];
extern HINSTANCE hInst;
extern BOOL bSkypeOut;
extern char protocol;

// Handles
static HANDLE hMenuCallItem, hMenuSkypeOutCallItem, hMenuHoldCallItem, hMenuFileTransferItem, hMenuChatInitItem;

// Check if alpha blending icons are supported
// Seems to be not neccessary
/*
BOOL SupportAlphaIcons(void) {
	HANDLE hMod;
	DLLVERSIONINFO tDVI={0};
	BOOL retval=FALSE;
	FARPROC pDllGetVersion;

	if (!(hMod=LoadLibrary("comctl32.dll"))) return FALSE;
	if (pDllGetVersion=GetProcAddress(hMod, "DllGetVersion")) {
		tDVI.cbSize=sizeof(tDVI);
		if (!pDllGetVersion ((DLLVERSIONINFO *)&tDVI)) {
			if (GetDeviceCaps(GetDC(NULL), BITSPIXEL)*GetDeviceCaps(GetDC(NULL), PLANES)>=32 &&
				tDVI.dwMajorVersion>=6) 
				retval=TRUE;
		}
	}
	FreeLibrary(hMod);
	return retval;
}
*/

CLISTMENUITEM CallItem(void) {
	CLISTMENUITEM mi={0};

	mi.cbSize=sizeof(mi);
	mi.position=-2000005000;
	mi.flags=CMIF_NOTOFFLINE;
	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_CALL));
	mi.pszContactOwner=pszSkypeProtoName;
	mi.pszName=Translate("Call (Skype)");
	mi.pszService=SKYPE_CALL;
	
	return mi;
}

CLISTMENUITEM SkypeOutCallItem(void) {
	CLISTMENUITEM mi={0};

	mi.cbSize=sizeof(mi);
	mi.position=-2000005000;
	mi.flags=CMIF_HIDDEN;
	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_CALLSKYPEOUT));
	mi.pszName=Translate("Call using SkypeOut");
	mi.pszService=SKYPEOUT_CALL;

	return mi;
}

CLISTMENUITEM HupItem(void) {
	CLISTMENUITEM mi={0};

	mi.cbSize=sizeof(mi);
	mi.position=-2000005000;
	mi.flags=CMIF_NOTOFFLINE;
	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_HANGUP));
	mi.pszName=Translate("Hang up call (Skype)");
	mi.pszService=SKYPE_CALL;

	return mi;
}

CLISTMENUITEM SkypeOutHupItem(void) {
	CLISTMENUITEM mi={0};

	mi.cbSize=sizeof(mi);
	mi.position=-2000005000;
	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_HANGUP));
	mi.pszName=Translate("Hang up SkypeOut call");
	mi.pszService=SKYPEOUT_CALL;
	return mi;
}

CLISTMENUITEM HoldCallItem(void) {
	CLISTMENUITEM mi={0};

	mi.cbSize=sizeof(mi);
	mi.position=-2000005000;
	mi.flags=CMIF_HIDDEN|CMIF_NOTOFFLINE;
	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_HOLD));
	mi.pszName=Translate("Hold call");
	mi.pszService=SKYPE_HOLDCALL;
	return mi;
}

CLISTMENUITEM ResumeCallItem(void) {
	CLISTMENUITEM mi={0};

	mi.cbSize=sizeof(mi);
	mi.position=-2000005000;
	mi.flags=CMIF_HIDDEN|CMIF_NOTOFFLINE;
	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_RESUME));
	mi.pszName=Translate("Resume call");
	mi.pszService=SKYPE_HOLDCALL;
	return mi;
}

CLISTMENUITEM FileTransferItem(void) {
	CLISTMENUITEM mi={0};

	// Stolen from file.c of Miranda core
	mi.cbSize=sizeof(mi);
	mi.position=-2000020000;
	mi.flags=CMIF_HIDDEN|CMIF_NOTOFFLINE;
	mi.hIcon=LoadSkinnedIcon(SKINICON_EVENT_FILE);
	mi.pszName=Translate("&File");
	mi.pszContactOwner=pszSkypeProtoName;
	mi.pszService=SKYPE_SENDFILE;
	return mi;
}

CLISTMENUITEM ChatInitItem(void) {
	CLISTMENUITEM mi={0};

	mi.cbSize=sizeof(mi);
	mi.position=-2000020000;
	mi.flags=CMIF_HIDDEN|CMIF_NOTOFFLINE;
	mi.hIcon=LoadIcon( hInst, MAKEINTRESOURCE( IDI_INVITE ));
	mi.pszName=Translate("&Open groupchat");
	mi.pszContactOwner=pszSkypeProtoName;
	mi.pszService=SKYPE_CHATNEW;
	return mi;
}

HANDLE add_contextmenu(HANDLE hContact) {
	CLISTMENUITEM mi;
	
	if (!HasVoiceService()) {
		mi=CallItem();
		hMenuCallItem=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0,(LPARAM)&mi);
	}
	
	mi=SkypeOutCallItem();
	hMenuSkypeOutCallItem=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0,(LPARAM)&mi);

	if (!HasVoiceService()) {
		mi=HoldCallItem();
		hMenuHoldCallItem=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0,(LPARAM)&mi);
	}

    // We cannot use flag PF1_FILESEND for sending files, as Skype opens its own
	// sendfile-Dialog.
	mi=FileTransferItem();
	hMenuFileTransferItem=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0,(LPARAM)&mi);
    
   	mi=ChatInitItem();
	hMenuChatInitItem=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0,(LPARAM)&mi);


	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-2000005000;
	mi.flags=0;
	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_IMPORT));
	mi.pszContactOwner=pszSkypeProtoName;
	mi.pszName=Translate("Import Skype history");
	mi.pszService=SKYPE_IMPORTHISTORY;
	return (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0,(LPARAM)&mi);
}

HANDLE add_mainmenu(void) {
	CLISTMENUITEM mi={0};

	mi.cbSize=sizeof(mi);
	mi.position=-2000005000;
	mi.flags=0;
	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_ADD));
	mi.pszContactOwner=pszSkypeProtoName;
	mi.pszName=Translate("Add Skype contact");
	mi.pszService=SKYPE_ADDUSER;
	return (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM, (WPARAM)NULL,(LPARAM)&mi);

}

int __cdecl  PrebuildContactMenu(WPARAM wParam, LPARAM lParam) {
	DBVARIANT dbv;
	CLISTMENUITEM mi;
	char *szProto;

	if (!(szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0))) return 0;

	if (!HasVoiceService()) {
		// Clear hold-Item in case it exists
		mi=HoldCallItem();
		mi.flags|=CMIM_ALL;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuHoldCallItem,(LPARAM)&mi);
	}

	if (!strcmp(szProto, pszSkypeProtoName)) {
		if (!HasVoiceService()) {
			if (!DBGetContactSetting((HANDLE)wParam, pszSkypeProtoName, "CallId", &dbv)) {
				if (DBGetContactSettingByte((HANDLE)wParam, pszSkypeProtoName, "OnHold", 0))
					mi=ResumeCallItem(); else mi=HoldCallItem();
				mi.flags=CMIM_ALL;
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuHoldCallItem,(LPARAM)&mi);

				mi=HupItem();
				DBFreeVariant(&dbv);
			} else mi=CallItem();
        
			if (DBGetContactSettingByte((HANDLE)wParam, pszSkypeProtoName, "ChatRoom", 0)!=0) 
				mi.flags |= CMIF_HIDDEN;
        
			mi.flags|=CMIM_ALL;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuCallItem,(LPARAM)&mi);
		}

		// Clear SkypeOut menu in case it exists
		mi=SkypeOutCallItem();
		mi.flags|=CMIM_ALL;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuSkypeOutCallItem,(LPARAM)&mi);

		// File sending and groupchat-creation works starting with protocol version 5
		if (protocol>=5) {
			mi=FileTransferItem();
            if (DBGetContactSettingByte((HANDLE)wParam, pszSkypeProtoName, "ChatRoom", 0)==0)
			    mi.flags ^= CMIF_HIDDEN;
			mi.flags |= CMIM_FLAGS;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuFileTransferItem,(LPARAM)&mi);
            
            mi=ChatInitItem();
			if (DBGetContactSettingByte(NULL, pszSkypeProtoName, "UseGroupchat", 0) &&
				DBGetContactSettingByte((HANDLE)wParam, pszSkypeProtoName, "ChatRoom", 0)==0)
					mi.flags ^= CMIF_HIDDEN;
			mi.flags |= CMIM_FLAGS;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuChatInitItem,(LPARAM)&mi);
		}

	} else if (bSkypeOut) {
		if (!DBGetContactSetting((HANDLE)wParam, pszSkypeProtoName, "CallId", &dbv)) {
			mi=SkypeOutHupItem();
			DBFreeVariant(&dbv);
		} else {
			mi=SkypeOutCallItem();
			if(!DBGetContactSetting((HANDLE)wParam,"UserInfo","MyPhone0",&dbv)) {
				DBFreeVariant(&dbv);
				mi.flags=0;
			}
		}
		mi.flags|=CMIM_ALL;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuSkypeOutCallItem,(LPARAM)&mi);
	}

	return 0;
}

/*
int ClistDblClick(WPARAM wParam, LPARAM lParam) {
	char *szProto;

	szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, wParam, 0 );
	if (szProto!=NULL && !strcmp(szProto, pszSkypeProtoName) && 
		DBGetContactSettingWord((HANDLE)wParam, pszSkypeProtoName, "Status", ID_STATUS_OFFLINE)==ID_STATUS_ONTHEPHONE) {
			SkypeCall(wParam, 0);
	}

	return 0;
}
*/

HANDLE find_contact(char *name) {
	char *szProto;
	int tCompareResult;
	HANDLE hContact;
	DBVARIANT dbv;

	// already on list?
	for (hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);hContact != NULL;hContact=(HANDLE)CallService( MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0)) 
	{
		szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0 );
		if (szProto!=NULL && !strcmp(szProto, pszSkypeProtoName) &&	DBGetContactSettingByte(hContact, pszSkypeProtoName, "ChatRoom", 0)==0)	
		{
			if (DBGetContactSetting(hContact, pszSkypeProtoName, SKYPE_NAME, &dbv)) continue;
            tCompareResult = strcmp(dbv.pszVal, name);
			DBFreeVariant(&dbv);
			if (tCompareResult) continue;
			return hContact; // already there, return handle
		}
	}
	return NULL;
}


HANDLE add_contact(char *name, DWORD flags) {
	HANDLE hContact;

	// already on list?
	if (hContact=find_contact(name)) {
		if (!(flags & PALF_TEMPORARY) && DBGetContactSettingByte(hContact, "CList", "NotOnList", 1)) {
			DBDeleteContactSetting( hContact, "CList", "NotOnList" );
			DBDeleteContactSetting( hContact, "CList", "Hidden" );
		}
		LOG("add_contact: Found", name);
		return hContact; // already there, return handle
	}
	// no, so add
	
	LOG("add_contact: Adding", name);
	hContact=(HANDLE)CallServiceSync(MS_DB_CONTACT_ADD, 0, 0);
	if (hContact) {
		char *str;

		if (CallServiceSync(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact,(LPARAM)pszSkypeProtoName)!=0) {
			LOG("add_contact", "Ouch! MS_PROTO_ADDTOCONTACT failed for some reason");
			CallServiceSync(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
			return NULL;
		}
		if (name[0]) DBWriteContactSettingString(hContact, pszSkypeProtoName, SKYPE_NAME, name);

   		if (flags & PALF_TEMPORARY ) {
			DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
			DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
		}
		if (name[0]) {
			if (str=(char*)malloc(strlen(name)+22)) {
				strcpy(str, "GET USER ");
				strcat(str, name);
				strcat(str, " DISPLAYNAME");
				SkypeSend(str);
				free(str);
			} else {LOG("add_contact", "Ouch! Memory allocation failed!");}
		} else {LOG("add_contact", "Info: The contact added has no name.");}
	} else {LOG("add_contact", "Ouch! MS_DB_CONTACT_ADD failed for some reason");}
	LOG("add_contact", "succeeded");
	return hContact;
}

void logoff_contacts(void) {
	HANDLE hContact;
	char *szProto;

	LOG("logoff_contacts", "Logging off contacts.");
	for (hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);hContact != NULL;hContact=(HANDLE)CallService( MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0)) {
		szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0 );
		if (szProto!=NULL && !strcmp(szProto, pszSkypeProtoName) &&	DBGetContactSettingByte(hContact, pszSkypeProtoName, "ChatRoom", 0) == 0)
		{
			if (DBGetContactSettingWord(hContact, pszSkypeProtoName, "Status", ID_STATUS_OFFLINE)!=ID_STATUS_OFFLINE)
				DBWriteContactSettingWord(hContact, pszSkypeProtoName, "Status", ID_STATUS_OFFLINE);

			DBDeleteContactSetting(hContact, pszSkypeProtoName, "CallId");
		}
	}
}
