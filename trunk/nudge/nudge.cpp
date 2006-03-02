#include "headers.h"
#include "nudge.h"

void CNudge::Save(void)
{
	char SectionName[MAXMODULELABELLENGTH + 30];
	sprintf(SectionName,"useByProtocol"); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->useByProtocol); 
}


void CNudge::Load(void)
{
	char SectionName[MAXMODULELABELLENGTH + 30];
	sprintf(SectionName,"useByProtocol"); 
	this->useByProtocol = DBGetContactSettingByte(NULL, "Nudge", SectionName, FALSE) != 0; 
}

void CNudgeElement::Save(void)
{
	char SectionName[MAXMODULELABELLENGTH + 30];
	sprintf(SectionName,"%s-popupBackColor", ProtocolName);
	DBWriteContactSettingDword(NULL, "Nudge", SectionName, this->popupBackColor);
	sprintf(SectionName,"%s-popupTextColor", ProtocolName); 
	DBWriteContactSettingDword(NULL, "Nudge", SectionName, this->popupTextColor);
	sprintf(SectionName,"%s-popupTimeSec", ProtocolName);
	DBWriteContactSettingDword(NULL, "Nudge", SectionName, this->popupTimeSec);
	sprintf(SectionName,"%s-popupWindowColor", ProtocolName);
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->popupWindowColor);
	sprintf(SectionName,"%s-showEvent", ProtocolName); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->showEvent); 
	sprintf(SectionName,"%s-showPopup", ProtocolName); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->showPopup); 
	sprintf(SectionName,"%s-shakeClist", ProtocolName); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->shakeClist); 
	sprintf(SectionName,"%s-shakeChat", ProtocolName); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->shakeChat); 
	sprintf(SectionName,"%s-enabled", ProtocolName); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->enabled);
	sprintf(SectionName,"%s-statusFlags", ProtocolName);
	DBWriteContactSettingDword(NULL, "Nudge", SectionName, this->statusFlags);
}


void CNudgeElement::Load(void)
{
	char SectionName[MAXMODULELABELLENGTH + 30];
	sprintf(SectionName,"%s-popupBackColor", ProtocolName);
	this->popupBackColor = DBGetContactSettingDword(NULL, "Nudge", SectionName, GetSysColor(COLOR_BTNFACE));
	sprintf(SectionName,"%s-popupTextColor", ProtocolName); 
	this->popupTextColor = DBGetContactSettingDword(NULL, "Nudge", SectionName, GetSysColor(COLOR_WINDOWTEXT));
	sprintf(SectionName,"%s-popupTimeSec", ProtocolName);
	this->popupTimeSec = DBGetContactSettingDword(NULL, "Nudge", SectionName, 4);
	sprintf(SectionName,"%s-popupWindowColor", ProtocolName);
	this->popupWindowColor = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0;
	sprintf(SectionName,"%s-showEvent", ProtocolName); 
	this->showEvent = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0; 
	sprintf(SectionName,"%s-showPopup", ProtocolName); 
	this->showPopup = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0; 
	sprintf(SectionName,"%s-shakeClist", ProtocolName); 
	this->shakeClist = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0;  
	sprintf(SectionName,"%s-shakeChat", ProtocolName); 
	this->shakeChat = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0; 
	sprintf(SectionName,"%s-enabled", ProtocolName); 
	this->enabled = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0;
	sprintf(SectionName,"%s-statusFlags", ProtocolName);
	this->statusFlags = DBGetContactSettingDword(NULL, "Nudge", SectionName, 0);
	this->LastSent = time(NULL);
}