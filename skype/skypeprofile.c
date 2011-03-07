#include "skypeprofile.h"
#include "skypeapi.h"
#include "utf8.h"

void SkypeProfile_Save(SkypeProfile *pstProf)
{
	DBWriteContactSettingByte(NULL, SKYPE_PROTONAME, "Gender", pstProf->Sex);
	DBWriteContactSettingString(NULL, SKYPE_PROTONAME, "HomePhone", pstProf->HomePhone);
	DBWriteContactSettingString(NULL, SKYPE_PROTONAME, "OfficePhone", pstProf->OfficePhone);
	DBWriteContactSettingString(NULL, SKYPE_PROTONAME, "HomePage", pstProf->HomePage);
	DBWriteContactSettingTString(NULL, SKYPE_PROTONAME, "Nick", pstProf->FullName);
	DBWriteContactSettingTString(NULL, SKYPE_PROTONAME, "City", pstProf->City);
	DBWriteContactSettingTString(NULL, SKYPE_PROTONAME, "Province", pstProf->Province);
}

void SkypeProfile_Load(SkypeProfile *pstProf)
{
	DBVARIANT dbv;

	pstProf->Sex = DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "Gender", 0);
	if(!DBGetContactSettingTString(NULL,SKYPE_PROTONAME,"Nick",&dbv)) 
	{	
		_tcsncpy (pstProf->FullName, dbv.ptszVal, sizeof(pstProf->FullName)/sizeof(TCHAR));
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingTString(NULL,SKYPE_PROTONAME,"HomePage",&dbv)) 
	{	
		_tcsncpy (pstProf->HomePage, dbv.ptszVal, sizeof(pstProf->HomePage)/sizeof(TCHAR));
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingTString(NULL,SKYPE_PROTONAME,"Province",&dbv)) 
	{	
		_tcsncpy (pstProf->Province, dbv.ptszVal, sizeof(pstProf->Province)/sizeof(TCHAR));
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingTString(NULL,SKYPE_PROTONAME,"City",&dbv)) 
	{	
		_tcsncpy (pstProf->City, dbv.ptszVal, sizeof(pstProf->City)/sizeof(TCHAR));
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingTString(NULL,SKYPE_PROTONAME,"OfficePhone",&dbv)) 
	{	
		_tcsncpy (pstProf->OfficePhone, dbv.ptszVal, sizeof(pstProf->OfficePhone)/sizeof(TCHAR));
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingTString(NULL,SKYPE_PROTONAME,"HomePhone",&dbv)) 
	{	
		_tcsncpy (pstProf->HomePhone, dbv.ptszVal, sizeof(pstProf->HomePhone)/sizeof(TCHAR));
		DBFreeVariant(&dbv);
	}
}

void SkypeProfile_LoadFromSkype(SkypeProfile *pstProf)
{
	TCHAR *ptr;

	if (ptr=SkypeGetProfile("FULLNAME"))
	{
		_tcsncpy (pstProf->FullName, ptr, sizeof(pstProf->FullName)/sizeof(TCHAR));
		free(ptr);
	}
	if (ptr=SkypeGetProfile("PHONE_HOME"))
	{
		_tcsncpy (pstProf->HomePhone, ptr, sizeof(pstProf->HomePhone)/sizeof(TCHAR));
		free(ptr);
	}
	if (ptr=SkypeGetProfile("PHONE_OFFICE"))
	{
		_tcsncpy (pstProf->OfficePhone, ptr, sizeof(pstProf->OfficePhone)/sizeof(TCHAR));
		free(ptr);
	}
	if (ptr=SkypeGetProfile("HOMEPAGE"))
	{
		_tcsncpy (pstProf->HomePage, ptr, sizeof(pstProf->HomePage)/sizeof(TCHAR));
		free(ptr);
	}
	if (ptr=SkypeGetProfile("CITY"))
	{
		_tcsncpy (pstProf->City, ptr, sizeof(pstProf->City)/sizeof(TCHAR));
		free(ptr);
	}
	if (ptr=SkypeGetProfile("PROVINCE"))
	{
		_tcsncpy (pstProf->Province, ptr, sizeof(pstProf->Province)/sizeof(TCHAR));
		free(ptr);
	}
}

void SkypeProfile_SaveToSkype(SkypeProfile *pstProf)
{
	char *tmp;

	SkypeSetProfile("PHONE_HOME", pstProf->HomePhone);

	SkypeSetProfile("PHONE_OFFICE", pstProf->OfficePhone);

	SkypeSetProfile("HOMEPAGE", pstProf->HomePage);

	if(utf8_encode((const char *)pstProf->FullName, &tmp) != -1 )
	{
		SkypeSetProfile("FULLNAME", tmp);
		free (tmp);
	}

	if(utf8_encode((const char *)pstProf->City, &tmp) != -1 )
	{
		SkypeSetProfile("CITY", tmp);
		free (tmp);
	}

	if(utf8_encode((const char *)pstProf->Province, &tmp) != -1 )
	{
		SkypeSetProfile("PROVINCE", tmp);
		free (tmp);
	}
}