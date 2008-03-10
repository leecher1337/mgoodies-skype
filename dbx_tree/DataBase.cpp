#include "DataBase.h"
#include "newpluginapi.h"

CDataBase *gDataBase = NULL;


CDataBase::CDataBase(const char* FileName)
: m_Sync()
{
	m_FileName[0] = new char[strlen(FileName) + 1];
	m_FileName[1] = new char[strlen(FileName) + 5];	
	strcpy_s(m_FileName[0], strlen(FileName) + 1, FileName);
	strcpy_s(m_FileName[1], strlen(FileName) + 5, FileName);
	
	char * tmp = strrchr(m_FileName[1], '.');
	if (tmp) 
		(*tmp) = '\0';

	strcat_s(m_FileName[1], strlen(FileName) + 5, ".pri");


	m_Opened = false;

	for (int i = 0; i < DBFileMax; ++i)
	{
		m_BlockManager[i] = NULL;
		m_FileAccess[i] = NULL;
		m_Cipher[i] = NULL;
		m_HeaderBlock[i] = 0;
	}
	
	m_Contacts = NULL;
	m_Settings = NULL;
}
CDataBase::~CDataBase()
{
	if (m_Contacts) delete m_Contacts;
	if (m_Settings) delete m_Settings;

	m_Contacts = NULL;
	m_Settings = NULL;

	for (int i = 0; i < 2; ++i)
	{
		if (m_BlockManager[i]) delete m_BlockManager[i];
		if (m_FileAccess[i]) delete m_FileAccess[i];
		m_BlockManager[i] = NULL;
		m_FileAccess[i] = NULL;		

		delete[] m_FileName[i];
	}

}
int CDataBase::CreateDB()
{

/*
	/// TODO: create and show wizard
	try 
	{
		m_FileAccessSet = new CDirectAccess(m_SettingsFN);
		m_FileAccessPri = new CDirectAccess(m_PrivateFN);
	}
	catch (char *)
	{
		return EMKPRF_CREATEFAILED;
	}
*/
	return 0;
}


int CDataBase::CheckFile(TDBFileType Index)
{
	try 
	{
		m_FileAccess[Index] = new CDirectAccess(m_FileName[Index]);	
	} 
	catch (char * )
	{
		return EGROKPRF_CANTREAD;
	}
	m_FileAccess[0]->Read(&m_Header[Index], 0, sizeof(m_Header[Index]));
	delete m_FileAccess[Index];
	m_FileAccess[Index] = NULL;

	if (0 != memcmp(m_Header[Index].Gen.Signature, cFileSignature[Index], sizeof(cFileSignature[Index])))
		return EGROKPRF_UNKHEADER;

	if (cDBVersion < m_Header[Index].Gen.Version)
		return EGROKPRF_VERNEWER;

	return EGROKPRF_NOERROR;
}

int CDataBase::CheckDB()
{
	int res = CheckFile(DBFileSetting);

	if (res != EGROKPRF_NOERROR)
		return res;

	
	if (PrivateFileExists())
		res = CheckFile(DBFilePrivate);
	
	return res;
}

int CDataBase::LoadFile(TDBFileType Index)
{
	try 
	{
		if (GetVersion() & 0x80000000) // win98
			m_FileAccess[Index] = new CDirectAccess(m_FileName[Index]);		
		else //win nt, 2000, xp.. usw		
			m_FileAccess[Index] = new CMappedMemory(m_FileName[Index]);
	}
	catch (char * )
	{
		return GetLastError();
	}

	m_FileAccess[Index]->SetEncryptionStart(sizeof(m_Header[Index]));
	m_FileAccess[Index]->Read(&m_Header[Index], 0, sizeof(m_Header[Index]));
	m_FileAccess[Index]->SetSize(m_Header[Index].Gen.FileSize);

	try
	{
		m_Cipher[Index] = MakeCipher(m_Header[Index].Gen.FileAccess);
	}
	catch (char *)
	{
		return -1;
	}

	m_BlockManager[Index] = new CBlockManager(*m_FileAccess[Index]);

	if ((m_Header[Index].Gen.FileAccess & cDBFAEncryptedMask) == cDBFAEncryptFull)
		m_FileAccess[Index]->SetCipher(m_Cipher[Index]);

	if ((m_Header[Index].Gen.FileAccess & cDBFAEncryptedMask) == cDBFAEncryptBlocks)
		m_BlockManager[Index]->SetCipher(m_Cipher[Index]);

	/// TODO ask password
	m_HeaderBlock[Index] = m_BlockManager[Index]->ScanFile(sizeof(m_Header[Index]), cHeaderBlockSignature);

	if (m_HeaderBlock[Index] == 0)			
		return -1;
	
	TGenericFileHeader buf;
	void* pbuf = &buf;
	uint32_t size = sizeof(buf);
	uint32_t sig = cHeaderBlockSignature;
	if (!m_BlockManager[Index]->ReadBlock(m_HeaderBlock[Index], pbuf, size, sig))
		return -1;

	if ((m_Header[Index].Gen.FileAccess & cDBFAEncryptedMask) == cDBFAEncryptHistory)
		m_Cipher[Index]->Decrypt(pbuf, size, cHeaderBlockSignature);

	if (memcmp(&m_Header[Index], pbuf, size) != 0)
		return -2; /// TODO go back and ask pw again

	return 0;
}

int CDataBase::OpenDB()
{
  if (!PrivateFileExists())
	{
		if (!CreateNewPrivateFile())
			return -1;
	}
	
	int res = LoadFile(DBFileSetting) ;
	if (res != 0) return res;

	res = LoadFile(DBFilePrivate);
	if (res != 0) return res;
	
	m_Contacts = new CContacts(*m_BlockManager[DBFilePrivate],
		                       m_Sync, 
													 m_Header[DBFilePrivate].Pri.RootContact, 
													 m_Header[DBFilePrivate].Pri.Contacts, 
													 m_Header[DBFilePrivate].Pri.Virtuals);

	m_Contacts->sigRootChanged().connect(this, &CDataBase::onContactsRootChanged);
	m_Contacts->sigVirtualRootChanged().connect(this, &CDataBase::onVirtualsRootChanged);
	
	if (m_Contacts->getRootContact() != m_Header[DBFilePrivate].Pri.RootContact)
	{
		m_Header[DBFilePrivate].Pri.RootContact = m_Contacts->getRootContact();
		ReWriteHeader(DBFilePrivate);
	}

	m_Settings = new CSettings(*m_BlockManager[DBFileSetting],
		                         *m_BlockManager[DBFilePrivate],
														 m_Sync,
														 m_Header[DBFileSetting].Set.Settings,
														 *m_Contacts);

	m_Settings->sigRootChanged().connect(this, &CDataBase::onSettingsRootChanged);


	return 0;
}

bool CDataBase::PrivateFileExists()
{
	HANDLE htmp = CreateFile(m_FileName[DBFilePrivate], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
	if (htmp != INVALID_HANDLE_VALUE)
	{
		CloseHandle(htmp);
		return true;
	}

	return false;
}


bool CDataBase::CreateNewPrivateFile()
{
	/// TODO Wizard
	return true;
}

CCipher * CDataBase::MakeCipher(uint32_t Access)
{
	int i;
	if (Access & cDBFAEncryptedMask)
	{
		i = 0;
		while (i < sizeof(cCipherList) / sizeof(cCipherList[0]))
		{
			if (cCipherList[i].ID == (Access & cDBFAEncryptMethodMask))
				return cCipherList[i].Create();

			++i;
		}	
		
		throw "CipherID not present!";
	}

	return NULL;
}

void CDataBase::ReWriteHeader(TDBFileType Index)
{ 
	TGenericFileHeader h = m_Header[Index];

	m_FileAccess[Index]->Write(&m_Header[Index], 0, sizeof(m_Header[Index]));
	
	if ((m_Header[Index].Gen.FileAccess & cDBFAEncryptedMask) == cDBFAEncryptHistory)		
		m_Cipher[Index]->Encrypt(&h, sizeof(h), cHeaderBlockSignature);
	
	m_BlockManager[Index]->WriteBlock(m_HeaderBlock[Index], &h, sizeof(h), cHeaderBlockSignature);
}


void CDataBase::onSettingsRootChanged(CSettings* Settings, CSettingsTree::TNodeRef NewRoot)
{
	m_Header[DBFileSetting].Set.Settings = NewRoot;
	ReWriteHeader(DBFileSetting);
}
void CDataBase::onVirtualsRootChanged(void* Virtuals, CVirtuals::TNodeRef NewRoot)
{	
	m_Header[DBFilePrivate].Pri.Virtuals = NewRoot;
	ReWriteHeader(DBFilePrivate);
}
void CDataBase::onContactsRootChanged(void* Contacts, CContacts::TNodeRef NewRoot)
{
	m_Header[DBFilePrivate].Pri.Contacts = NewRoot;
	ReWriteHeader(DBFilePrivate);
}

CContacts & CDataBase::getContacts()
{
	return *m_Contacts;
}
CSettings & CDataBase::getSettings()
{
	return *m_Settings;
}