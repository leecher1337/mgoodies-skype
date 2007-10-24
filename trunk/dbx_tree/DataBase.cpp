#include "DataBase.h"
#include "newpluginapi.h"

CDataBase *gDataBase = NULL;


CDataBase::CDataBase(const char* FileName)
: m_Sync()
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
	if (m_Entries) delete m_Entries;
	if (m_Virtuals) delete m_Virtuals;
	if (m_Settings) delete m_Settings;

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

	/// TODO: create and show a dialog
	try 
	{
		m_SettingsFile = new CDirectAccess(m_SettingsFN);
		m_PrivateFile = new CDirectAccess(m_PrivateFN);
	}
	catch (char *)
	{
		return EMKPRF_CREATEFAILED;
	}

	memcpy(m_SettingsHeader.Signature, cSettingsSignature, sizeof(cSettingsSignature));
	m_SettingsHeader.Version = cDBVersion;
	m_SettingsHeader.Settings = 0;
	m_SettingsHeader.FileSize = sizeof(m_SettingsHeader);
	m_SettingsHeader.WastedBytes = 0;

	m_SettingsFile->Write(&m_SettingsHeader, 0, sizeof(m_SettingsHeader));
	delete m_SettingsFile;
	m_SettingsFile = NULL;

	memcpy(m_PrivateHeader.Signature, cPrivateSignature, sizeof(cPrivateSignature));
	m_PrivateHeader.Version = cDBVersion;
	m_PrivateHeader.RootEntry = 0;
	m_PrivateHeader.Entries = 0;
	m_PrivateHeader.Virtuals = 0;
	m_PrivateHeader.FileSize = sizeof(m_PrivateHeader);
	m_PrivateHeader.WastedBytes = 0;
	m_PrivateHeader.EventIndex = 1;

	m_PrivateFile->Write(&m_PrivateHeader, 0, sizeof(m_PrivateHeader));
	delete m_PrivateFile;
	m_PrivateFile = NULL;

	return 0;
}


int CDataBase::CheckDB()
{
	try 
	{
		m_SettingsFile = new CDirectAccess(m_SettingsFN);	
	} 
	catch (char * e)
	{
		return EGROKPRF_CANTREAD;
	}
	m_SettingsFile->Read(&m_SettingsHeader, 0, sizeof(m_SettingsHeader));
	delete m_SettingsFile;
	m_SettingsFile = NULL;

	if (0 != memcmp(m_SettingsHeader.Signature, cSettingsSignature, sizeof(cSettingsSignature)))
		return EGROKPRF_UNKHEADER;

	if (cDBVersion < m_SettingsHeader.Version)
		return EGROKPRF_VERNEWER;

	HANDLE htmp = CreateFile(m_PrivateFN, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
	if (htmp != INVALID_HANDLE_VALUE)
	{
		CloseHandle(htmp);

		try 
		{
			m_PrivateFile = new CDirectAccess(m_PrivateFN);
		} 
		catch (char * e)
		{
			return EGROKPRF_CANTREAD;
		}
		m_PrivateFile->Read(&m_PrivateHeader, 0, sizeof(m_PrivateHeader));
		delete m_PrivateFile;
		m_PrivateFile = NULL;

		if (0 != memcmp(m_PrivateHeader.Signature, cPrivateSignature, sizeof(cPrivateSignature)))
			return EGROKPRF_UNKHEADER;

		if (cDBVersion < m_PrivateHeader.Version)
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

	m_SettingsFile->Read(&m_SettingsHeader, 0, sizeof(m_SettingsHeader));
	m_PrivateFile->Read(&m_PrivateHeader, 0, sizeof(m_PrivateHeader));



	m_Settings = new CSettings(*m_SettingsFile, m_SettingsHeader.Settings);
	m_Settings->sigRootChanged().connect(this, &CDataBase::onSettingsRootChanged);

	m_Virtuals = new CVirtuals(*m_PrivateFile, m_PrivateHeader.Virtuals);
	m_Virtuals->sigRootChanged().connect(this, &CDataBase::onVirtualsRootChanged);

	m_Entries = new CEntries(*m_PrivateFile, m_PrivateHeader.Entries);
	m_Entries->sigRootChanged().connect(this, &CDataBase::onEntriesRootChanged);

	return 0;
}


void CDataBase::onSettingsRootChanged(void* Settings, unsigned int NewRoot)
{

}
void CDataBase::onVirtualsRootChanged(void* Virtuals, unsigned int NewRoot)
{

}
void CDataBase::onEntriesRootChanged(void* Entries, unsigned int NewRoot)
{

}