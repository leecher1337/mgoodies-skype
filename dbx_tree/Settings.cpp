#include "Settings.h"
#include <math.h>

__forceinline bool TSettingKey::operator <  (const TSettingKey & Other) const
{
	return Hash < Other.Hash;	
}

__forceinline bool TSettingKey::operator == (const TSettingKey & Other) const
{
	return (Hash == Other.Hash);
}

__forceinline bool TSettingKey::operator >  (const TSettingKey & Other) const
{	
	return Hash > Other.Hash;
}



CSettingsTree::CSettingsTree(CFileAccess & FileAccess, unsigned int RootNode, TDBEntryHandle Entry)
: CFileBTree(FileAccess, RootNode)
{
	m_Entry = Entry;
}

CSettingsTree::~CSettingsTree()
{

}

__forceinline TDBEntryHandle CSettingsTree::getEntry()
{
	return m_Entry;
}

TDBSettingHandle CSettingsTree::_FindSetting(const unsigned int Hash, const char * Name, const unsigned int Length)
{
	TSettingKey key;
	key.Hash = Hash;
	iterator i = Find(key);
	
	unsigned int res = 0;
	TSetting set;
	char * str = new char[Length + 1];
	while ((res == 0) && (i) && (i.Key().Hash == Hash))
	{
		m_FileAccess.Read(&set, i.Data(), sizeof(set));
	
		if (set.NameLength == Length)
		{
			m_FileAccess.Read(str, i.Data() + sizeof(set), Length + 1);
			if (strcmp(Name, str) == 0)
				res = i.Data();
		}
	
		++i;
	} 

	delete [] str;
	return res;
}

bool CSettingsTree::_DeleteSetting(const unsigned int Hash, const TDBSettingHandle hSetting)
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

bool CSettingsTree::_ChangeSetting(const unsigned int Hash, const TDBSettingHandle OldSetting, const TDBSettingHandle NewSetting)
{
	TSettingKey key;
	key.Hash = Hash;
	iterator i = Find(key);
	while ((i) && (i.Key().Hash == Hash) && (i.Data() != OldSetting))
		++i;

	if ((i) && (i.Key().Hash == Hash))
	{
		i.SetData(NewSetting);
		return true;
	}

	return false;
}

bool CSettingsTree::_AddSetting(const unsigned int Hash, const TDBSettingHandle hSetting)
{
	TSettingKey key;
	key.Hash = Hash;
	Insert(key, hSetting);
	return true;
}



/// lookup3, by Bob Jenkins, May 2006, Public Domain.
#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

#define HASHmix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}
#define HASHfinal(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c, 4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}


unsigned int CSettings::Hash(void * Data, unsigned int Length)
{
	register unsigned int a,b,c; // internal state
  union { const void *ptr; unsigned int i; } u; // needed for Mac Powerbook G4

  // Set up the internal state
  a = b = c = 0xdeadbeef + Length; // + initval = 0

  u.ptr = Data;
  if ((u.i & 0x3) == 0) 
	{
    const unsigned int  *k = (const unsigned int *)Data; // read 32-bit chunks

    // all but last block: aligned reads and affect 32 bits of (a,b,c)
    while (Length > 12)
    {
      a += k[0];
      b += k[1];
      c += k[2];
      HASHmix(a,b,c);
      Length -= 12;
      k += 3;
    }

    switch(Length)
    {
			case 12: c += k[2];            b += k[1]; a += k[0]; break;
			case 11: c += k[2] & 0xffffff; b += k[1]; a += k[0]; break;
			case 10: c += k[2] & 0xffff;   b += k[1]; a += k[0]; break;
			case 9 : c += k[2] & 0xff;     b += k[1]; a += k[0]; break;
			case 8 : b += k[1];            a += k[0]; break;
			case 7 : b += k[1] & 0xffffff; a += k[0]; break;
			case 6 : b += k[1] & 0xffff;   a += k[0]; break;
			case 5 : b += k[1] & 0xff;     a += k[0]; break;
			case 4 : a += k[0];            break;
			case 3 : a += k[0] & 0xffffff; break;
			case 2 : a += k[0] & 0xffff;   break;
			case 1 : a += k[0] & 0xff;     break;
			case 0 : return c;  // zero length strings require no mixing
    }

  } else if ((u.i & 0x1) == 0) {
    const unsigned short *k = (const unsigned short *)Data;         /* read 16-bit chunks */
    const unsigned char  *k8;

    // all but last block: aligned reads and different mixing
    while (Length > 12)
    {
      a += k[0] + (((unsigned int)k[1]) << 16);
      b += k[2] + (((unsigned int)k[3]) << 16);
      c += k[4] + (((unsigned int)k[5]) << 16);
      HASHmix(a,b,c);
      Length -= 12;
      k += 6;
    }

    // handle the last (probably partial) block
    k8 = (const unsigned char *)k;
    switch(Length)
    {
			case 12: c += k[4] + (((unsigned int)k[5]) << 16);
							 b += k[2] + (((unsigned int)k[3]) << 16);
							 a += k[0] + (((unsigned int)k[1]) << 16);
							 break;
			case 11: c += ((unsigned int)k8[10]) << 16; // fall through 
			case 10: c += k[4];
							 b += k[2] + (((unsigned int)k[3]) << 16);
							 a += k[0] + (((unsigned int)k[1]) << 16);
							 break;
			case 9 : c += k8[8];                        // fall through
			case 8 : b += k[2] + (((unsigned int)k[3]) << 16);
							 a += k[0] + (((unsigned int)k[1]) << 16);
							 break;
			case 7 : b += ((unsigned int)k8[6]) << 16;  // fall through
			case 6 : b += k[2];
							 a += k[0] + (((unsigned int)k[1]) << 16);
							 break;
			case 5 : b += k8[4];                        // fall through
			case 4 : a += k[0] + (((unsigned int)k[1]) << 16);
							 break;
			case 3 : a += ((unsigned int)k8[2]) << 16;  // fall through
			case 2 : a += k[0];
							 break;
			case 1 : a += k8[0];
							 break;
			case 0 : return c; // zero length requires no mixing
    }

  } else { // need to read the key one byte at a time
    const unsigned char *k = (const unsigned char *)Data;

    // all but the last block: affect some 32 bits of (a,b,c)
    while (Length > 12)
    {
      a += k[0];
      a += ((unsigned int)k[1] ) <<  8;
      a += ((unsigned int)k[2] ) << 16;
      a += ((unsigned int)k[3] ) << 24;
      b += k[4];
      b += ((unsigned int)k[5] ) <<  8;
      b += ((unsigned int)k[6] ) << 16;
      b += ((unsigned int)k[7] ) << 24;
      c += k[8];
      c += ((unsigned int)k[9] ) << 8;
      c += ((unsigned int)k[10]) << 16;
      c += ((unsigned int)k[11]) << 24;
      HASHmix(a,b,c);
      Length -= 12;
      k += 12;
    }

    // last block: affect all 32 bits of (c)
    switch(Length) // all the case statements fall through
    {
			case 12: c += ((unsigned int)k[11]) << 24;
			case 11: c += ((unsigned int)k[10]) << 16;
			case 10: c += ((unsigned int)k[9] ) <<  8;
			case 9 : c += k[8];
			case 8 : b += ((unsigned int)k[7] ) << 24;
			case 7 : b += ((unsigned int)k[6] ) << 16;
			case 6 : b += ((unsigned int)k[5] ) <<  8;
			case 5 : b += k[4];
			case 4 : a += ((unsigned int)k[3] ) << 24;
			case 3 : a += ((unsigned int)k[2] ) << 16;
			case 2 : a += ((unsigned int)k[1] ) <<  8;
			case 1 : a += k[0];
							 break;
			case 0 : return c;
    }
  }

  HASHfinal(a,b,c);
  return c;
}

#undef rot
#undef HASHmix
#undef HASHfinal





CSettings::CSettings(CFileAccess & SettingsFile, CFileAccess & PrivateFile, CMultiReadExclusiveWriteSynchronizer & Synchronize, unsigned int SettingsRoot, CEntries & Entries)
:	m_Sync(Synchronize),
	m_SettingsFile(SettingsFile),
	m_PrivateFile(PrivateFile),
	m_Entries(Entries),
	m_SettingsMap(),

	m_sigRootChanged()	
{
	CSettingsTree * settree = new CSettingsTree(SettingsFile, SettingsRoot, 0);

	settree->sigRootChanged().connect(this, &CSettings::onRootChanged);
	m_SettingsMap.insert(std::make_pair(0, settree));

	m_IterAllocSize = 1;
	m_Iterations = new PSettingIteration[1];
}

CSettings::~CSettings()
{
	m_Sync.BeginWrite();
	std::map<TDBEntryHandle, CSettingsTree*>::iterator it = m_SettingsMap.begin();

	while (it != m_SettingsMap.end())
	{
		delete it->second;
		++it;
	}

	m_SettingsMap.clear();

	for (unsigned int i = 0; i < m_IterAllocSize; i++)
	{
		if (m_Iterations[i])
			IterationClose(i + 1);
	}
	delete [] m_Iterations;
	m_Sync.EndWrite();
}


CSettingsTree * CSettings::getSettingsTree(TDBEntryHandle hEntry)
{
	TSettingsTreeMap::iterator i = m_SettingsMap.find(hEntry);
	if (i != m_SettingsMap.end())
		return i->second;

	unsigned int root = m_Entries._getSettingsRoot(hEntry);
	if (root == DB_INVALIDPARAM)
		return NULL;

	CSettingsTree * tree = new CSettingsTree(m_PrivateFile, root, hEntry);
	tree->sigRootChanged().connect(this, &CSettings::onRootChanged);
	m_SettingsMap.insert(std::make_pair(hEntry, tree));

	return tree;	
}

CSettings::TOnRootChanged & CSettings::sigRootChanged()
{
	return m_sigRootChanged;
}
void CSettings::onRootChanged(void* SettingsTree, unsigned int NewRoot)
{
	if (((CSettingsTree*)SettingsTree)->getEntry() == 0)
		m_sigRootChanged.emit(this, NewRoot);
	else 
		m_Entries._setSettingsRoot(((CSettingsTree*)SettingsTree)->getEntry(), NewRoot);
}

	

TDBSettingHandle CSettings::FindSetting(TDBSettingDescriptor & Descriptor)
{
	unsigned int NameLength = strlen(Descriptor.pszSettingName);
	unsigned int namehash = Hash(Descriptor.pszSettingName, NameLength);
	CSettingsTree * tree;
	TDBSettingHandle res = 0;

	m_Sync.BeginRead();

	if (Descriptor.Entry == 0)
	{
		tree = getSettingsTree(0);
		res = tree->_FindSetting(namehash, Descriptor.pszSettingName, NameLength);
		m_Sync.EndRead();
		return res | cSettingsFileFlag;
	}

	unsigned int entryflags = m_Entries.getFlags(Descriptor.Entry);

	if (entryflags == DB_INVALIDPARAM)
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
	}


	if (Descriptor.Flags & DB_SDF_FoundEntryValid)
	{
		tree = getSettingsTree(Descriptor.FoundInEntry);
		if (tree)
		{
			res = tree->_FindSetting(namehash, Descriptor.pszSettingName, NameLength);
			if (res)
			{
				m_Sync.EndRead();			
				return res;
			}
		}
	}

	// build list of entries
	std::queue<TDBEntryHandle> entries;
	entries.push(Descriptor.Entry);
	
	if ((entryflags & DB_EF_IsVirtual) && ((Descriptor.Flags & DB_SDF_NoPrimaryVirtualLookup) == 0))
	{	// virtual lookup
		TDBEntryHandle e = m_Entries.VirtualGetParent(Descriptor.Entry);
		if (e != DB_INVALIDPARAM)
			entries.push(e);
		
	}

	if (Descriptor.Flags & DB_SDF_SearchSubEntries)
	{
		TDBEntryIterFilter filter = {0};
		
		filter.cbSize = sizeof(filter);
		filter.hParentEntry = Descriptor.Entry;

		if ((entryflags & DB_EF_IsGroup) && !((Descriptor.Flags & DB_SDF_SearchOutOfGroups) == DB_SDF_SearchOutOfGroups))
		{	// only groups
			filter.fHasFlags = DB_EF_IsGroup;
		}

		TDBEntryIterationHandle iter = m_Entries.IterationInit(filter);
		if ((iter != 0) && (iter != DB_INVALIDPARAM))
		{
			TDBEntryHandle e = m_Entries.IterationNext(iter);
			while ((e != 0) && (e != DB_INVALIDPARAM))
			{
				entries.push(e);				
				
				if (!((Descriptor.Flags & DB_SDF_NoVirtualLookup) == DB_SDF_NoVirtualLookup))
				{
					e = m_Entries.VirtualGetParent(e);
					if (e != DB_INVALIDPARAM)
						entries.push(e);		
				}
				
				e = m_Entries.IterationNext(iter);
			}

		}
	}

	if (Descriptor.Flags & DB_SDF_SearchParents)
	{
		TDBEntryHandle p;
		p = m_Entries.getParent(Descriptor.Entry);			

		while ((p != 0) && (p != DB_INVALIDPARAM) && (p != m_Entries.getRootEntry()) && ((m_Entries.getFlags(p) & DB_EF_IsGroup) == (entryflags & DB_EF_IsGroup)))
		{
			entries.push(p);

			if (!((Descriptor.Flags & DB_SDF_NoVirtualLookup) == DB_SDF_NoVirtualLookup))
			{
				TDBEntryHandle e = m_Entries.VirtualGetParent(p);
				if (e != DB_INVALIDPARAM)
					entries.push(e);		
			}
			
			p = m_Entries.getParent(p);
		}
	}
	

	if (Descriptor.Flags & DB_SDF_RootHasStandard)
	{
		entries.push(m_Entries.getRootEntry());
	}


	// search the setting
	res = 0;
	TDBEntryHandle e = 0;
	while ((res == 0) && (!entries.empty()))
	{
		e = entries.front();
		entries.pop();
		
		tree = getSettingsTree(e);
		if (tree)
			res = tree->_FindSetting(namehash, Descriptor.pszSettingName, NameLength);
	}

	if (res)
	{
		Descriptor.FoundInEntry = e;
		Descriptor.Flags |= DB_SDF_FoundEntryValid;
	}

	m_Sync.EndRead();

	return res;
}
unsigned int CSettings::DeleteSetting(TDBSettingDescriptor & Descriptor)
{
	m_Sync.BeginWrite();

	TDBSettingHandle hset = FindSetting(Descriptor);
	if ((hset == 0) || (hset == DB_INVALIDPARAM))
	{		
		m_Sync.EndWrite();
		return hset;
	}

	unsigned int res = DeleteSetting(hset);

	m_Sync.EndWrite();
	
	return res;
}
unsigned int CSettings::DeleteSetting(TDBSettingHandle hSetting)
{
	m_Sync.BeginWrite();
	
	CFileAccess & file(m_PrivateFile);

	if (hSetting & cSettingsFileFlag)
	{
		file = m_SettingsFile;
		hSetting = hSetting & ~cSettingsFileFlag;
	}

	TSetting set;
	file.Read(&set, hSetting, sizeof(set));

	if (set.Signature != cSettingSignature)
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	CSettingsTree * tree = getSettingsTree(set.Entry);
	if (tree == NULL)
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	char * str = new char[set.NameLength + 1];
	file.Read(str, hSetting + sizeof(set), set.NameLength + 1);
	
	tree->_DeleteSetting(Hash(str, set.NameLength), hSetting);
	
	if (set.Type & DB_STF_VariableLength)
		file.Free(hSetting, sizeof(set) + set.NameLength + 1 + set.AllocSize);
	else
		file.Free(hSetting, sizeof(set) + set.NameLength + 1);

	m_Sync.EndWrite();

	delete [] str;
	return 0;
}
TDBSettingHandle CSettings::WriteSetting(TDBSetting & Setting)
{
	m_Sync.BeginWrite();

	TDBSettingHandle hset = FindSetting(*Setting.Descriptor);
	if (hset == DB_INVALIDPARAM)
	{		
		m_Sync.EndWrite();
		return hset;
	}

	hset = WriteSetting(Setting, hset);

	m_Sync.EndWrite();
	
	return hset;
}
TDBSettingHandle CSettings::WriteSetting(TDBSetting & Setting, TDBSettingHandle hSetting)
{
	m_Sync.BeginWrite();
	
	CFileAccess & file(m_PrivateFile);
	bool fileflag = false;

	TSetting set = {0};
	CSettingsTree * tree = NULL;

	if (hSetting & cSettingsFileFlag)
	{
		file = m_SettingsFile;
		hSetting = hSetting & ~cSettingsFileFlag;
		fileflag = true;
	}

	if (hSetting == 0)
	{
		if (Setting.Descriptor->Entry == 0)
		{
			file = m_SettingsFile;
			fileflag = true;
		}

		if ((Setting.Descriptor) && (Setting.Descriptor->pszSettingName)) // setting needs a name
			tree = getSettingsTree(Setting.Descriptor->Entry);
		
	} else {		
		file.Read(&set, hSetting, sizeof(set));

		if (set.Signature == cSettingSignature)
			tree = getSettingsTree(set.Entry);
	}

	if (tree == NULL)
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}


	if (hSetting == 0) // create new setting
	{
		int neededsize = 0;
		switch (Setting.Type)
		{
			case DB_ST_ASCIIZ: case DB_ST_UTF8:
				{
					if (Setting.Value.Length == 0)
						neededsize = strlen(Setting.Value.pAnsii);
					else
						neededsize = Setting.Value.Length - 1;
				} break;
			case DB_ST_WCHAR:
				{
					if (Setting.Value.Length == 0)
						neededsize = sizeof(wchar_t) * wcslen(Setting.Value.pWide);
					else
						neededsize = sizeof(wchar_t) * (Setting.Value.Length - 1);
				} break;
			case DB_ST_BLOB:
				neededsize = Setting.Value.Length;
				break;
		}

		set.Signature = cSettingSignature;
		set.Entry = Setting.Descriptor->Entry;
		set.Flags = 0;
		set.Type = Setting.Type;
		set.AllocSize = neededsize;

		set.NameLength = strlen(Setting.Descriptor->pszSettingName);
		
		neededsize += sizeof(set) + set.NameLength + 1;
		
		hSetting = file.Alloc(neededsize);

		tree->_AddSetting(Hash(Setting.Descriptor->pszSettingName, set.NameLength), hSetting);		


	} else {

		if (((Setting.Type & DB_STF_VariableLength) == 0) && (set.Type & DB_STF_VariableLength))
		{ // shrink setting (variable size->fixed size)
			file.Free(hSetting + sizeof(set) + set.NameLength + 1, set.AllocSize);
		}

		if ((Setting.Type & DB_STF_VariableLength) && ((set.Type & DB_STF_VariableLength) == 0))
		{ // trick it
			set.AllocSize = 0;
		}
	}



	if (Setting.Type & DB_STF_VariableLength)
	{
		unsigned int neededblob;
		switch (Setting.Type)
		{
			case DB_ST_ASCIIZ: case DB_ST_UTF8:
				{
					if (Setting.Value.Length == 0)
						neededblob = strlen(Setting.Value.pAnsii);
					else
						neededblob = Setting.Value.Length - 1;
				} break;
			case DB_ST_WCHAR:
				{
					if (Setting.Value.Length == 0)
						neededblob = sizeof(wchar_t) * wcslen(Setting.Value.pWide);
					else
						neededblob = sizeof(wchar_t) * (Setting.Value.Length - 1);
				} break;
			default:
				neededblob = Setting.Value.Length;
				break;
		}


		if (set.AllocSize < neededblob)
		{
			char * name = new char[set.NameLength + 1];
			file.Read(name, hSetting + sizeof(set), set.NameLength + 1);
			unsigned int hash = Hash(name, set.NameLength);

			file.Free(hSetting, sizeof(set) + set.NameLength + 1 + set.AllocSize);
			unsigned int tmp = file.Alloc(sizeof(set) + set.NameLength + 1 + neededblob);

			tree->_ChangeSetting(hash, hSetting, tmp);
			hSetting = tmp;

			file.Write(name, hSetting + sizeof(set), set.NameLength + 1);

			delete [] name;

			set.AllocSize = neededblob;
		}

		set.BlobLength = neededblob;
		set.Type = Setting.Type;
		file.Write(&set, hSetting, sizeof(set));
		file.Write(Setting.Value.pBlob, hSetting + sizeof(set) + set.NameLength + 1, neededblob);

	} else {

		switch (Setting.Type)	
		{
			case DB_ST_BYTE: case DB_ST_CHAR: case DB_ST_BOOL:
				memset(((unsigned char*) &Setting.Value) + 1, 0, sizeof(Setting.Value) - 1); break;
			case DB_ST_WORD: case DB_ST_SHORT:
				memset(((unsigned char*) &Setting.Value) + 2, 0, sizeof(Setting.Value) - 2); break;
			case DB_ST_DWORD: case DB_ST_INT: case DB_ST_FLOAT:
				memset(((unsigned char*) &Setting.Value) + 4, 0, sizeof(Setting.Value) - 4); break;			
		}
		set.Type = Setting.Type;
		set.Value = Setting.Value;
		file.Write(&set, hSetting, sizeof(set));
	}

	if (fileflag)
	{
		hSetting = hSetting | cSettingsFileFlag;
	}
	
	m_Sync.EndWrite();

	return hSetting;
}
unsigned int CSettings::ReadSetting(TDBSetting & Setting)
{
	m_Sync.BeginRead();

	TDBSettingHandle hset = FindSetting(*Setting.Descriptor);
	if ((hset == 0) || (hset == DB_INVALIDPARAM))
	{		
		m_Sync.EndRead();
		return hset;
	}

	PDBSettingDescriptor back = Setting.Descriptor;
	Setting.Descriptor = NULL;
	hset = ReadSetting(Setting, hset);
	Setting.Descriptor = back;

	m_Sync.EndRead();

	return hset;
}
unsigned int CSettings::ReadSetting(TDBSetting & Setting, TDBSettingHandle hSetting)
{
	m_Sync.BeginRead();
	
	CFileAccess & file(m_PrivateFile);

	if (hSetting & cSettingsFileFlag)
	{
		file = m_SettingsFile;
		hSetting = hSetting & ~cSettingsFileFlag;
	}

	TSetting set = {0};
	if (hSetting == 0)
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
	} else {		
		file.Read(&set, hSetting, sizeof(set));

		if (set.Signature != cSettingSignature)
		{
			m_Sync.EndRead();
			return DB_INVALIDPARAM;
		}
	}

	if (Setting.Type == 0)
	{
		Setting.Type = set.Type;
		Setting.Value = set.Value;
		if (set.Type & DB_STF_VariableLength)
		{
			Setting.Value.Length = set.BlobLength;
			if (set.Type == DB_ST_WCHAR)
			{
				Setting.Value.Length = (set.BlobLength + sizeof(wchar_t) - 1) / sizeof(wchar_t);
				Setting.Value.pWide = (wchar_t*) mir_realloc(Setting.Value.pWide, set.BlobLength + sizeof(wchar_t) - 1);
				file.Read(Setting.Value.pWide, hSetting + sizeof(set) + set.NameLength + 1, set.BlobLength);
				Setting.Value.pWide[Setting.Value.Length - 1] = 0;
			} else if	((set.Type == DB_ST_ASCIIZ) || (set.Type == DB_ST_UTF8))
			{
				Setting.Value.Length = set.BlobLength + 1;
				Setting.Value.pAnsii = (char *) mir_realloc(Setting.Value.pWide, set.BlobLength + 1);
				file.Read(Setting.Value.pAnsii, hSetting + sizeof(set) + set.NameLength + 1, set.BlobLength);
				Setting.Value.pWide[Setting.Value.Length - 1] = 0;
			} else {
				Setting.Value.Length = set.BlobLength;
				Setting.Value.pBlob = (unsigned char *) mir_realloc(Setting.Value.pWide, set.BlobLength + 1);
				file.Read(Setting.Value.pBlob, hSetting + sizeof(set) + set.NameLength + 1, set.BlobLength);
			}
		}
	} else {
		switch (set.Type)
		{
			case DB_ST_BYTE: case DB_ST_WORD: case DB_ST_DWORD: case DB_ST_QWORD:
				{
					switch (Setting.Type)
					{
						case DB_ST_BYTE:  Setting.Value.Byte  = (unsigned char)        set.Value.QWord; break;
						case DB_ST_WORD:  Setting.Value.Word  = (unsigned short)       set.Value.QWord; break;
						case DB_ST_DWORD: Setting.Value.DWord = (unsigned int)         set.Value.QWord; break;
						case DB_ST_QWORD: Setting.Value.QWord =                        set.Value.QWord; break;
						case DB_ST_CHAR:  Setting.Value.Char  = (signed char)          set.Value.QWord; break;
						case DB_ST_SHORT: Setting.Value.Short = (signed short)         set.Value.QWord; break;
						case DB_ST_INT:   Setting.Value.Int   = (signed int)           set.Value.QWord; break;
						case DB_ST_INT64: Setting.Value.Int64 = (signed long long int) set.Value.QWord; break;
						case DB_ST_BOOL:  Setting.Value.Bool  = set.Value.QWord != 0; break;

						case DB_ST_ASCIIZ: case DB_ST_UTF8:
							{
								char buffer[24];
								buffer[0] = 0;
								sprintf_s(buffer, 24, "%llu", set.Value.QWord);
								unsigned int l = strlen(buffer);
								Setting.Value.pAnsii = (char *) mir_realloc(Setting.Value.pAnsii, l + 1);
								memcpy(Setting.Value.pAnsii, buffer, l + 1);
								Setting.Value.Length = l + 1;							
							} break;
						case DB_ST_WCHAR:
							{
								wchar_t buffer[24];
								buffer[0] = 0;
								swprintf_s(buffer, 24, L"%llu", set.Value.QWord);
								unsigned int l = wcslen(buffer);
								Setting.Value.pWide = (wchar_t *) mir_realloc(Setting.Value.pWide, (l + 1) * sizeof(wchar_t));
								memcpy(Setting.Value.pWide, buffer, (l + 1) * sizeof(wchar_t));
								Setting.Value.Length = l + 1;
							} break;
						case DB_ST_BLOB:
							{
								unsigned int l = 0;
								switch (set.Type)
								{									
									case DB_ST_BYTE:  l = 1; break;
									case DB_ST_WORD:  l = 2; break;
									case DB_ST_DWORD: l = 4; break;
									case DB_ST_QWORD: l = 8; break;
								}
								
								Setting.Value.pBlob = (unsigned char *) mir_realloc(Setting.Value.pBlob, l);
								memcpy(Setting.Value.pBlob, &set.Value, l);
								Setting.Value.Length = l;

							} break;									
					}

				} break;
			case DB_ST_CHAR: case DB_ST_SHORT: case DB_ST_INT: case DB_ST_INT64:
				{
					signed long long int val = 0;
					switch (set.Type) 
					{
						case DB_ST_CHAR:  val = set.Value.Char;  break;
						case DB_ST_SHORT: val = set.Value.Short; break;
						case DB_ST_INT:   val = set.Value.Int;   break;
						case DB_ST_INT64: val = set.Value.Int64; break;
					}
					switch (Setting.Type)
					{
						case DB_ST_BYTE:  Setting.Value.Byte  = (unsigned char)          val; break;
						case DB_ST_WORD:  Setting.Value.Word  = (unsigned short)         val; break;
						case DB_ST_DWORD: Setting.Value.DWord = (unsigned int)           val; break;
						case DB_ST_QWORD: Setting.Value.QWord = (unsigned long long int) val; break;
						case DB_ST_CHAR:  Setting.Value.Char  = (signed char)            val; break;
						case DB_ST_SHORT: Setting.Value.Short = (signed short)           val; break;
						case DB_ST_INT:   Setting.Value.Int   = (signed int)             val; break;
						case DB_ST_INT64: Setting.Value.Int64 =                          val; break;
						case DB_ST_BOOL:  Setting.Value.Bool  = val != 0; break;

						case DB_ST_ASCIIZ: case DB_ST_UTF8:
							{
								char buffer[24];
								buffer[0] = 0;
								sprintf_s(buffer, 24, "%lli", val);
								unsigned int l = strlen(buffer);
								Setting.Value.pAnsii = (char *) mir_realloc(Setting.Value.pAnsii, l + 1);
								memcpy(Setting.Value.pAnsii, buffer, l + 1);
								Setting.Value.Length = l + 1;							
							} break;
						case DB_ST_WCHAR:
							{
								wchar_t buffer[24];
								buffer[0] = 0;
								swprintf_s(buffer, 24, L"%lli", val);
								unsigned int l = wcslen(buffer);
								Setting.Value.pWide = (wchar_t *) mir_realloc(Setting.Value.pWide, (l + 1) * sizeof(wchar_t));
								memcpy(Setting.Value.pWide, buffer, (l + 1) * sizeof(wchar_t));
								Setting.Value.Length = l + 1;
							} break;
						case DB_ST_BLOB:
							{
								unsigned int l = 0;
								switch (set.Type)
								{									
									case DB_ST_CHAR:  l = 1; break;
									case DB_ST_SHORT: l = 2; break;
									case DB_ST_INT:   l = 4; break;
									case DB_ST_INT64: l = 8; break;
								}
								
								Setting.Value.pBlob = (unsigned char *) mir_realloc(Setting.Value.pBlob, l);
								memcpy(Setting.Value.pBlob, &set.Value, l);
								Setting.Value.Length = l;

							} break;								
					}

				} break;
			case DB_ST_FLOAT: case DB_ST_DOUBLE:
				{
					double val = 0;
					if (set.Type == DB_ST_DOUBLE)
						val = set.Value.Double;
					else
						val = set.Value.Float;
				
					switch (Setting.Type)
					{
						case DB_ST_BYTE:  Setting.Value.Byte  = (unsigned char)          floor(val); break;
						case DB_ST_WORD:  Setting.Value.Word  = (unsigned short)         floor(val); break;
						case DB_ST_DWORD: Setting.Value.DWord = (unsigned int)           floor(val); break;
						case DB_ST_QWORD: Setting.Value.QWord = (unsigned long long int) floor(val); break;
						case DB_ST_CHAR:  Setting.Value.Char  = (signed char)            floor(val); break;
						case DB_ST_SHORT: Setting.Value.Short = (signed short)           floor(val); break;
						case DB_ST_INT:   Setting.Value.Int   = (signed int)             floor(val); break;
						case DB_ST_INT64: Setting.Value.Int64 = (signed long long int)   floor(val); break;
						case DB_ST_BOOL:  Setting.Value.Bool  = val != 0; break;
							
						case DB_ST_ASCIIZ: case DB_ST_UTF8:
							{
								char buffer[128];
								buffer[0] = 0;
								sprintf_s(buffer, 24, "%lf", set.Value.QWord);
								unsigned int l = strlen(buffer);
								Setting.Value.pAnsii = (char *) mir_realloc(Setting.Value.pAnsii, l + 1);
								memcpy(Setting.Value.pAnsii, buffer, l + 1);
								Setting.Value.Length = l + 1;							
							} break;
						case DB_ST_WCHAR:
							{
								wchar_t buffer[128];
								buffer[0] = 0;
								swprintf_s(buffer, 24, L"%lf", set.Value.QWord);
								unsigned int l = wcslen(buffer);
								Setting.Value.pWide = (wchar_t *) mir_realloc(Setting.Value.pWide, (l + 1) * sizeof(wchar_t));
								memcpy(Setting.Value.pWide, buffer, (l + 1) * sizeof(wchar_t));
								Setting.Value.Length = l + 1;
							} break;
						case DB_ST_BLOB:
							{
								unsigned int l = 4;
								if (set.Type == DB_ST_DOUBLE)
									l = 8;
								
								Setting.Value.pBlob = (unsigned char *) mir_realloc(Setting.Value.pBlob, l);
								memcpy(Setting.Value.pBlob, &set.Value, l);
								Setting.Value.Length = l;

							} break;									
					}

				} break;
			case DB_ST_BOOL:
				{
					switch (Setting.Type)
					{
						case DB_ST_BYTE: case DB_ST_WORD: case DB_ST_DWORD: case DB_ST_QWORD: 
						case DB_ST_CHAR: case DB_ST_SHORT: case DB_ST_INT: case DB_ST_INT64:
							{
								if (set.Value.Bool)
									Setting.Value.QWord = 1;
								else
									Setting.Value.QWord = 0;
							} break;
						case DB_ST_FLOAT:
							{
								if (set.Value.Bool)
									Setting.Value.Float = 1;
								else
									Setting.Value.Float = 0;
							} break;
						case DB_ST_DOUBLE:
							{
								if (set.Value.Bool)
									Setting.Value.Double = 1;
								else
									Setting.Value.Double = 0;
							} break;
						case DB_ST_ASCIIZ: case DB_ST_UTF8:
							{
								char * buffer = "false";
								unsigned int l = 5;
								if (set.Value.Bool)
								{
									buffer = "true";
									l = 4;
								}

								Setting.Value.pAnsii = (char *) mir_realloc(Setting.Value.pAnsii, l + 1);
								memcpy(Setting.Value.pAnsii, buffer, l + 1);
								Setting.Value.Length = l + 1;							
							} break;
						case DB_ST_WCHAR:
							{
								wchar_t * buffer = L"false";
								unsigned int l = 5;
								if (set.Value.Bool)
								{
									buffer = L"true";
									l = 4;
								}

								Setting.Value.pWide = (wchar_t *) mir_realloc(Setting.Value.pWide, (l + 1) * sizeof(wchar_t));
								memcpy(Setting.Value.pWide, buffer, (l + 1) * sizeof(wchar_t));
								Setting.Value.Length = l + 1;							
							} break;
						case DB_ST_BLOB:
							{
								Setting.Value.pBlob = (unsigned char *) mir_realloc(Setting.Value.pBlob, 1);
								(*((bool*)Setting.Value.pBlob)) = set.Value.Bool;								
								Setting.Value.Length = 1;
							} break;
					}
				} break;
			case DB_ST_ASCIIZ:
				{
					switch (Setting.Type)
					{
						case DB_ST_BYTE: case DB_ST_WORD: case DB_ST_DWORD: case DB_ST_QWORD: case DB_ST_BOOL: 
						case DB_ST_CHAR: case DB_ST_SHORT: case DB_ST_INT: case DB_ST_INT64:
							{
								Setting.Value.QWord = 0;		
							} break;
						case DB_ST_ASCIIZ:
							{
								Setting.Value.pAnsii = (char *) mir_realloc(Setting.Value.pAnsii, set.BlobLength + 1);
								file.Read(Setting.Value.pAnsii, hSetting + sizeof(set) + set.NameLength + 1, set.BlobLength);
								Setting.Value.pAnsii[set.BlobLength] = 0;
								Setting.Value.Length = set.BlobLength + 1;
							} break;
						case DB_ST_UTF8:
							{
								char * buffer = new char[set.BlobLength + 1];								
								file.Read(buffer, hSetting + sizeof(set) + set.NameLength + 1, set.BlobLength);
								buffer[set.BlobLength] = 0;
								Setting.Value.pUTF8 = mir_utf8encode(buffer);
								Setting.Value.Length = DB_INVALIDPARAM;
								delete [] buffer;
							} break;
						case DB_ST_WCHAR:
							{
								char * buffer = new char[set.BlobLength + 1];								
								file.Read(buffer, hSetting + sizeof(set) + set.NameLength + 1, set.BlobLength);
								buffer[set.BlobLength] = 0;
								Setting.Value.pWide = mir_a2u(buffer);
								Setting.Value.Length = DB_INVALIDPARAM;
								delete [] buffer;
							} break;
						case DB_ST_BLOB:
							{
								Setting.Value.pBlob = (unsigned char *) mir_realloc(Setting.Value.pBlob, set.BlobLength);
								file.Read(Setting.Value.pBlob, hSetting + sizeof(set) + set.NameLength + 1, set.BlobLength);
								Setting.Value.Length = set.BlobLength;
							} break;
					}
				} break;
			case DB_ST_UTF8:
				{
					switch (Setting.Type)
					{
						case DB_ST_BYTE: case DB_ST_WORD: case DB_ST_DWORD: case DB_ST_QWORD: case DB_ST_BOOL: 
						case DB_ST_CHAR: case DB_ST_SHORT: case DB_ST_INT: case DB_ST_INT64:
							{
								Setting.Value.QWord = 0;		
							} break;
						case DB_ST_ASCIIZ:
							{
								Setting.Value.pAnsii = (char *) mir_realloc(Setting.Value.pAnsii, set.BlobLength + 1);
								file.Read(Setting.Value.pAnsii, hSetting + sizeof(set) + set.NameLength + 1, set.BlobLength);
								Setting.Value.pAnsii[set.BlobLength] = 0;
								mir_utf8decode(Setting.Value.pAnsii, NULL);
								Setting.Value.Length = DB_INVALIDPARAM;
							} break;
						case DB_ST_UTF8:
							{
								Setting.Value.pUTF8 = (char *) mir_realloc(Setting.Value.pUTF8, set.BlobLength + 1);
								file.Read(Setting.Value.pUTF8, hSetting + sizeof(set) + set.NameLength + 1, set.BlobLength);
								Setting.Value.pUTF8[set.BlobLength] = 0;	
								Setting.Value.Length = set.BlobLength + 1;
							} break;
						case DB_ST_WCHAR:
							{
								char * buffer = (char *) mir_realloc(Setting.Value.pWide, set.BlobLength + 1);
								file.Read(buffer, hSetting + sizeof(set) + set.NameLength + 1, set.BlobLength);
								buffer[set.BlobLength] = 0;
								Setting.Value.pWide = mir_utf8decodeW(buffer);
								mir_free(buffer);
								Setting.Value.Length = DB_INVALIDPARAM;								
							} break;
						case DB_ST_BLOB:
							{
								Setting.Value.pBlob = (unsigned char *) mir_realloc(Setting.Value.pBlob, set.BlobLength);
								file.Read(Setting.Value.pBlob, hSetting + sizeof(set) + set.NameLength + 1, set.BlobLength);								
								Setting.Value.Length = set.BlobLength;
							} break;
					}
				} break;
			case DB_ST_WCHAR:
				{
					switch (Setting.Type)
					{
						case DB_ST_BYTE: case DB_ST_WORD: case DB_ST_DWORD: case DB_ST_QWORD: case DB_ST_BOOL: 
						case DB_ST_CHAR: case DB_ST_SHORT: case DB_ST_INT: case DB_ST_INT64:
							{
								Setting.Value.QWord = 0;		
							} break;
						case DB_ST_ASCIIZ:
							{
								wchar_t * buffer = (wchar_t *) mir_realloc(Setting.Value.pAnsii, set.BlobLength + sizeof(wchar_t));
								file.Read(buffer, hSetting + sizeof(set) + set.NameLength + 1, set.BlobLength);
								buffer[set.BlobLength / sizeof(wchar_t)] = 0;
								Setting.Value.pAnsii = mir_u2a(Setting.Value.pWide);
								mir_free(buffer);
								Setting.Value.Length = set.BlobLength / sizeof(wchar_t) + 1;
							} break;
						case DB_ST_UTF8:
							{
								wchar_t * buffer = (wchar_t *) mir_realloc(Setting.Value.pUTF8, set.BlobLength + sizeof(wchar_t));
								file.Read(buffer, hSetting + sizeof(set) + set.NameLength + 1, set.BlobLength);
								buffer[set.BlobLength / sizeof(wchar_t)] = 0;	
								Setting.Value.pUTF8 = mir_utf8encodeW(buffer);
								mir_free(buffer);
								Setting.Value.Length = DB_INVALIDPARAM;
							} break;
						case DB_ST_WCHAR:
							{
								Setting.Value.pWide = (wchar_t *) mir_realloc(Setting.Value.pWide, set.BlobLength + sizeof(wchar_t));
								file.Read(Setting.Value.pWide, hSetting + sizeof(set) + set.NameLength + 1, set.BlobLength);
								Setting.Value.pWide[set.BlobLength / sizeof(wchar_t)] = 0;	
								Setting.Value.Length = set.BlobLength / sizeof(wchar_t) + 1;						
							} break;
						case DB_ST_BLOB:
							{
								Setting.Value.pBlob = (unsigned char *) mir_realloc(Setting.Value.pBlob, set.BlobLength);
								file.Read(Setting.Value.pBlob, hSetting + sizeof(set) + set.NameLength + 1, set.BlobLength);								
								Setting.Value.Length = set.BlobLength;
							} break;
					}
				} break;
			case DB_ST_BLOB:
				{
					switch (Setting.Type)
					{
						case DB_ST_BYTE: case DB_ST_WORD: case DB_ST_DWORD: case DB_ST_QWORD: case DB_ST_BOOL: 
						case DB_ST_CHAR: case DB_ST_SHORT: case DB_ST_INT: case DB_ST_INT64:
							{
								Setting.Value.QWord = 0;		
							} break;
						case DB_ST_ASCIIZ: case DB_ST_WCHAR: case DB_ST_UTF8:
							{
								Setting.Value.Length = 0;
								if (Setting.Value.pBlob)
									mir_free(Setting.Value.pBlob);

								Setting.Value.pBlob = NULL;
							} break;
						case DB_ST_BLOB:
							{
								Setting.Value.pBlob = (unsigned char *) mir_realloc(Setting.Value.pBlob, set.BlobLength);
								file.Read(Setting.Value.pBlob, hSetting + sizeof(set) + set.NameLength + 1, set.BlobLength);								
								Setting.Value.Length = set.BlobLength;
							} break;
					}
				} break;

		}
	}


	if (Setting.Descriptor)
	{
		Setting.Descriptor->Entry = set.Entry;
		Setting.Descriptor->FoundInEntry = set.Entry;
		Setting.Descriptor->Flags = DB_SDF_FoundEntryValid;

		Setting.Descriptor->pszSettingName = (char *) mir_realloc(Setting.Descriptor->pszSettingName, set.NameLength + 1);
		file.Read(Setting.Descriptor->pszSettingName, hSetting + sizeof(set), set.NameLength + 1);		
		Setting.Descriptor->pszSettingName[set.NameLength] = 0;
	}

	m_Sync.EndRead();

	return set.Type;
}




TDBSettingIterationHandle CSettings::IterationInit(TDBSettingIterFilter & Filter)
{
	m_Sync.BeginWrite();





	unsigned int i = 0;

	while ((i < m_IterAllocSize) && (m_Iterations[i] != NULL))
		i++;

	if (i == m_IterAllocSize)
	{
		PSettingIteration * backup = m_Iterations;
		m_IterAllocSize = m_IterAllocSize << 1;
		m_Iterations = new PSettingIteration[m_IterAllocSize];
		memset(m_Iterations, 0, sizeof(PSettingIteration*) * m_IterAllocSize);
		memcpy(m_Iterations, backup, sizeof(PSettingIteration*) * (m_IterAllocSize >> 1));
		delete [] backup;
	}


	std::queue<TDBEntryHandle> entries;
	entries.push(Filter.hEntry);

	CSettingsTree * tree = getSettingsTree(Filter.hEntry);

	if (tree == NULL)
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	if (Filter.hEntry != 0)
	{	
		unsigned int entryflags = m_Entries.getFlags(Filter.hEntry);

		if (entryflags == DB_INVALIDPARAM)
		{
			m_Sync.EndWrite();
			return DB_INVALIDPARAM;
		}
		
		if ((entryflags & DB_EF_IsVirtual) && ((Filter.Options & DB_SIFO_NoPrimaryVirtualLookup) == 0))
		{	// virtual lookup
			TDBEntryHandle e = m_Entries.VirtualGetParent(Filter.hEntry);
			if (e != DB_INVALIDPARAM)
				entries.push(e);
			
		}

		if (Filter.Options & DB_SIFO_SearchSubEntries)
		{
			TDBEntryIterFilter filter = {0};
			
			filter.cbSize = sizeof(filter);
			filter.hParentEntry = Filter.hEntry;

			if ((entryflags & DB_EF_IsGroup) && !((Filter.Options & DB_SIFO_SearchOutOfGroups) == DB_SIFO_SearchOutOfGroups))
			{	// only groups
				filter.fHasFlags = DB_EF_IsGroup;
			}

			TDBEntryIterationHandle iter = m_Entries.IterationInit(filter);
			if ((iter != 0) && (iter != DB_INVALIDPARAM))
			{
				TDBEntryHandle e = m_Entries.IterationNext(iter);
				while ((e != 0) && (e != DB_INVALIDPARAM))
				{
					entries.push(e);				
					
					if (!((Filter.Options & DB_SIFO_NoVirtualLookup) == DB_SIFO_NoVirtualLookup))
					{
						e = m_Entries.VirtualGetParent(e);
						if (e != DB_INVALIDPARAM)
							entries.push(e);		
					}
					
					e = m_Entries.IterationNext(iter);
				}

			}
		}

		if (Filter.Options & DB_SIFO_SearchParents)
		{
			TDBEntryHandle p;
			p = m_Entries.getParent(Filter.hEntry);			

			while ((p != 0) && (p != DB_INVALIDPARAM) && (p != m_Entries.getRootEntry()) && ((m_Entries.getFlags(p) & DB_EF_IsGroup) == (entryflags & DB_EF_IsGroup)))
			{
				entries.push(p);

				if (!((Filter.Options & DB_SDF_NoVirtualLookup) == DB_SDF_NoVirtualLookup))
				{
					TDBEntryHandle e = m_Entries.VirtualGetParent(p);
					if (e != DB_INVALIDPARAM)
						entries.push(e);		
				}
				
				p = m_Entries.getParent(p);
			}
		}
		

		if (Filter.Options & DB_SIFO_RootHasStandard)
		{
			entries.push(m_Entries.getRootEntry());
		}
	}

	for (unsigned int j = 0; j < Filter.ExtraCount; ++j)
	{
		entries.push(Filter.ExtraEntries[j]);
	}

	PSettingIteration iter = new TSettingIteration;
	iter->Filter = Filter;
	iter->FilterNameStartLength = 0;
	if (Filter.NameStart)
	{
		unsigned int l = strlen(Filter.NameStart);
		iter->Filter.NameStart = new char[l + 1];
		memcpy(iter->Filter.NameStart, Filter.NameStart, l + 1);
		iter->FilterNameStartLength = l;
	}

	TSettingKey key;
	key.Hash = 0;
	TDBEntryHandle entry = entries.front();
	entries.pop();

	CSettingsTree::iterator * tmp = new CSettingsTree::iterator(tree->LowerBound(key));
	tmp->setManaged();
	iter->Heap = new TSettingsHeap(*tmp, TSettingsHeap::ITForward, true);
	
	while (!entries.empty())
	{
		tree = getSettingsTree(entries.front());
		entries.pop();
		if (tree != NULL)
		{
			tmp = new CSettingsTree::iterator(tree->LowerBound(key));
			tmp->setManaged();
			iter->Heap->Insert(*tmp);
		}
	}
			
	iter->Frame = new std::queue<TSettingIterationResult>;
	m_Iterations[i] = iter;

	m_Sync.EndWrite();
	return i + 1;
}


typedef struct TSettingIterationHelper {
		TDBSettingHandle Handle;
		CSettingsTree * Tree;
		unsigned short NameLen;
		char * Name;
	} TSettingIterationHelper;

TDBSettingHandle CSettings::IterationNext(TDBSettingIterationHandle Iteration)
{
	m_Sync.BeginRead();

	if (Iteration == 0)
		return 0;

	if ((Iteration > m_IterAllocSize) || (m_Iterations[Iteration - 1] == NULL))
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
	}

	PSettingIteration iter = m_Iterations[Iteration - 1];

	while (iter->Frame->empty() && iter->Heap->Top())
	{
		while (iter->Heap->Top() && iter->Heap->Top().wasDeleted())
			iter->Heap->Pop();

		if (iter->Heap->Top())
		{
			unsigned int h = iter->Heap->Top().Key().Hash;
			std::queue<TSettingIterationHelper> q;
			TSettingIterationHelper help;
			help.NameLen = 0;
			help.Name = NULL;
			
			help.Handle = iter->Heap->Top().Data();
			help.Tree = (CSettingsTree *) iter->Heap->Top().Tree();
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
		
				if (help.NameLen == 0)
				{
					if (help.Tree->getEntry() == 0)
						m_SettingsFile.Read(&help.NameLen, help.Handle + offsetof(TSetting, NameLength), sizeof(unsigned short));
					else
						m_PrivateFile.Read(&help.NameLen, help.Handle + offsetof(TSetting, NameLength), sizeof(unsigned short));
				}
				
				if (help.Name == NULL)
				{
					help.Name = new char[help.NameLen + 1];
					if (help.Tree->getEntry() == 0)
						m_SettingsFile.Read(help.Name, help.Handle + sizeof(TSetting), help.NameLen + 1);
					else
						m_PrivateFile.Read(help.Name, help.Handle + sizeof(TSetting), help.NameLen + 1);
				}


				q.push(help);
				while (q.front().Handle != help.Handle)
				{
					TSettingIterationHelper tmp;
					tmp = q.front();
					q.pop();

					if (tmp.NameLen == 0)
					{
						if (tmp.Tree->getEntry() == 0)
							m_SettingsFile.Read(&tmp.NameLen, tmp.Handle + offsetof(TSetting, NameLength), sizeof(unsigned short));
						else
							m_PrivateFile.Read(&tmp.NameLen, tmp.Handle + offsetof(TSetting, NameLength), sizeof(unsigned short));
					}

					if (tmp.NameLen != help.NameLen)
					{
						q.push(tmp);
					} else {
						if (tmp.Name == NULL)
						{
							tmp.Name = new char[tmp.NameLen + 1];
							if (tmp.Tree->getEntry() == 0)
								m_SettingsFile.Read(tmp.Name, tmp.Handle + sizeof(TSetting), tmp.NameLen + 1);
							else
								m_PrivateFile.Read(tmp.Name, tmp.Handle + sizeof(TSetting), tmp.NameLen + 1);
						}

						if (strcmp(tmp.Name, help.Name) != 0)
						{
							q.push(tmp);
						} else {
							delete [] tmp.Name;
						}
					}
				}

				// namefilter
				if ((iter->Filter.NameStart == NULL) || ((iter->FilterNameStartLength >= help.NameLen) && (memcmp(iter->Filter.NameStart, help.Name, iter->FilterNameStartLength) == 0)))
				{
					TSettingIterationResult tmp;			
					if (help.Tree->getEntry() == 0)
						help.Handle |= cSettingsFileFlag;

					tmp.Handle = help.Handle;
					tmp.Entry = help.Tree->getEntry();
					tmp.Name = help.Name;
					tmp.NameLen = help.NameLen;
					iter->Frame->push(tmp);
				} else {
					delete [] help.Name;
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
			iter->Filter.Descriptor->Entry = res.Entry;
			iter->Filter.Descriptor->pszSettingName = (char *) mir_realloc(iter->Filter.Descriptor->pszSettingName, res.NameLen + 1);
			memcpy(iter->Filter.Descriptor->pszSettingName, res.Name, res.NameLen + 1);
			iter->Filter.Descriptor->Flags = DB_SDF_FoundEntryValid;
			iter->Filter.Descriptor->FoundInEntry = res.Entry;
		}
		if (iter->Filter.Setting)
		{
			if ((iter->Filter.Setting->Type & DB_STF_VariableLength) && (iter->Filter.Setting->Value.pBlob))
			{
				mir_free(iter->Filter.Setting->Value.pBlob);
				iter->Filter.Setting->Value.pBlob = NULL;
			}
			iter->Filter.Setting->Type = 0;

			ReadSetting(*iter->Filter.Setting, res.Handle);
		}

		delete [] res.Name;
	}

	m_Sync.EndRead();

	return res.Handle;
}
unsigned int CSettings::IterationClose(TDBSettingIterationHandle Iteration)
{
	m_Sync.BeginWrite();

	if ((Iteration > m_IterAllocSize) || (Iteration == 0) || (m_Iterations[Iteration - 1] == NULL))
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	if (m_Iterations[Iteration - 1]->Filter.NameStart)
		delete [] m_Iterations[Iteration - 1]->Filter.NameStart;

	if (m_Iterations[Iteration - 1]->Filter.Descriptor)
	{
		mir_free(m_Iterations[Iteration - 1]->Filter.Descriptor->pszSettingName);
		m_Iterations[Iteration - 1]->Filter.Descriptor->pszSettingName = NULL;
	}
	if (m_Iterations[Iteration - 1]->Filter.Setting)
	{
		if (m_Iterations[Iteration - 1]->Filter.Setting->Descriptor)
		{
			mir_free(m_Iterations[Iteration - 1]->Filter.Setting->Descriptor->pszSettingName);
			m_Iterations[Iteration - 1]->Filter.Setting->Descriptor->pszSettingName = NULL;
		}	
		
		if (m_Iterations[Iteration - 1]->Filter.Setting->Type & DB_STF_VariableLength)
		{
			mir_free(m_Iterations[Iteration - 1]->Filter.Setting->Value.pBlob);
			m_Iterations[Iteration - 1]->Filter.Setting->Value.pBlob = NULL;
		}
	}

	delete m_Iterations[Iteration - 1]->Frame;
	delete m_Iterations[Iteration - 1]->Heap;
	delete m_Iterations[Iteration - 1];
	m_Iterations[Iteration - 1] = NULL;

	m_Sync.EndWrite();
	return 0;
}