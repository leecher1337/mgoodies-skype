#include "skypeprofile.h"
#include "skypeapi.h"
#include "utf8.h"

void CSkypeProfile::Save(void)
{
	DBWriteContactSettingByte(NULL, this->SkypeProtoName, "Gender", this->Sex);
	DBWriteContactSettingString(NULL, this->SkypeProtoName, "HomePhone", this->HomePhone);
	DBWriteContactSettingString(NULL, this->SkypeProtoName, "OfficePhone", this->OfficePhone);
	DBWriteContactSettingString(NULL, this->SkypeProtoName, "HomePage", this->HomePage);

	if(DBWriteContactSettingTString(NULL, this->SkypeProtoName, "Nick", this->FullName)) {
		#if defined( _UNICODE )
			char buff[TEXT_LEN];
			WideCharToMultiByte(code_page, 0, this->FullName, -1, buff, TEXT_LEN, 0, 0);
			buff[TEXT_LEN] = 0;
			DBWriteContactSettingString(0, this->SkypeProtoName, "Nick", buff);
		#endif
	}
	
	if(DBWriteContactSettingTString(NULL, this->SkypeProtoName, "City", this->City)) {
		#if defined( _UNICODE )
			char buff[TEXT_LEN];
			WideCharToMultiByte(code_page, 0, this->City, -1, buff, TEXT_LEN, 0, 0);
			buff[TEXT_LEN] = 0;
			DBWriteContactSettingString(0, this->SkypeProtoName, "City", buff);
		#endif
	}
	
	if(DBWriteContactSettingTString(NULL, this->SkypeProtoName, "Province", this->Province)) {
		#if defined( _UNICODE )
			char buff[TEXT_LEN];
			WideCharToMultiByte(code_page, 0, this->Province, -1, buff, TEXT_LEN, 0, 0);
			buff[TEXT_LEN] = 0;
			DBWriteContactSettingString(0, this->SkypeProtoName, "Province", buff);
		#endif
	}
}


void CSkypeProfile::Load(void)
{
	DBVARIANT dbv;
	memset(this->FullName,0,sizeof(this->FullName));
	memset(this->HomePage,0,sizeof(this->HomePage));
	memset(this->Province,0,sizeof(this->Province));
	memset(this->OfficePhone,0,sizeof(this->OfficePhone));
	memset(this->HomePhone,0,sizeof(this->HomePhone));

	this->Sex = DBGetContactSettingByte(NULL, SkypeProtoName, "Gender", 0);
	if(!DBGetContactSettingTString(NULL,this->SkypeProtoName,"Nick",&dbv)) 
	{
		_tcsncpy(this->FullName,dbv.ptszVal,TEXT_LEN);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSetting(NULL,this->SkypeProtoName,"HomePage",&dbv)) 
	{
		sprintf(this->HomePage,"%s",dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingTString(NULL,this->SkypeProtoName,"Province",&dbv)) 
	{
		_tcsncpy(this->Province,dbv.ptszVal,TEXT_LEN);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingTString(NULL,this->SkypeProtoName,"City",&dbv)) 
	{
		_tcsncpy(this->City,dbv.ptszVal,TEXT_LEN);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSetting(NULL,this->SkypeProtoName,"OfficePhone",&dbv)) 
	{
		sprintf(this->OfficePhone,"%s",dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSetting(NULL,this->SkypeProtoName,"HomePhone",&dbv)) 
	{
		sprintf(this->HomePhone,"%s",dbv.pszVal);
		DBFreeVariant(&dbv);
	}
}

void CSkypeProfile::LoadFromSkype(void)
{
	strcpy(this->FullName, SkypeGetProfile("FULLNAME"));
	strcpy(this->HomePhone, SkypeGetProfile("PHONE_HOME"));
	strcpy(this->OfficePhone, SkypeGetProfile("PHONE_OFFICE"));
	strcpy(this->HomePage, SkypeGetProfile("HOMEPAGE"));
	strcpy(this->City, SkypeGetProfile("CITY"));
	strcpy(this->Province, SkypeGetProfile("PROVINCE"));
}

void CSkypeProfile::SaveToSkype(void)
{
	char *tmp;

	SkypeSetProfile("PHONE_HOME", this->HomePhone);

	SkypeSetProfile("PHONE_OFFICE", this->OfficePhone);

	SkypeSetProfile("HOMEPAGE", this->HomePage);

	if(utf8_encode((const char *)this->FullName, &tmp) != -1 )
		SkypeSetProfile("FULLNAME", tmp);

	if(utf8_encode((const char *)this->City, &tmp) != -1 )
		SkypeSetProfile("CITY", tmp);

	if(utf8_encode((const char *)this->Province, &tmp) != -1 )
		SkypeSetProfile("PROVINCE", tmp);
}