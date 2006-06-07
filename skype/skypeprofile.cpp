#include "skypeprofile.h"
#include "skypeapi.h"

void CSkypeProfile::Save(void)
{
	DBWriteContactSettingByte(NULL, this->SkypeProtoName, "Gender", this->Sex);
	DBWriteContactSettingString(NULL, this->SkypeProtoName, "Nick", this->FullName);
	DBWriteContactSettingString(NULL, this->SkypeProtoName, "HomePhone", this->HomePhone);
	DBWriteContactSettingString(NULL, this->SkypeProtoName, "OfficePhone", this->OfficePhone);
	DBWriteContactSettingString(NULL, this->SkypeProtoName, "City", this->City);
	DBWriteContactSettingString(NULL, this->SkypeProtoName, "Province", this->Province);
	DBWriteContactSettingString(NULL, this->SkypeProtoName, "HomePage", this->HomePage);
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
	if(!DBGetContactSetting(NULL,this->SkypeProtoName,"Nick",&dbv)) 
	{
		sprintf_s(this->FullName,"%s",dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSetting(NULL,this->SkypeProtoName,"HomePage",&dbv)) 
	{
		sprintf_s(this->HomePage,"%s",dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSetting(NULL,this->SkypeProtoName,"Province",&dbv)) 
	{
		sprintf_s(this->Province,"%s",dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSetting(NULL,this->SkypeProtoName,"City",&dbv)) 
	{
		sprintf_s(this->City,"%s",dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSetting(NULL,this->SkypeProtoName,"OfficePhone",&dbv)) 
	{
		sprintf_s(this->OfficePhone,"%s",dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSetting(NULL,this->SkypeProtoName,"HomePhone",&dbv)) 
	{
		sprintf_s(this->HomePhone,"%s",dbv.pszVal);
		DBFreeVariant(&dbv);
	}
}

void CSkypeProfile::LoadFromSkype(void)
{
	strcpy_s(this->FullName, SkypeGetProfile("FULLNAME"));
	strcpy_s(this->HomePhone, SkypeGetProfile("PHONE_HOME"));
	strcpy_s(this->OfficePhone, SkypeGetProfile("PHONE_OFFICE"));
	strcpy_s(this->HomePage, SkypeGetProfile("HOMEPAGE"));
	strcpy_s(this->City, SkypeGetProfile("CITY"));
	strcpy_s(this->Province, SkypeGetProfile("PROVINCE"));
}

void CSkypeProfile::SaveToSkype(void)
{
	SkypeSetProfile("FULLNAME", this->FullName);
	SkypeSetProfile("PHONE_HOME", this->HomePhone);
	SkypeSetProfile("PHONE_OFFICE", this->OfficePhone);
	SkypeSetProfile("HOMEPAGE", this->HomePage);
	SkypeSetProfile("CITY", this->City);
	SkypeSetProfile("PROVINCE", this->Province);
}