// System includes
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <process.h>
#include <time.h>
#include "resource.h"

// Miranda database access
#include "../../include/newpluginapi.h"
#include "../../include/m_database.h"

class CSkypeProfile
{
public:
	char SkypeProtoName[256];
	char FullName[256];
	char HomePhone[256];
	char OfficePhone[256];
	char HomePage[256];
	char City[256];
	char Province[256];
	BYTE Sex;

	void Load(void);
	void Save(void);
	void CSkypeProfile::SaveToSkype(void);
	void CSkypeProfile::LoadFromSkype(void);
};