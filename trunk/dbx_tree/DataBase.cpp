#include "DataBase.h"
#include "newpluginapi.h"

CDataBase *gDataBase = NULL;


CDataBase::CDataBase(const char* FileName)
{
	m_SettingsFN = new char[strlen(FileName) + 1];
	m_PrivateFN = new char[strlen(FileName) + 5];	
	strcpy_s(m_SettingsFN, strlen(FileName) + 1, FileName);
	strcpy_s(m_PrivateFN, strlen(FileName) + 5, FileName);
	
	char * tmp = strrchr(m_PrivateFN, '.');
	if (tmp) 
		(*tmp) = '\0';

	strcat_s(m_PrivateFN, strlen(FileName) + 5, ".usr");


	m_Opened = false;
	m_SettingsFile = NULL;
	m_PrivateFile = NULL;
	
	m_Entries = NULL;
	m_Virtuals = NULL;
	m_Settings = NULL;
}
CDataBase::~CDataBase()
{
	/*if (m_Entries) delete m_Entries;
	if (m_Virtuals) delete m_Virtuals;
	if (m_Settings) delete m_Settings;
*/
	if (m_SettingsFile) delete m_SettingsFile;
	if (m_PrivateFile) delete m_PrivateFile;

	m_SettingsFile = NULL;
	m_PrivateFile = NULL;
	
	m_Entries = NULL;
	m_Virtuals = NULL;
	m_Settings = NULL;

	delete[] m_PrivateFile;
	delete[] m_SettingsFile;
}
int CDataBase::CreateDB()
{
	try 
	{
		m_SettingsFile = new CDirectAccess(m_SettingsFN);
		m_PrivateFile = new CDirectAccess(m_PrivateFN);
	}
	catch (char *)
	{
		return EMKPRF_CREATEFAILED;
	}

	TSettingsHeader sethead = {0};
	TPrivateHeader usrhead = {0};

	memcpy(sethead.Signature, cSettingsSignature, sizeof(cSettingsSignature));
	sethead.Version = cDBVersion;
	sethead.Settings = 0;
	sethead.FileSize = sizeof(TSettingsHeader);
	sethead.WastedBytes = 0;

	m_SettingsFile->Write(&sethead, 0, sizeof(TSettingsHeader));
	delete m_SettingsFile;
	m_SettingsFile = NULL;

	memcpy(usrhead.Signature, cPrivateSignature, sizeof(cPrivateSignature));
	usrhead.Version = cDBVersion;
	usrhead.RootEntry = 0;
	usrhead.Entries = 0;
	usrhead.Virtuals = 0;
	usrhead.FileSize = sizeof(TPrivateHeader);
	usrhead.WastedBytes = 0;
	usrhead.EventIndex = 1;

	m_PrivateFile->Write(&usrhead, 0, sizeof(TPrivateHeader));
	delete m_PrivateFile;
	m_PrivateFile = NULL;

	return 0;
}


int CDataBase::CheckDB()
{
	TSettingsHeader sethead = {0};

	try 
	{
		m_SettingsFile = new CDirectAccess(m_SettingsFN);	
	} 
	catch (char * e)
	{
		return EGROKPRF_CANTREAD;
	}
	m_SettingsFile->Read(&sethead, 0, sizeof(TSettingsHeader));
	delete m_SettingsFile;
	m_SettingsFile = NULL;

	if (0 != memcmp(sethead.Signature, cSettingsSignature, sizeof(cSettingsSignature)))
		return EGROKPRF_UNKHEADER;

	if (cDBVersion < sethead.Version)
		return EGROKPRF_VERNEWER;

	HANDLE htmp = CreateFile(m_PrivateFN, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
	if (htmp != INVALID_HANDLE_VALUE)
	{
		TPrivateHeader usrhead = {0};
		
		CloseHandle(htmp);

		try 
		{
			m_PrivateFile = new CDirectAccess(m_PrivateFN);
		} 
		catch (char * e)
		{
			return EGROKPRF_CANTREAD;
		}
		m_PrivateFile->Read(&usrhead, 0, sizeof(TPrivateHeader));
		delete m_PrivateFile;
		m_PrivateFile = NULL;

		if (0 != memcmp(usrhead.Signature, cPrivateSignature, sizeof(cPrivateSignature)))
			return EGROKPRF_UNKHEADER;

		if (cDBVersion < usrhead.Version)
			return EGROKPRF_VERNEWER;
		
	}

	return EGROKPRF_NOERROR;
}
int CDataBase::OpenDB()
{
	if (GetVersion() & 0x80000000)
	{ // win98
		try 
		{
			m_SettingsFile = new CDirectAccess(m_SettingsFN);
			m_PrivateFile = new CDirectAccess(m_PrivateFN);
		} 
		catch (char * e)
		{
			return GetLastError();
		}
	} else { //win nt, 2000, xp.. usw
		try 
		{
			m_SettingsFile = new CMappedMemory(m_SettingsFN);
			m_PrivateFile = new CMappedMemory(m_PrivateFN);
		} 
		catch (char * e)
		{
			return GetLastError();
		}
	}

	return 0;
}
