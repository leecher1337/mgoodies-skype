#include "headers.h"
#include "nudge.h"

void CNudge::Save(void)
{
	char SectionName[512];
	mir_snprintf(SectionName,512,"useByProtocol"); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->useByProtocol);
	mir_snprintf(SectionName,512,"RecvTimeSec");
	DBWriteContactSettingDword(NULL, "Nudge", SectionName, this->recvTimeSec);
	mir_snprintf(SectionName,512,"SendTimeSec");
	DBWriteContactSettingDword(NULL, "Nudge", SectionName, this->sendTimeSec);
	mir_snprintf(SectionName,512,"ResendDelaySec");
	DBWriteContactSettingDword(NULL, "Nudge", SectionName, this->resendDelaySec);
}


void CNudge::Load(void)
{
	char SectionName[512];
	mir_snprintf(SectionName,512,"useByProtocol"); 
	this->useByProtocol = DBGetContactSettingByte(NULL, "Nudge", SectionName, FALSE) != 0;
	mir_snprintf(SectionName,512,"RecvTimeSec");
	this->recvTimeSec = DBGetContactSettingDword(NULL, "Nudge", SectionName, 30);
	mir_snprintf(SectionName,512,"SendTimeSec");
	this->sendTimeSec = DBGetContactSettingDword(NULL, "Nudge", SectionName, 30);
	mir_snprintf(SectionName,512,"ResendDelaySec");
	this->resendDelaySec = DBGetContactSettingDword(NULL, "Nudge", SectionName, 3);
}

int CNudgeElement::ShowContactMenu(bool show)
{

	CLISTMENUITEM mi;
	memset( &mi, 0, sizeof( mi ));
	mi.cbSize = sizeof( mi );
	mi.flags =  show? CMIM_FLAGS : CMIM_FLAGS | CMIF_HIDDEN ;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hContactMenu, (LPARAM)&mi);

	return 0;
}

void CNudgeElement::Save(void)
{
	char SectionName[512];
	mir_snprintf(SectionName,512,"%s-popupBackColor", ProtocolName);
	DBWriteContactSettingDword(NULL, "Nudge", SectionName, this->popupBackColor);
	mir_snprintf(SectionName,512,"%s-popupTextColor", ProtocolName); 
	DBWriteContactSettingDword(NULL, "Nudge", SectionName, this->popupTextColor);
	mir_snprintf(SectionName,512,"%s-popupTimeSec", ProtocolName);
	DBWriteContactSettingDword(NULL, "Nudge", SectionName, this->popupTimeSec);
	mir_snprintf(SectionName,512,"%s-popupWindowColor", ProtocolName);
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->popupWindowColor);
	mir_snprintf(SectionName,512,"%s-showEvent", ProtocolName); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->showEvent); 
	mir_snprintf(SectionName,512,"%s-showStatus", ProtocolName); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->showStatus); 
	mir_snprintf(SectionName,512,"%s-showPopup", ProtocolName); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->showPopup); 
	mir_snprintf(SectionName,512,"%s-shakeClist", ProtocolName); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->shakeClist); 
	mir_snprintf(SectionName,512,"%s-shakeChat", ProtocolName); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->shakeChat); 
	mir_snprintf(SectionName,512,"%s-enabled", ProtocolName); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->enabled);
	mir_snprintf(SectionName,512,"%s-autoResend", ProtocolName); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->autoResend);
	mir_snprintf(SectionName,512,"%s-statusFlags", ProtocolName);
	DBWriteContactSettingDword(NULL, "Nudge", SectionName, this->statusFlags);
	mir_snprintf(SectionName,512,"%s-recText", ProtocolName);
	if(DBWriteContactSettingTString(NULL, "Nudge", SectionName, this->recText)) {
		#if defined( _UNICODE )
			char buff[TEXT_LEN];
			WideCharToMultiByte(code_page, 0, this->recText, -1, buff, TEXT_LEN, 0, 0);
			buff[TEXT_LEN] = 0;
			DBWriteContactSettingString(0, "Nudge", SectionName, buff);
		#endif
	}
	mir_snprintf(SectionName,512,"%s-senText", ProtocolName);
	if(DBWriteContactSettingTString(NULL, "Nudge", SectionName, this->senText)) {
		#if defined( _UNICODE )
			char buff[TEXT_LEN];
			WideCharToMultiByte(code_page, 0, this->senText, -1, buff, TEXT_LEN, 0, 0);
			buff[TEXT_LEN] = 0;
			DBWriteContactSettingString(0, "Nudge", SectionName, buff);
		#endif
	}
}


void CNudgeElement::Load(void)
{
	DBVARIANT dbv;
	char SectionName[512];
	mir_snprintf(SectionName,512,"%s-popupBackColor", ProtocolName);
	this->popupBackColor = DBGetContactSettingDword(NULL, "Nudge", SectionName, GetSysColor(COLOR_BTNFACE));
	mir_snprintf(SectionName,512,"%s-popupTextColor", ProtocolName); 
	this->popupTextColor = DBGetContactSettingDword(NULL, "Nudge", SectionName, GetSysColor(COLOR_WINDOWTEXT));
	mir_snprintf(SectionName,512,"%s-popupTimeSec", ProtocolName);
	this->popupTimeSec = DBGetContactSettingDword(NULL, "Nudge", SectionName, 4);
	mir_snprintf(SectionName,512,"%s-popupWindowColor", ProtocolName);
	this->popupWindowColor = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0;
	mir_snprintf(SectionName,512,"%s-showEvent", ProtocolName); 
	this->showEvent = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0; 
	mir_snprintf(SectionName,512,"%s-showStatus", ProtocolName); 
	this->showStatus = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0; 
	mir_snprintf(SectionName,512,"%s-showPopup", ProtocolName); 
	this->showPopup = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0; 
	mir_snprintf(SectionName,512,"%s-shakeClist", ProtocolName); 
	this->shakeClist = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0;  
	mir_snprintf(SectionName,512,"%s-shakeChat", ProtocolName); 
	this->shakeChat = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0; 
	mir_snprintf(SectionName,512,"%s-enabled", ProtocolName); 
	this->enabled = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0;
	mir_snprintf(SectionName,512,"%s-autoResend", ProtocolName); 
	this->autoResend = DBGetContactSettingByte(NULL, "Nudge", SectionName, FALSE) != 0;
	mir_snprintf(SectionName,512,"%s-statusFlags", ProtocolName);
	this->statusFlags = DBGetContactSettingDword(NULL, "Nudge", SectionName, 967);
	mir_snprintf(SectionName,512,"%s-recText", ProtocolName);
	if(!DBGetContactSettingTString(NULL,"Nudge",SectionName,&dbv)) 
	{
		_tcsncpy(this->recText,dbv.ptszVal,TEXT_LEN);
		if(_tcsclen(this->recText) < 1)
			_tcsncpy(this->recText,TranslateT("You received a nudge"),TEXT_LEN);
		DBFreeVariant(&dbv);
	}
	mir_snprintf(SectionName,512,"%s-senText", ProtocolName);
	if(!DBGetContactSettingTString(NULL,"Nudge",SectionName,&dbv)) 
	{
		_tcsncpy(this->senText,dbv.ptszVal,TEXT_LEN);
		if(_tcsclen(this->senText) < 1)
			_tcsncpy(this->senText,TranslateT("You sent a nudge"),TEXT_LEN);
		DBFreeVariant(&dbv);
	}
}