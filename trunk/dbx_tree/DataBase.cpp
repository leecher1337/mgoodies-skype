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
		m_EncryptionManager[i] = NULL;
		m_HeaderBlock[i] = 0;
	}
	
	m_Contacts = NULL;
	m_Settings = NULL;
	m_Events   = NULL;
}
CDataBase::~CDataBase()
{
	if (m_Events)   delete m_Events;
	if (m_Settings) delete m_Settings;
	if (m_Contacts) delete m_Contacts;

	m_Contacts = NULL;
	m_Settings = NULL;
	m_Events   = NULL;

	for (int i = 0; i < DBFileMax; ++i)
	{
		if (m_BlockManager[i])      delete m_BlockManager[i];
		if (m_FileAccess[i])        delete m_FileAccess[i];
		if (m_EncryptionManager[i]) delete m_EncryptionManager[i];

		m_BlockManager[i]      = NULL;
		m_FileAccess[i]        = NULL;
		m_EncryptionManager[i] = NULL;

		delete[] m_FileName[i];
	}

}
int CDataBase::CreateDB()
{
	/// TODO: create and show wizard
	if (!CreateNewFile(DBFileSetting) || 
		  !CreateNewFile(DBFilePrivate))
		return EMKPRF_CREATEFAILED;

	return 0;
}


int CDataBase::CheckFile(TDBFileType Index)
{
	TGenericFileHeader h = {0};
	DWORD r = 0;
	HANDLE htmp = CreateFile(m_FileName[Index], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
	if (htmp != INVALID_HANDLE_VALUE)
	{
		__try
		{
			SetFilePointer(htmp, 0, NULL, FILE_BEGIN);
			if (ReadFile(htmp, &h, sizeof(h), &r, NULL))
			{
				if (0 != memcmp(h.Gen.Signature, cFileSignature[Index], sizeof(cFileSignature[Index])))
					return EGROKPRF_UNKHEADER;

				if (cDBVersion < h.Gen.Version)
					return EGROKPRF_VERNEWER;
				
				
				return EGROKPRF_NOERROR;
			}
		} __finally {
			CloseHandle(htmp);
		}
	}

	return EGROKPRF_CANTREAD;
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
	m_EncryptionManager[Index] = new CEncryptionManager;

	if (GetVersion() & 0x80000000) // win98
		m_FileAccess[Index] = new CDirectAccess(m_FileName[Index], *m_EncryptionManager[Index], sizeof(m_Header[Index]));		
	else //win nt, 2000, xp...
		m_FileAccess[Index] = new CMappedMemory(m_FileName[Index], *m_EncryptionManager[Index], sizeof(m_Header[Index]));

	m_FileAccess[Index]->Read(&m_Header[Index], 0, sizeof(m_Header[Index]));
	m_EncryptionManager[Index]->InitEncryption(m_Header[Index].Gen.FileEncryption);

	m_FileAccess[Index]->SetSize(m_Header[Index].Gen.FileSize);
	m_FileAccess[Index]->sigFileSizeChanged().connect(this, &CDataBase::onFileSizeChanged);

	m_BlockManager[Index] = new CBlockManager(*m_FileAccess[Index], *m_EncryptionManager[Index]);
	m_HeaderBlock[Index] = m_BlockManager[Index]->ScanFile(sizeof(m_Header[Index]), cHeaderBlockSignature, m_Header[Index].Gen.FileSize);

	if (m_HeaderBlock[Index] == 0)			
		throwException("Header Block not found! File damaged: \"%s\"", m_FileName[Index]);
	
	TGenericFileHeader buf;
	void* pbuf = &buf;
	uint32_t size = sizeof(buf);
	uint32_t sig = cHeaderBlockSignature;
	if (!m_BlockManager[Index]->ReadBlock(m_HeaderBlock[Index], pbuf, size, sig))
		throwException("Header Block cannot be read! File damaged: \"%s\"", m_FileName[Index]);

	m_EncryptionManager[Index]->Decrypt(pbuf, size, ET_DATA, cHeaderBlockSignature, 0);

	buf.Gen.Obscure = 0;

	if (memcmp(&m_Header[Index], pbuf, size) != 0)
		throwException("Header Block in \"%s\" damaged!", m_FileName[Index]);;

	return 0;
}

int CDataBase::OpenDB()
{
  if (!PrivateFileExists())
	{
		// TODO WIZARD
		if (!CreateNewFile(DBFilePrivate))
			return -1;
	}
	
	int res = LoadFile(DBFileSetting);
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
														 *m_EncryptionManager[DBFileSetting],
														 *m_EncryptionManager[DBFilePrivate],
														 m_Sync,
														 m_Header[DBFileSetting].Set.Settings,
														 *m_Contacts);

	m_Settings->sigRootChanged().connect(this, &CDataBase::onSettingsRootChanged);
	
	m_Events = new CEvents(*m_BlockManager[DBFilePrivate],
		                     *m_EncryptionManager[DBFilePrivate],
												 m_Header[DBFilePrivate].Pri.EventLinks,
												 m_Sync,
												 *m_Contacts,
												 *m_Settings,
												 m_Header[DBFilePrivate].Pri.EventIndexCounter);

	m_Events->sigLinkRootChanged().connect(this, &CDataBase::onEventLinksRootChanged);
	m_Events->_sigIndexCounterChanged().connect(this, &CDataBase::onEventIndexCounterChanged);
	
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


bool CDataBase::CreateNewFile(TDBFileType File)
{
	try {
		TGenericFileHeader h = {0};
		memcpy(&h.Gen.Signature, &cFileSignature[File], sizeof(h.Gen.Signature));
		h.Gen.Version = cDBVersion;
		
		CEncryptionManager enc;
		CDirectAccess fa(m_FileName[File], enc, sizeof(TGenericFileHeader));
		fa.SetSize(sizeof(h));		
		CBlockManager bm(fa, enc);
		bm.ScanFile(sizeof(h), 0, sizeof(h));
		uint32_t block = bm.CreateBlock(sizeof(h), cHeaderBlockSignature);

		h.Gen.FileSize = fa.GetSize();
		bm.WriteBlock(block, &h, sizeof(h), cHeaderBlockSignature);
		fa.Write(&h, 0, sizeof(h));

	}
	catch (CException e)
	{
		return false;
	}
	return true;
}

void CDataBase::ReWriteHeader(TDBFileType Index)
{
	TGenericFileHeader h = m_Header[Index];
	uint32_t tmp[3];
	tmp[0] = GetTickCount();
	tmp[1] = GetCurrentThreadId();
	tmp[2] = _time32(NULL);

	h.Gen.Obscure = Hash(tmp, sizeof(tmp));
	m_EncryptionManager[Index]->Encrypt(&h, sizeof(h), ET_DATA, cHeaderBlockSignature, 0);
	m_BlockManager[Index]->WriteBlock(m_HeaderBlock[Index], &h, sizeof(h), cHeaderBlockSignature);

	m_FileAccess[Index]->Write(&m_Header[Index], 0, sizeof(m_Header[Index]));
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
void CDataBase::onEventLinksRootChanged(void* Events, CEventLinks::TNodeRef NewRoot)
{
	m_Header[DBFilePrivate].Pri.EventLinks = NewRoot;
	ReWriteHeader(DBFilePrivate);
}
void CDataBase::onFileSizeChanged(CFileAccess * File, uint32_t Size)
{
	if (File == m_FileAccess[DBFileSetting])
	{
		m_Header[DBFileSetting].Gen.FileSize = Size;
		ReWriteHeader(DBFileSetting);
	} else {		
		m_Header[DBFilePrivate].Gen.FileSize = Size;
		ReWriteHeader(DBFilePrivate);
	}
}

void CDataBase::onEventIndexCounterChanged(CEvents * Events, uint32_t Counter)
{
	m_Header[DBFilePrivate].Pri.EventIndexCounter = Counter;
	ReWriteHeader(DBFilePrivate);
}

int CDataBase::getProfileName(int BufferSize, char * Buffer)
{
	char * slash = strrchr(m_FileName[DBFileSetting], '\\');
	if (!slash)
		return -1;

	slash++;
	int l = strlen(slash);
	l -= 4;
	if (BufferSize < l + 1)
		return -1;

	memcpy(Buffer, slash, l);
	Buffer[l] = 0;

	return 0;
}
int CDataBase::getProfilePath(int BufferSize, char * Buffer)
{
	char * slash = strrchr(m_FileName[DBFileSetting], '\\');
	if (!slash)
		return -1;

	int l = (int)((slash - m_FileName[DBFileSetting]) / sizeof(char));
	
	if (BufferSize < l + 1)
	{
		return -1;
	}

	memcpy(Buffer, m_FileName[DBFileSetting], l);
	Buffer[l] = 0;

	return 0;
}