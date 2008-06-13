#include "Settings.h"
#include <math.h> // floor function
#include "Hash.h"

inline bool TSettingKey::operator <  (const TSettingKey & Other) const
{
	return Hash < Other.Hash;	
}

inline bool TSettingKey::operator == (const TSettingKey & Other) const
{
	return (Hash == Other.Hash);
}

inline bool TSettingKey::operator >  (const TSettingKey & Other) const
{	
	return Hash > Other.Hash;
}



CSettingsTree::CSettingsTree(CSettings & Owner, CBlockManager & BlockManager, CEncryptionManager & EncryptionManager, TNodeRef RootNode, TDBTContactHandle Contact)
: CFileBTree(BlockManager, RootNode, cSettingNodeSignature),
	m_Owner(Owner),
	m_EncryptionManager(EncryptionManager)
{
	m_Contact = Contact;
}

CSettingsTree::~CSettingsTree()
{

}

inline TDBTContactHandle CSettingsTree::getContact()
{
	return m_Contact;
}

void CSettingsTree::setContact(TDBTContactHandle NewContact)
{
	m_Contact = NewContact;
}

TDBTSettingHandle CSettingsTree::_FindSetting(const uint32_t Hash, const char * Name, const uint32_t Length)
{
	TSettingKey key;
	key.Hash = Hash;
	iterator i = Find(key);
	uint16_t l;
	
	TDBTSettingHandle res = 0;

	char * str = NULL;

	while ((res == 0) && (i) && (i.Key().Hash == Hash))
	{
		l = Length;
		if (m_Owner._ReadSettingName(m_BlockManager, m_EncryptionManager, i.Data(), l, str) &&
			(strncmp(str, Name, Length) == 0))
		{
			res = i.Data();
		} else {
			++i;
		}
	} 

	if (str) 
		free(str);

	return res;
}

bool CSettingsTree::_DeleteSetting(const uint32_t Hash, const TDBTSettingHandle hSetting)
{
	TSettingKey key;
	key.Hash = Hash;
	iterator i = Find(key);

	while ((i) && (i.Key().Hash == Hash) && (i.Data() != hSetting))
		++i;

	if ((i) && (i.Key().Hash == Hash))
	{
		Delete(i);
		return true;
	}
	
	return false;
}

bool CSettingsTree::_AddSetting(const uint32_t Hash, const TDBTSettingHandle hSetting)
{
	TSettingKey key;
	key.Hash = Hash;
	Insert(key, hSetting);
	return true;
}

CSettings::CSettings(
		CBlockManager & BlockManagerSet,
		CBlockManager & BlockManagerPri,
		CEncryptionManager & EncryptionManagerSet,
		CEncryptionManager & EncryptionManagerPri,
		CMultiReadExclusiveWriteSynchronizer & Synchronize, 
		CSettingsTree::TNodeRef SettingsRoot,
		CContacts & Contacts
)
:	m_Sync(Synchronize),
	m_BlockManagerSet(BlockManagerSet),
	m_BlockManagerPri(BlockManagerPri),
	m_EncryptionManagerSet(EncryptionManagerSet),
	m_EncryptionManagerPri(EncryptionManagerPri),
	m_Contacts(Contacts),
	m_SettingsMap(),
	m_sigRootChanged(),
	m_Modules()
{
	CSettingsTree * settree = new CSettingsTree(*this, m_BlockManagerSet, m_EncryptionManagerSet, SettingsRoot, 0);

	settree->sigRootChanged().connect(this, &CSettings::onRootChanged);
	m_SettingsMap.insert(std::make_pair(0, settree));

	m_Contacts._sigDeleteSettings().connect(this, &CSettings::onDeleteSettings);
	m_Contacts._sigMergeSettings().connect (this, &CSettings::onMergeSettings);

	_LoadModules();
	_EnsureModuleExists("$Modules");
}

CSettings::~CSettings()
{
	SYNC_BEGINWRITE(m_Sync);

	TSettingsTreeMap::iterator it = m_SettingsMap.begin();

	while (it != m_SettingsMap.end())
	{
		delete it->second;
		++it;
	}

	m_SettingsMap.clear();

	TModulesMap::iterator it2 = m_Modules.begin();
	while (it2 != m_Modules.end())
	{
		if (it2->second)
			delete [] it2->second;
		++it2;
	}

	SYNC_ENDWRITE(m_Sync);
}


CSettingsTree * CSettings::getSettingsTree(TDBTContactHandle hContact)
{
	TSettingsTreeMap::iterator i = m_SettingsMap.find(hContact);
	if (i != m_SettingsMap.end())
		return i->second;

	uint32_t root = m_Contacts._getSettingsRoot(hContact);
	if (root == DBT_INVALIDPARAM)
		return NULL;

	CSettingsTree * tree = new CSettingsTree(*this, m_BlockManagerPri, m_EncryptionManagerPri, root, hContact);
	tree->sigRootChanged().connect(this, &CSettings::onRootChanged);
	m_SettingsMap.insert(std::make_pair(hContact, tree));

	return tree;	
}

inline bool CSettings::_ReadSettingName(CBlockManager & BlockManager, CEncryptionManager & EncryptionManager, TDBTSettingHandle Setting, uint16_t & NameLength, char *& NameBuf)
{
	uint32_t sig = cSettingSignature;
	uint16_t len;
	char * buf;

	if (!BlockManager.ReadPart(Setting, &len, offsetof(TSetting, NameLength), sizeof(len), sig))
		return false;

	if ((NameLength != 0) && (NameLength != len))
		return false;

	NameLength = len;
	NameBuf = (char*) realloc(NameBuf, NameLength + 1);

	if (EncryptionManager.IsEncrypted(Setting, ET_DATA))
	{
		len = EncryptionManager.AlignSize(Setting, ET_DATA, sizeof(((TSetting*)NULL)->Value) + NameLength + 1);
		buf = (char*) malloc(len);
		__try
		{
			BlockManager.ReadPart(Setting, buf, offsetof(TSetting, Value), len, sig);
			EncryptionManager.Decrypt(buf, len, ET_DATA, Setting, 0);
			memcpy(NameBuf, buf + sizeof(((TSetting*)NULL)->Value), NameLength + 1);
		}
		__finally 
		{
			free(buf);
		}
	} else {	
		BlockManager.ReadPart(Setting, NameBuf, sizeof(TSetting), NameLength + 1, sig);
	}

	NameBuf[NameLength] = 0;

	return true;
}

void CSettings::_EnsureModuleExists(char * Module)
{
	if ((Module == NULL) || (*Module == 0))
		return;

	char * e = strchr(Module, '/');
	if (e)
		*e = 0;

	TModulesMap::iterator i = m_Modules.find(*((uint16_t*)Module));
	while ((i != m_Modules.end()) && (i->first == *((uint16_t*)Module)) && (strcmp(i->second, Module) != 0))
	{
		++i;
	}

	if ((i == m_Modules.end()) || (i->first != *((uint16_t*)Module)))
	{
		unsigned int l = strlen(Module);
		char * tmp = new char [l + 1];
		memcpy(tmp, Module, l + 1);
		m_Modules.insert(std::make_pair(*((uint16_t *) tmp), tmp));

		char namebuf[512];
		strcpy_s(namebuf, "$Modules/");
		strcat_s(namebuf, Module);

		TDBTSettingDescriptor desc = {0};
		desc.cbSize = sizeof(desc);
		desc.pszSettingName = namebuf;

		TDBTSetting set = {0};
		set.cbSize = sizeof(set);
		set.Descriptor = &desc;
		set.Type = DBT_ST_DWORD;
		
		WriteSetting(set, cSettingsFileFlag);
	}

	if (e)
		*e = '/';
}

void CSettings::_LoadModules()
{
	TDBTSettingDescriptor desc = {0};
	desc.cbSize = sizeof(desc);

	TDBTSettingIterFilter f = {0};
	f.cbSize = sizeof(f);
	f.Descriptor = &desc;
	f.NameStart = "$Modules/";
	
	TDBTSettingIterationHandle hiter = IterationInit(f);
	
	if ((hiter != 0) && (hiter != DBT_INVALIDPARAM))
	{
		TDBTSettingHandle res = IterationNext(hiter);
		while ((res != 0) && (res != DBT_INVALIDPARAM))
		{
			unsigned int l = strlen(desc.pszSettingName);
			char * tmp = new char [l - 9 + 1];
			memcpy(tmp, desc.pszSettingName + 9, l - 9 + 1);
			m_Modules.insert(std::make_pair(*((uint16_t *) tmp), tmp));
			res = IterationNext(hiter);
		}

		IterationClose(hiter);
	}
}

CSettings::TOnRootChanged & CSettings::sigRootChanged()
{
	return m_sigRootChanged;
}
void CSettings::onRootChanged(void* SettingsTree, CSettingsTree::TNodeRef NewRoot)
{
	if (((CSettingsTree*)SettingsTree)->getContact() == 0)
		m_sigRootChanged.emit(this, NewRoot);
	else 
		m_Contacts._setSettingsRoot(((CSettingsTree*)SettingsTree)->getContact(), NewRoot);
}

void CSettings::onDeleteSettingCallback(void * Tree, TSettingKey Key, TDBTSettingHandle Data, uint32_t Param)
{
	if (Param == 0)
	{
		m_BlockManagerSet.DeleteBlock(Data);
	} else {
		m_BlockManagerPri.DeleteBlock(Data);
	}
}
void CSettings::onDeleteSettings(CContacts * Contacts, TDBTContactHandle hContact)
{
	CSettingsTree * tree = getSettingsTree(hContact);

	m_Contacts._setSettingsRoot(hContact, 0);

	if (tree)
	{
		CSettingsTree::TDeleteCallback callback;
		callback.connect(this, &CSettings::onDeleteSettingCallback);

		tree->DeleteTree(&callback, hContact);

		TSettingsTreeMap::iterator i = m_SettingsMap.find(hContact);
		delete i->second; // tree
		m_SettingsMap.erase(i);
	}
}


typedef struct TSettingMergeHelper 
{
	TDBTContactHandle Source;
	TDBTContactHandle Dest;
	CSettingsTree * SourceTree;



} TSettingMergeHelper, *PSettingMergeHelper;


void CSettings::onMergeSettingCallback(void * Tree, TSettingKey Key, TDBTSettingHandle Data, uint32_t Param)
{
	PSettingMergeHelper hlp = (PSettingMergeHelper)Param;

	uint16_t dnl = 0;
	char * dnb = NULL;
		
	_ReadSettingName(m_BlockManagerPri, m_EncryptionManagerPri, Data, dnl, dnb);

	CSettingsTree::iterator i = hlp->SourceTree->Find(Key);
	TDBTSettingHandle res = 0;
	while (i && (i.Key() == Key) && (res == 0))
	{
		uint16_t snl = dnl;
		char * snb = NULL;
		
		if (_ReadSettingName(m_BlockManagerPri, m_EncryptionManagerPri, i.Data(), snl, snb)
			  && (strcmp(dnb, snb) == 0)) // found it
		{
			res = i.Data();
		}
	}

	if (res == 0)
	{
		hlp->SourceTree->Insert(Key, Data);
	} else {
		i.SetData(Data);
		m_BlockManagerPri.DeleteBlock(res);
	}
}

void CSettings::onMergeSettings(CContacts * Contacts, TDBTContactHandle Source, TDBTContactHandle Dest)
{
	if ((Source == 0) || (Dest == 0))
		throwException("Cannot Merge with global settings!\nSource %d Dest %d", Source, Dest);

	CSettingsTree * stree = getSettingsTree(Source);
	CSettingsTree * dtree = getSettingsTree(Dest);

	if (stree && dtree)
	{
		m_Contacts._setSettingsRoot(Source, 0);

		stree->setContact(Dest);
		m_Contacts._setSettingsRoot(Dest, stree->getRoot());

		TSettingKey key = {0};
		CSettingsTree::iterator it = stree->LowerBound(key);

		while (it) // transfer all source settings to new contact
		{
			m_BlockManagerPri.WritePart(it.Data(), &Dest, offsetof(TSetting, Contact), sizeof(Dest));
			++it;
		}

		// merge the dest tree into the source tree. override existing items
		// do it this way, because source tree should be much larger
		TSettingMergeHelper hlp;
		hlp.Source = Source;
		hlp.Dest = Dest;
		hlp.SourceTree = stree;

		CSettingsTree::TDeleteCallback callback;
		callback.connect(this, &CSettings::onMergeSettingCallback);
		dtree->DeleteTree(&callback, (uint32_t)&hlp);

		TSettingsTreeMap::iterator i = m_SettingsMap.find(Dest);
		delete i->second; // dtree
		i->second = stree;
		m_SettingsMap.erase(Source);
		
	}
}




TDBTSettingHandle CSettings::FindSetting(TDBTSettingDescriptor & Descriptor)
{
	if (Descriptor.Flags & DBT_SDF_FoundValid)
		return Descriptor.FoundHandle;

	uint32_t namelength = strlen(Descriptor.pszSettingName);
	uint32_t namehash;
	
	if (Descriptor.Flags & DBT_SDF_HashValid)
	{
		namehash = Descriptor.Hash;
	} else {
		namehash = Hash(Descriptor.pszSettingName, namelength);
		Descriptor.Hash = namehash;
		Descriptor.Flags = Descriptor.Flags | DBT_SDF_HashValid;
	}

	Descriptor.Flags = Descriptor.Flags & ~DBT_SDF_FoundValid;

	CSettingsTree * tree;
	TDBTSettingHandle res = 0;

	SYNC_BEGINREAD(m_Sync);

	if ((Descriptor.Contact == 0) || (Descriptor.Options == 0))
	{
		tree = getSettingsTree(Descriptor.Contact);
		if (tree == NULL)
		{
			SYNC_ENDREAD(m_Sync);
			return DBT_INVALIDPARAM;
		}

		res = tree->_FindSetting(namehash, Descriptor.pszSettingName, namelength);
	
		if (res)
		{
			Descriptor.FoundInContact = Descriptor.Contact;
			Descriptor.FoundHandle = res;
			Descriptor.Flags = Descriptor.Flags | DBT_SDF_FoundValid;
		}

		SYNC_ENDREAD(m_Sync);

		if (Descriptor.Contact == 0)
			res = res | cSettingsFileFlag;

		return res;
	}

	uint32_t cf = m_Contacts.getFlags(Descriptor.Contact);
	if (cf == DBT_INVALIDPARAM)
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}
	
	// search the setting
	res = 0;
	
	TDBTContactIterFilter f;
	f.cbSize = sizeof(f);
	if (cf & DBT_CF_IsGroup)
	{
		f.fDontHasFlags = 0;
		f.fHasFlags = DBT_CF_IsGroup;
	} else {
		f.fDontHasFlags = DBT_CF_IsGroup;
		f.fHasFlags = 0;	
	}
	f.Options = Descriptor.Options;

	TDBTContactIterationHandle i = m_Contacts.IterationInit(f, Descriptor.Contact);
	if ((i == DBT_INVALIDPARAM) || (i == 0))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	TDBTContactHandle e = m_Contacts.IterationNext(i);
	TDBTContactHandle found = 0;
	while ((res == 0) && (e != 0))
	{
		tree = getSettingsTree(e);
		if (tree)
		{
			res = tree->_FindSetting(namehash, Descriptor.pszSettingName, namelength);
			found = e;
		}

		e = m_Contacts.IterationNext(i);
	}

	m_Contacts.IterationClose(i);

	if (res)
	{
		Descriptor.FoundInContact = found;
		Descriptor.FoundHandle = res;
		Descriptor.Flags = Descriptor.Flags | DBT_SDF_FoundValid;
	}

	SYNC_ENDREAD(m_Sync);

	return res;
}
unsigned int CSettings::DeleteSetting(TDBTSettingDescriptor & Descriptor)
{
	SYNC_BEGINWRITE(m_Sync);

	TDBTSettingHandle hset = FindSetting(Descriptor);
	if ((hset == 0) || (hset == DBT_INVALIDPARAM))
	{		
		SYNC_ENDWRITE(m_Sync);
		return hset;
	}

	unsigned int res = 0;
	if ((Descriptor.Flags & DBT_SDF_FoundValid) && (Descriptor.Flags & DBT_SDF_HashValid))
	{
		CBlockManager * file = &m_BlockManagerPri;
		TDBTContactHandle con;
		uint32_t sig = cSettingSignature;

		if (Descriptor.FoundInContact == 0)
		{
			file = &m_BlockManagerSet;
			hset = hset & ~cSettingsFileFlag;
		}
		
		if (file->ReadPart(hset, &con, offsetof(TSetting, Contact), sizeof(con), sig) &&
			(con == Descriptor.FoundInContact))
		{
			CSettingsTree * tree = getSettingsTree(con);
			if (tree)
			{
				tree->_DeleteSetting(Descriptor.Hash, hset);
				file->DeleteBlock(hset);
			}
		}

	} else {
		res = DeleteSetting(hset);
	}

	SYNC_ENDWRITE(m_Sync);
	
	return res;
}
unsigned int CSettings::DeleteSetting(TDBTSettingHandle hSetting)
{
	SYNC_BEGINWRITE(m_Sync);
	
	CBlockManager * file = &m_BlockManagerPri;
	CEncryptionManager * enc = &m_EncryptionManagerPri;

	if (hSetting & cSettingsFileFlag)
	{
		file = &m_BlockManagerSet;
		enc = &m_EncryptionManagerSet;
		hSetting = hSetting & ~cSettingsFileFlag;
	}

	void* buf = NULL;
	uint32_t size = 0;
	uint32_t sig = cSettingSignature;
	
	if (!file->ReadBlock(hSetting, buf, size, sig))
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	TSetting* set = (TSetting*)buf;

	CSettingsTree * tree = getSettingsTree(set->Contact);
	if (tree == NULL)
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	// encryption
	if (enc->IsEncrypted(hSetting, ET_DATA))
	{
		enc->Decrypt(&(set->Value), enc->AlignSize(hSetting, ET_DATA, sizeof(set->Value) + set->NameLength + 1), ET_DATA, hSetting, 0);
	}

	char * str = (char*) (set+1);
	tree->_DeleteSetting(Hash(str, set->NameLength), hSetting);
	
	file->DeleteBlock(hSetting);
	SYNC_ENDWRITE(m_Sync);

	free(buf);
	return 0;
}
TDBTSettingHandle CSettings::WriteSetting(TDBTSetting & Setting)
{
	SYNC_BEGINWRITE(m_Sync);

	TDBTSettingHandle hset = FindSetting(*Setting.Descriptor);
	if (hset == DBT_INVALIDPARAM)
	{		
		SYNC_ENDWRITE(m_Sync);
		return hset;
	}

	hset = WriteSetting(Setting, hset);

	SYNC_ENDWRITE(m_Sync);
	
	return hset;
}
TDBTSettingHandle CSettings::WriteSetting(TDBTSetting & Setting, TDBTSettingHandle hSetting)
{
	if (!hSetting && !(Setting.Descriptor && Setting.Descriptor->Contact))
		return DBT_INVALIDPARAM;

	SYNC_BEGINWRITE(m_Sync);
	
	CBlockManager * file = &m_BlockManagerPri;
	CEncryptionManager * enc = &m_EncryptionManagerPri;
	bool fileflag = false;
	uint32_t sig = cSettingSignature;

	if (hSetting & cSettingsFileFlag)
	{
		file = &m_BlockManagerSet;
		enc = &m_EncryptionManagerSet;
		hSetting = hSetting & ~cSettingsFileFlag;
		fileflag = true;
	}

	CSettingsTree * tree = NULL;
	
	if (hSetting == 0)
	{
		if (Setting.Descriptor->Contact == 0)
		{
			file = &m_BlockManagerSet;
			enc = &m_EncryptionManagerSet;
			fileflag = true;
		}

		if ((Setting.Descriptor) && (Setting.Descriptor->pszSettingName)) // setting needs a name
		{
			tree = getSettingsTree(Setting.Descriptor->Contact);
			_EnsureModuleExists(Setting.Descriptor->pszSettingName);
		}
		
	} else {		
		TDBTContactHandle e;
		if (file->ReadPart(hSetting, &e, offsetof(TSetting, Contact), sizeof(e), sig)) // check if hSetting is valid
			tree = getSettingsTree(e);
	}

	if (tree == NULL)
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	uint8_t * buf;
	TSetting * set = NULL;
	uint32_t blobsize = 0;
	uint32_t blocksize = 0;

	if (Setting.Type & DBT_STF_VariableLength)
	{		
		switch (Setting.Type)
		{
			case DBT_ST_ASCIIZ: case DBT_ST_UTF8:
				{
					if (Setting.Value.Length == 0)
						blobsize = strlen(Setting.Value.pAnsii) + 1;
					else
						blobsize = Setting.Value.Length;
				} break;
			case DBT_ST_WCHAR:
				{
					if (Setting.Value.Length == 0)
						blobsize = sizeof(wchar_t) * (wcslen(Setting.Value.pWide) + 1);
					else
						blobsize = sizeof(wchar_t) * (Setting.Value.Length);
				} break;
			default:
				blobsize = Setting.Value.Length;
				break;
		}
	}	

	blocksize = sizeof(TSetting) - sizeof(set->Value) + enc->AlignSize(hSetting, ET_DATA, sizeof(set->Value) + strlen(Setting.Descriptor->pszSettingName) + 1 + blobsize);
	
	if (hSetting == 0) // create new setting
	{	
		hSetting = file->CreateBlock(blocksize, cSettingSignature);

		if (blocksize != sizeof(TSetting) - sizeof(set->Value) + enc->AlignSize(hSetting, ET_DATA, sizeof(set->Value) + strlen(Setting.Descriptor->pszSettingName) + 1 + blobsize))
		{
			blocksize = sizeof(TSetting) - sizeof(set->Value) + enc->AlignSize(hSetting, ET_DATA, sizeof(set->Value) + strlen(Setting.Descriptor->pszSettingName) + 1 + blobsize);
			file->ResizeBlock(hSetting, blocksize, false);
		}

		buf = (uint8_t *)malloc(blocksize);
		set = (TSetting*)buf;
		memset(&(set->Reserved), 0, sizeof(set->Reserved));

		set->Contact = Setting.Descriptor->Contact;
		set->Flags = 0;
		set->AllocSize = blobsize;

		if (Setting.Descriptor && (Setting.Descriptor->Flags & DBT_SDF_HashValid))
		{
			tree->_AddSetting(Setting.Descriptor->Hash, hSetting);
		} else  {
			tree->_AddSetting(Hash(Setting.Descriptor->pszSettingName, strlen(Setting.Descriptor->pszSettingName)), hSetting);
		}

	} else {

		buf = (uint8_t *)malloc(blocksize);
		set = (TSetting*)buf;
		memset(&(set->Reserved), 0, sizeof(set->Reserved));

		file->ReadPart(hSetting, set, 0, sizeof(TSetting), sig);
		if (((Setting.Type & DBT_STF_VariableLength) == 0) && (set->Type & DBT_STF_VariableLength))
		{ // shrink setting (variable size->fixed size)
			file->ResizeBlock(hSetting, blocksize);
		}

		if ((Setting.Type & DBT_STF_VariableLength) && ((set->Type & DBT_STF_VariableLength) == 0))
		{ // trick it
			set->AllocSize = 0;
		}
	}

	set->Type = Setting.Type;
	set->NameLength = strlen(Setting.Descriptor->pszSettingName);
	memcpy(set + 1, Setting.Descriptor->pszSettingName, set->NameLength + 1);
	
	if (Setting.Type & DBT_STF_VariableLength)
	{
		if (set->AllocSize < blobsize)
		{			
			set->AllocSize = file->ResizeBlock(hSetting, blocksize) - 
				                (sizeof(TSetting) + set->NameLength + 1);
		}

		set->BlobLength = blobsize;
		
		memcpy(buf + sizeof(TSetting) + set->NameLength + 1, Setting.Value.pBlob, blobsize);
	} else {
		memset(&(set->Value), 0, sizeof(set->Value));
		switch (Setting.Type)
		{
			case DBT_ST_BOOL:
				set->Value.Bool = Setting.Value.Bool; break;
			case DBT_ST_BYTE: case DBT_ST_CHAR:
				set->Value.Byte = Setting.Value.Byte; break;
			case DBT_ST_SHORT: case DBT_ST_WORD:
				set->Value.Short = Setting.Value.Short; break;
			case DBT_ST_INT: case DBT_ST_DWORD:
				set->Value.Int = Setting.Value.Int; break;
			default:
				set->Value = Setting.Value; break;
		}
	}

	if (enc->IsEncrypted(hSetting, ET_DATA))
	{
		enc->Encrypt(&(set->Value), enc->AlignSize(hSetting, ET_DATA, sizeof(set->Value) + set->NameLength + 1 + set->AllocSize), ET_DATA, hSetting, 0);
	}

	file->WriteBlock(hSetting, buf, blocksize, cSettingSignature);
	free(buf);

	if (fileflag)
	{
		hSetting = hSetting | cSettingsFileFlag;
	}
	
	SYNC_ENDWRITE(m_Sync);

	return hSetting;
}
unsigned int CSettings::ReadSetting(TDBTSetting & Setting)
{
	SYNC_BEGINREAD(m_Sync);

	TDBTSettingHandle hset = FindSetting(*Setting.Descriptor);
	if ((hset == 0) || (hset == DBT_INVALIDPARAM))
	{		
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	PDBTSettingDescriptor back = Setting.Descriptor;
	Setting.Descriptor = NULL;
	if (ReadSetting(Setting, hset) == DBT_INVALIDPARAM)
		hset = DBT_INVALIDPARAM;

	Setting.Descriptor = back;

	SYNC_ENDREAD(m_Sync);

	return hset;
}
unsigned int CSettings::ReadSetting(TDBTSetting & Setting, TDBTSettingHandle hSetting)
{
	CBlockManager * file = &m_BlockManagerPri;
	CEncryptionManager * enc = &m_EncryptionManagerPri;

	if (hSetting & cSettingsFileFlag)
	{
		file = &m_BlockManagerSet;
		enc = &m_EncryptionManagerSet;
		hSetting = hSetting & ~cSettingsFileFlag;
	}

	void* buf = NULL;
	uint32_t size = 0;
	uint32_t sig = cSettingSignature;

	if (hSetting == 0)
		return DBT_INVALIDPARAM;


	SYNC_BEGINREAD(m_Sync);

	if (!file->ReadBlock(hSetting, buf, size, sig))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	SYNC_ENDREAD(m_Sync);

	TSetting* set = (TSetting*)buf;
	uint8_t* str = (uint8_t*)buf + sizeof(TSetting) + set->NameLength + 1;

	if (enc->IsEncrypted(hSetting, ET_DATA))
	{
		enc->Decrypt(&(set->Value), enc->AlignSize(hSetting, ET_DATA, size - sizeof(TSetting) + sizeof(set->Value)), ET_DATA, hSetting, 0);
	}

	if (Setting.Type == 0)
	{
		Setting.Type = set->Type;
		if (set->Type & DBT_STF_VariableLength)
		{
			Setting.Value.Length = set->BlobLength;
			switch (set->Type)
			{
				case DBT_ST_WCHAR:
				{
					Setting.Value.Length = set->BlobLength / sizeof(wchar_t);
					Setting.Value.pWide = (wchar_t*) mir_realloc(Setting.Value.pWide, sizeof(wchar_t) * Setting.Value.Length);
					memcpy(Setting.Value.pWide, str, set->BlobLength);
					Setting.Value.pWide[Setting.Value.Length - 1] = 0;
					
				} break;
				case DBT_ST_ASCIIZ: case DBT_ST_UTF8:
				{
					Setting.Value.Length = set->BlobLength;
					Setting.Value.pAnsii = (char *) mir_realloc(Setting.Value.pAnsii, set->BlobLength);
					memcpy(Setting.Value.pAnsii, str, set->BlobLength);					
					Setting.Value.pAnsii[Setting.Value.Length - 1] = 0;
					
				} break;
				default:
				{
					Setting.Value.Length = set->BlobLength;
					Setting.Value.pBlob = (uint8_t *) mir_realloc(Setting.Value.pBlob, set->BlobLength);
					memcpy(Setting.Value.pBlob, str, set->BlobLength);					
				} break;
			}
		} else {
			Setting.Value = set->Value;
		}
	} else {
		switch (set->Type)
		{
			case DBT_ST_BYTE: case DBT_ST_WORD: case DBT_ST_DWORD: case DBT_ST_QWORD:
				{
					switch (Setting.Type)
					{
						case DBT_ST_BYTE:  Setting.Value.Byte  = (uint8_t)   set->Value.QWord; break;
						case DBT_ST_WORD:  Setting.Value.Word  = (uint16_t)  set->Value.QWord; break;
						case DBT_ST_DWORD: Setting.Value.DWord = (uint32_t)  set->Value.QWord; break;
						case DBT_ST_QWORD: Setting.Value.QWord = (uint64_t)  set->Value.QWord; break;
						case DBT_ST_CHAR:  Setting.Value.Char  = ( int8_t)   set->Value.QWord; break;
						case DBT_ST_SHORT: Setting.Value.Short = ( int16_t)  set->Value.QWord; break;
						case DBT_ST_INT:   Setting.Value.Int   = ( int32_t)  set->Value.QWord; break;
						case DBT_ST_INT64: Setting.Value.Int64 = ( int64_t)  set->Value.QWord; break;
						case DBT_ST_BOOL:  Setting.Value.Bool  = set->Value.QWord != 0; break;

						case DBT_ST_ASCIIZ: case DBT_ST_UTF8:
							{
								char buffer[24];
								buffer[0] = 0;
								sprintf_s(buffer, 24, "%llu", set->Value.QWord);
								Setting.Value.Length = strlen(buffer) + 1;
								Setting.Value.pAnsii = (char *) mir_realloc(Setting.Value.pAnsii, Setting.Value.Length);
								memcpy(Setting.Value.pAnsii, buffer, Setting.Value.Length);
													
							} break;
						case DBT_ST_WCHAR:
							{
								wchar_t buffer[24];
								buffer[0] = 0;
								swprintf_s(buffer, 24, L"%llu", set->Value.QWord);
								Setting.Value.Length = wcslen(buffer) + 1;
								Setting.Value.pWide = (wchar_t *) mir_realloc(Setting.Value.pWide, Setting.Value.Length * sizeof(wchar_t));
								memcpy(Setting.Value.pWide, buffer, Setting.Value.Length * sizeof(wchar_t));

							} break;
						case DBT_ST_BLOB:
							{
								Setting.Value.Length = 0;
								switch (set->Type)
								{									
									case DBT_ST_BYTE:  Setting.Value.Length = 1; break;
									case DBT_ST_WORD:  Setting.Value.Length = 2; break;
									case DBT_ST_DWORD: Setting.Value.Length = 4; break;
									case DBT_ST_QWORD: Setting.Value.Length = 8; break;
								}
								
								Setting.Value.pBlob = (uint8_t *) mir_realloc(Setting.Value.pBlob, Setting.Value.Length);
								memcpy(Setting.Value.pBlob, &set->Value, Setting.Value.Length);
			

							} break;									
					}

				} break;
			case DBT_ST_CHAR: case DBT_ST_SHORT: case DBT_ST_INT: case DBT_ST_INT64:
				{
					int64_t val = 0;
					switch (set->Type) 
					{
						case DBT_ST_CHAR:  val = set->Value.Char;  break;
						case DBT_ST_SHORT: val = set->Value.Short; break;
						case DBT_ST_INT:   val = set->Value.Int;   break;
						case DBT_ST_INT64: val = set->Value.Int64; break;
					}
					switch (Setting.Type)
					{
						case DBT_ST_BYTE:  Setting.Value.Byte  = (uint8_t)   val; break;
						case DBT_ST_WORD:  Setting.Value.Word  = (uint16_t)  val; break;
						case DBT_ST_DWORD: Setting.Value.DWord = (uint32_t)  val; break;
						case DBT_ST_QWORD: Setting.Value.QWord = (uint64_t)  val; break;
						case DBT_ST_CHAR:  Setting.Value.Char  = ( int8_t)   val; break;
						case DBT_ST_SHORT: Setting.Value.Short = ( int16_t)  val; break;
						case DBT_ST_INT:   Setting.Value.Int   = ( int32_t)  val; break;
						case DBT_ST_INT64: Setting.Value.Int64 = ( int64_t)  val; break;
						case DBT_ST_BOOL:  Setting.Value.Bool  = val != 0; break;

						case DBT_ST_ASCIIZ: case DBT_ST_UTF8:
							{
								char buffer[24];
								buffer[0] = 0;
								sprintf_s(buffer, 24, "%lli", val);
								Setting.Value.Length = strlen(buffer) + 1;
								Setting.Value.pAnsii = (char *) mir_realloc(Setting.Value.pAnsii, Setting.Value.Length);
								memcpy(Setting.Value.pAnsii, buffer, Setting.Value.Length);
							
							} break;
						case DBT_ST_WCHAR:
							{
								wchar_t buffer[24];
								buffer[0] = 0;
								swprintf_s(buffer, 24, L"%lli", val);
								Setting.Value.Length = wcslen(buffer) + 1;
								Setting.Value.pWide = (wchar_t *) mir_realloc(Setting.Value.pWide, Setting.Value.Length * sizeof(wchar_t));
								memcpy(Setting.Value.pWide, buffer, Setting.Value.Length * sizeof(wchar_t));
								
							} break;
						case DBT_ST_BLOB:
							{
								Setting.Value.Length = 0;
								switch (set->Type)
								{									
									case DBT_ST_CHAR:  Setting.Value.Length = 1; break;
									case DBT_ST_SHORT: Setting.Value.Length = 2; break;
									case DBT_ST_INT:   Setting.Value.Length = 4; break;
									case DBT_ST_INT64: Setting.Value.Length = 8; break;
								}
								
								Setting.Value.pBlob = (unsigned char *) mir_realloc(Setting.Value.pBlob, Setting.Value.Length);
								memcpy(Setting.Value.pBlob, &set->Value, Setting.Value.Length);
						
							} break;								
					}

				} break;
			case DBT_ST_FLOAT: case DBT_ST_DOUBLE:
				{
					double val = 0;
					if (set->Type == DBT_ST_DOUBLE)
						val = set->Value.Double;
					else
						val = set->Value.Float;
				
					switch (Setting.Type)
					{
						case DBT_ST_BYTE:  Setting.Value.Byte  = (uint8_t)   floor(val); break;
						case DBT_ST_WORD:  Setting.Value.Word  = (uint16_t)  floor(val); break;
						case DBT_ST_DWORD: Setting.Value.DWord = (uint32_t)  floor(val); break;
						case DBT_ST_QWORD: Setting.Value.QWord = (uint64_t)  floor(val); break;
						case DBT_ST_CHAR:  Setting.Value.Char  = ( int8_t)   floor(val); break;
						case DBT_ST_SHORT: Setting.Value.Short = ( int16_t)  floor(val); break;
						case DBT_ST_INT:   Setting.Value.Int   = ( int32_t)  floor(val); break;
						case DBT_ST_INT64: Setting.Value.Int64 = ( int64_t)  floor(val); break;
						case DBT_ST_BOOL:  Setting.Value.Bool  = val != 0; break;
							
						case DBT_ST_ASCIIZ: case DBT_ST_UTF8:
							{
								char buffer[128];
								buffer[0] = 0;
								sprintf_s(buffer, 24, "%lf", set->Value.QWord);
								Setting.Value.Length = strlen(buffer) + 1;
								Setting.Value.pAnsii = (char *) mir_realloc(Setting.Value.pAnsii, Setting.Value.Length);
								memcpy(Setting.Value.pAnsii, buffer, Setting.Value.Length);								
							} break;
						case DBT_ST_WCHAR:
							{
								wchar_t buffer[128];
								buffer[0] = 0;
								swprintf_s(buffer, 24, L"%lf", set->Value.QWord);
								Setting.Value.Length = wcslen(buffer) + 1;
								Setting.Value.pWide = (wchar_t *) mir_realloc(Setting.Value.pWide, Setting.Value.Length * sizeof(wchar_t));
								memcpy(Setting.Value.pWide, buffer, Setting.Value.Length * sizeof(wchar_t));
							} break;
						case DBT_ST_BLOB:
							{
								Setting.Value.Length = 4;
								if (set->Type == DBT_ST_DOUBLE)
									Setting.Value.Length = 8;
								
								Setting.Value.pBlob = (uint8_t*) mir_realloc(Setting.Value.pBlob, Setting.Value.Length);
								memcpy(Setting.Value.pBlob, &set->Value, Setting.Value.Length);

							} break;									
					}

				} break;
			case DBT_ST_BOOL:
				{
					switch (Setting.Type)
					{
						case DBT_ST_BYTE: case DBT_ST_WORD: case DBT_ST_DWORD: case DBT_ST_QWORD: 
						case DBT_ST_CHAR: case DBT_ST_SHORT: case DBT_ST_INT: case DBT_ST_INT64:
							{
								if (set->Value.Bool)
									Setting.Value.QWord = 1;
								else
									Setting.Value.QWord = 0;
							} break;
						case DBT_ST_FLOAT:
							{
								if (set->Value.Bool)
									Setting.Value.Float = 1;
								else
									Setting.Value.Float = 0;
							} break;
						case DBT_ST_DOUBLE:
							{
								if (set->Value.Bool)
									Setting.Value.Double = 1;
								else
									Setting.Value.Double = 0;
							} break;
						case DBT_ST_ASCIIZ: case DBT_ST_UTF8:
							{
								char * buffer = "false";
								Setting.Value.Length = 5;
								if (set->Value.Bool)
								{
									buffer = "true";
									Setting.Value.Length = 4;
								}

								Setting.Value.pAnsii = (char *) mir_realloc(Setting.Value.pAnsii, Setting.Value.Length);
								memcpy(Setting.Value.pAnsii, buffer, Setting.Value.Length);						
							} break;
						case DBT_ST_WCHAR:
							{
								wchar_t * buffer = L"false";
								Setting.Value.Length = 5;
								if (set->Value.Bool)
								{
									buffer = L"true";
									Setting.Value.Length = 4;
								}

								Setting.Value.pWide = (wchar_t *) mir_realloc(Setting.Value.pWide, Setting.Value.Length * sizeof(wchar_t));
								memcpy(Setting.Value.pWide, buffer, Setting.Value.Length * sizeof(wchar_t));						
							} break;
						case DBT_ST_BLOB:
							{
								Setting.Value.pBlob = (uint8_t*) mir_realloc(Setting.Value.pBlob, 1);
								(*((bool*)Setting.Value.pBlob)) = set->Value.Bool;								
								Setting.Value.Length = 1;
							} break;
					}
				} break;
			case DBT_ST_ASCIIZ:
				{
					switch (Setting.Type)
					{
						case DBT_ST_BYTE: case DBT_ST_WORD: case DBT_ST_DWORD: case DBT_ST_QWORD: case DBT_ST_BOOL: 
						case DBT_ST_CHAR: case DBT_ST_SHORT: case DBT_ST_INT: case DBT_ST_INT64:
							{
								Setting.Value.QWord = 0;		
							} break;
						case DBT_ST_ASCIIZ:
							{
								Setting.Value.Length = set->BlobLength;
								Setting.Value.pAnsii = (char *) mir_realloc(Setting.Value.pAnsii, set->BlobLength);
								memcpy(Setting.Value.pAnsii, str, set->BlobLength);
								Setting.Value.pAnsii[Setting.Value.Length - 1] = 0;								
							} break;
						case DBT_ST_UTF8:
							{								
								str[set->BlobLength - 1] = 0;
								Setting.Value.pUTF8 = mir_utf8encode((char*)str);
								Setting.Value.Length = strlen(Setting.Value.pUTF8) + 1;
							} break;
						case DBT_ST_WCHAR:
							{				
								str[set->BlobLength - 1] = 0;
								Setting.Value.pWide = mir_a2u((char*)str);
								Setting.Value.Length = wcslen(Setting.Value.pWide) + 1;
							} break;
						case DBT_ST_BLOB:
							{
								Setting.Value.Length = set->BlobLength;
								Setting.Value.pBlob = (uint8_t *) mir_realloc(Setting.Value.pBlob, set->BlobLength);
								memcpy(Setting.Value.pBlob, str, set->BlobLength);								
							} break;
					}
				} break;
			case DBT_ST_UTF8:
				{
					switch (Setting.Type)
					{
						case DBT_ST_BYTE: case DBT_ST_WORD: case DBT_ST_DWORD: case DBT_ST_QWORD: case DBT_ST_BOOL: 
						case DBT_ST_CHAR: case DBT_ST_SHORT: case DBT_ST_INT: case DBT_ST_INT64:
							{
								Setting.Value.QWord = 0;		
							} break;
						case DBT_ST_ASCIIZ:
							{								
								str[set->BlobLength - 1] = 0;		
								mir_utf8decode((char*)str, NULL);								
								Setting.Value.Length = strlen((char*)str) + 1;
								mir_realloc(Setting.Value.pAnsii, Setting.Value.Length);								
							} break;
						case DBT_ST_UTF8:
							{
								Setting.Value.Length = set->BlobLength;
								Setting.Value.pUTF8 = (char *) mir_realloc(Setting.Value.pUTF8, set->BlobLength);
								memcpy(Setting.Value.pUTF8, str, set->BlobLength);
								Setting.Value.pUTF8[set->BlobLength - 1] = 0;									
							} break;
						case DBT_ST_WCHAR:
							{								
								str[set->BlobLength - 1] = 0;
								Setting.Value.pWide = mir_utf8decodeW((char*)str);								
								Setting.Value.Length = wcslen(Setting.Value.pWide) + 1;
							} break;
						case DBT_ST_BLOB:
							{
								Setting.Value.pBlob = (unsigned char *) mir_realloc(Setting.Value.pBlob, set->BlobLength);
								memcpy(Setting.Value.pBlob, str, set->BlobLength);
								Setting.Value.Length = set->BlobLength;
							} break;
					}
				} break;
			case DBT_ST_WCHAR:
				{
					switch (Setting.Type)
					{
						case DBT_ST_BYTE: case DBT_ST_WORD: case DBT_ST_DWORD: case DBT_ST_QWORD: case DBT_ST_BOOL: 
						case DBT_ST_CHAR: case DBT_ST_SHORT: case DBT_ST_INT: case DBT_ST_INT64:
							{
								Setting.Value.QWord = 0;		
							} break;
						case DBT_ST_ASCIIZ:
							{
								((wchar_t*)str)[set->BlobLength / sizeof(wchar_t) - 1] = 0;
								Setting.Value.pAnsii = mir_u2a((wchar_t*)str);
								Setting.Value.Length = strlen(Setting.Value.pAnsii) + 1;
							} break;
						case DBT_ST_UTF8:
							{
								((wchar_t*)str)[set->BlobLength / sizeof(wchar_t) - 1] = 0;
								Setting.Value.pUTF8 = mir_utf8encodeW((wchar_t*)str);
								Setting.Value.Length = strlen(Setting.Value.pUTF8) + 1;
							} break;
						case DBT_ST_WCHAR:
							{
								Setting.Value.Length = set->BlobLength / sizeof(wchar_t);
								((wchar_t*)str)[set->BlobLength / sizeof(wchar_t) - 1] = 0;
								Setting.Value.pWide = (wchar_t*) mir_realloc(Setting.Value.pWide, Setting.Value.Length * sizeof(wchar_t));
								memcpy(Setting.Value.pWide, str, set->BlobLength);
								Setting.Value.pWide[set->BlobLength / sizeof(wchar_t) - 1] = 0;
							} break;
						case DBT_ST_BLOB:
							{
								Setting.Value.pBlob = (uint8_t *) mir_realloc(Setting.Value.pBlob, set->BlobLength);
								memcpy(Setting.Value.pBlob, str, set->BlobLength);
								Setting.Value.Length = set->BlobLength;
							} break;
					}
				} break;
			case DBT_ST_BLOB:
				{
					switch (Setting.Type)
					{
						case DBT_ST_BYTE: case DBT_ST_WORD: case DBT_ST_DWORD: case DBT_ST_QWORD: case DBT_ST_BOOL: 
						case DBT_ST_CHAR: case DBT_ST_SHORT: case DBT_ST_INT: case DBT_ST_INT64:
							{
								Setting.Value.QWord = 0;		
							} break;
						case DBT_ST_ASCIIZ: case DBT_ST_WCHAR: case DBT_ST_UTF8:
							{
								Setting.Value.Length = 0;
								if (Setting.Value.pBlob)
									mir_free(Setting.Value.pBlob);

								Setting.Value.pBlob = NULL;
							} break;
						case DBT_ST_BLOB:
							{
								Setting.Value.pBlob = (uint8_t *) mir_realloc(Setting.Value.pBlob, set->BlobLength);
								memcpy(Setting.Value.pBlob, str, set->BlobLength);
								Setting.Value.Length = set->BlobLength;
							} break;
					}
				} break;

		}
	}


	if (Setting.Descriptor)
	{
		Setting.Descriptor->Contact = set->Contact;
		Setting.Descriptor->FoundInContact = set->Contact;
		
		Setting.Descriptor->pszSettingName = (char *) mir_realloc(Setting.Descriptor->pszSettingName, set->NameLength + 1);
		memcpy(Setting.Descriptor->pszSettingName, set + 1, set->NameLength + 1);
		Setting.Descriptor->pszSettingName[set->NameLength] = 0;
	}

	return set->Type;
}




TDBTSettingIterationHandle CSettings::IterationInit(TDBTSettingIterFilter & Filter)
{
	SYNC_BEGINREAD(m_Sync);

	std::queue<TDBTContactHandle> contacts;
	contacts.push(Filter.hContact);

	CSettingsTree * tree = getSettingsTree(Filter.hContact);

	if (tree == NULL)
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	if (Filter.hContact != 0)
	{	
		uint32_t cf = m_Contacts.getFlags(Filter.hContact);

		if (cf == DBT_INVALIDPARAM)
		{
			SYNC_ENDREAD(m_Sync);
			return DBT_INVALIDPARAM;
		}
		
		TDBTContactIterFilter f = {0};
		f.cbSize = sizeof(f);
		if (cf & DBT_CF_IsGroup)
		{
			f.fHasFlags = DBT_CF_IsGroup;
		} else {
			f.fDontHasFlags = DBT_CF_IsGroup;
		}
		f.Options = Filter.Options;

		TDBTContactIterationHandle citer = m_Contacts.IterationInit(f, Filter.hContact);
		if (citer != DBT_INVALIDPARAM)
		{
			m_Contacts.IterationNext(citer); // the initial contact was already added
			TDBTContactHandle e = m_Contacts.IterationNext(citer);
			while (e != 0)
			{
				contacts.push(e);
				e = m_Contacts.IterationNext(citer);
			}

			m_Contacts.IterationClose(citer);
		}
	}

	for (unsigned int j = 0; j < Filter.ExtraCount; ++j)
	{
		contacts.push(Filter.ExtraContacts[j]);
	}

	PSettingIteration iter = new TSettingIteration;
	iter->Filter = Filter;
	iter->FilterNameStartLength = 0;
	if (Filter.NameStart)
	{
		uint16_t l = strlen(Filter.NameStart);
		iter->Filter.NameStart = new char[l + 1];
		memcpy(iter->Filter.NameStart, Filter.NameStart, l + 1);
		iter->FilterNameStartLength = l;
	}

	TSettingKey key;
	key.Hash = 0;

	// pop first Contact. we have always one and always its tree
	contacts.pop();

	CSettingsTree::iterator * tmp = new CSettingsTree::iterator(tree->LowerBound(key));
	tmp->setManaged();
	iter->Heap = new TSettingsHeap(*tmp, TSettingsHeap::ITForward, true);
	
	while (!contacts.empty())
	{
		tree = getSettingsTree(contacts.front());
		contacts.pop();
		if (tree != NULL)
		{
			tmp = new CSettingsTree::iterator(tree->LowerBound(key));
			tmp->setManaged();
			iter->Heap->Insert(*tmp);
		}
	}
	
	iter->Frame = new std::queue<TSettingIterationResult>;

	SYNC_ENDREAD(m_Sync);

	return (TDBTSettingIterationHandle)iter;
}


typedef struct TSettingIterationHelper {
		TDBTSettingHandle Handle;
		CSettingsTree * Tree;
		uint16_t NameLen;
		char * Name;
	} TSettingIterationHelper;

TDBTSettingHandle CSettings::IterationNext(TDBTSettingIterationHandle Iteration)
{
	SYNC_BEGINREAD(m_Sync);

	PSettingIteration iter = (PSettingIteration)Iteration;
	uint32_t sig = cSettingSignature;

	while (iter->Frame->empty() && iter->Heap->Top())
	{
		while (iter->Heap->Top() && iter->Heap->Top().wasDeleted())
			iter->Heap->Pop();

		if (iter->Heap->Top())
		{
			uint32_t h = iter->Heap->Top().Key().Hash;
			std::queue<TSettingIterationHelper> q;
			TSettingIterationHelper help;
			help.NameLen = 0;
			help.Name = NULL;
			
			help.Handle = iter->Heap->Top().Data();
			help.Tree = (CSettingsTree *) iter->Heap->Top().Tree();
			if (help.Tree)
				q.push(help);
			
			iter->Heap->Pop();

			// add all candidates
			while (iter->Heap->Top() && (iter->Heap->Top().Key().Hash == h))
			{
				if (!iter->Heap->Top().wasDeleted())
				{
					help.Handle = iter->Heap->Top().Data();
					help.Tree = (CSettingsTree *) iter->Heap->Top().Tree();
					q.push(help);
				}
				iter->Heap->Pop();
			}

			while (!q.empty())
			{
				help = q.front();
				q.pop();

				if (help.Name == NULL)
				{
					if (help.Tree->getContact() == 0)
						_ReadSettingName(m_BlockManagerSet, m_EncryptionManagerSet, help.Handle, help.NameLen, help.Name);
					else
						_ReadSettingName(m_BlockManagerPri, m_EncryptionManagerPri, help.Handle, help.NameLen, help.Name);
				}
		

				q.push(help);
				while (q.front().Handle != help.Handle)  // remove all qequed settings with same name
				{
					bool namereadres = false;

					TSettingIterationHelper tmp;
					tmp = q.front();
					q.pop();

					if (tmp.Name == NULL)
					{
						if (help.Tree->getContact() == 0)
							namereadres = _ReadSettingName(m_BlockManagerSet, m_EncryptionManagerSet, tmp.Handle, tmp.NameLen, tmp.Name);
						else
							namereadres = _ReadSettingName(m_BlockManagerPri, m_EncryptionManagerPri, tmp.Handle, tmp.NameLen, tmp.Name);
					}

					if (!namereadres)
					{
						q.push(tmp);
					} else {
						if (strcmp(tmp.Name, help.Name) != 0)
						{
							q.push(tmp);
						} else {
							free(tmp.Name);
						}
					}
				}

				// namefilter
				if ((iter->Filter.NameStart == NULL) || ((iter->FilterNameStartLength <= help.NameLen) && (memcmp(iter->Filter.NameStart, help.Name, iter->FilterNameStartLength) == 0)))
				{
					TSettingIterationResult tmp;			
					if (help.Tree->getContact() == 0)
						help.Handle |= cSettingsFileFlag;

					tmp.Handle = help.Handle;
					tmp.Contact = help.Tree->getContact();
					tmp.Name = help.Name;
					tmp.NameLen = help.NameLen;
					iter->Frame->push(tmp);
				} else {
					free(help.Name);
				}

				q.pop();
			}
		}
	}


	TSettingIterationResult res = {0};
	if (!iter->Frame->empty())
	{
		res = iter->Frame->front();
		iter->Frame->pop();

		if ((iter->Filter.Descriptor) && ((iter->Filter.Setting == NULL) || (iter->Filter.Setting->Descriptor != iter->Filter.Descriptor)))
		{
			iter->Filter.Descriptor->Contact = res.Contact;
			iter->Filter.Descriptor->pszSettingName = (char *) mir_realloc(iter->Filter.Descriptor->pszSettingName, res.NameLen + 1);
			memcpy(iter->Filter.Descriptor->pszSettingName, res.Name, res.NameLen + 1);
			iter->Filter.Descriptor->FoundInContact = res.Contact;
		}
		if (iter->Filter.Setting)
		{
			if ((iter->Filter.Setting->Type & DBT_STF_VariableLength) && (iter->Filter.Setting->Value.pBlob))
			{
				mir_free(iter->Filter.Setting->Value.pBlob);
				iter->Filter.Setting->Value.pBlob = NULL;
			}
			iter->Filter.Setting->Type = 0;

			ReadSetting(*iter->Filter.Setting, res.Handle);
		}

		free(res.Name);
	}

	SYNC_ENDREAD(m_Sync);

	return res.Handle;
}
unsigned int CSettings::IterationClose(TDBTSettingIterationHandle Iteration)
{
	PSettingIteration iter = (PSettingIteration) Iteration;

	SYNC_BEGINREAD(m_Sync);
	delete iter->Heap;  // only this need syncronization
	SYNC_ENDREAD(m_Sync);

	if (iter->Filter.NameStart)
		delete [] iter->Filter.NameStart;

	if (iter->Filter.Descriptor && iter->Filter.Descriptor->pszSettingName)
	{
		mir_free(iter->Filter.Descriptor->pszSettingName);
		iter->Filter.Descriptor->pszSettingName = NULL;
	}
	if (iter->Filter.Setting)
	{
		if (iter->Filter.Setting->Descriptor)
		{
			mir_free(iter->Filter.Setting->Descriptor->pszSettingName);
			iter->Filter.Setting->Descriptor->pszSettingName = NULL;
		}	
		
		if (iter->Filter.Setting->Type & DBT_STF_VariableLength)
		{
			mir_free(iter->Filter.Setting->Value.pBlob);
			iter->Filter.Setting->Value.pBlob = NULL;
		}
	}

	while (!iter->Frame->empty())
	{
		if (iter->Frame->front().Name)
			free(iter->Frame->front().Name);

		iter->Frame->pop();
	}
	delete iter->Frame;
	delete iter;

	return 0;
}


int CSettings::CompEnumModules(DBMODULEENUMPROC CallBack, LPARAM lParam)
{
	SYNC_BEGINREAD(m_Sync);

	TModulesMap::iterator i = m_Modules.begin();
	int res = 0;
	while ((i != m_Modules.end()) && (res == 0))
	{
		char * tmp = i->second;
		SYNC_ENDREAD(m_Sync);

		res = CallBack(tmp, 0, lParam);

		SYNC_BEGINREAD(m_Sync);
		++i;
	}
	SYNC_ENDREAD(m_Sync);

	return res;
}