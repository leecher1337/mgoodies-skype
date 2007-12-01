#include "Settings.h"
#include <queue>
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
	char * str = (char *) malloc(Length + 1);
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

	free(str);
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

}

CSettings::~CSettings()
{
	std::map<TDBEntryHandle, CSettingsTree*>::iterator i = m_SettingsMap.begin();

	while (i != m_SettingsMap.end())
	{
		delete i->second;
		++i;
	}

	m_SettingsMap.clear();
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
		return DB_INVALIDPARAM;


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
		TDBEntryHandle p,n;
		p = m_Entries.getParent(Descriptor.Entry);
		n = m_Entries.getParent(Descriptor.Entry);

		while ((n != 0) && (n != DB_INVALIDPARAM))
		{
			entries.push(p);

			if (!((Descriptor.Flags & DB_SDF_NoVirtualLookup) == DB_SDF_NoVirtualLookup))
			{
				p = m_Entries.VirtualGetParent(p);
				if (p != DB_INVALIDPARAM)
					entries.push(p);		
			}
			
			p = n;
			n = m_Entries.getParent(Descriptor.Entry);
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

	char * str = (char *) malloc(set.NameLength + 1);
	file.Read(str, hSetting + sizeof(set), set.NameLength + 1);
	
	tree->_DeleteSetting(Hash(str, set.NameLength), hSetting);
	
	if (set.Type & DB_STF_VariableLength)
		file.Free(hSetting, sizeof(set) + set.NameLength + 1 + set.AllocSize);
	else
		file.Free(hSetting, sizeof(set) + set.NameLength + 1);

	m_Sync.EndWrite();

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

	if (hSetting & cSettingsFileFlag)
	{
		file = m_SettingsFile;
		hSetting = hSetting & ~cSettingsFileFlag;
		fileflag = true;
	}

	TSetting set = {0};
	if (hSetting == 0)
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	} else {		
		file.Read(&set, hSetting, sizeof(set));

		if (set.Signature != cSettingSignature)
		{
			m_Sync.EndWrite();
			return DB_INVALIDPARAM;
		}
	}

	CSettingsTree * tree = getSettingsTree(set.Entry);
	if (tree == NULL)
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	if (((Setting.Type & DB_STF_VariableLength) == 0) && (set.Type & DB_STF_VariableLength))
	{ // shrink setting (variable size->fixed size)
		file.Free(hSetting + sizeof(set) + set.NameLength + 1, set.AllocSize);
	}

	if ((Setting.Type & DB_STF_VariableLength) && ((set.Type & DB_STF_VariableLength) == 0))
	{ // trick it
		set.AllocSize = 0;
	}

	if (Setting.Type & DB_STF_VariableLength)
	{
		unsigned int neededblob = Setting.Value.Length;
		if (Setting.Type == DB_ST_WCHAR)
			neededblob = neededblob * sizeof(wchar_t);

		if (set.AllocSize < neededblob)
		{
			char * name = (char *) malloc(set.NameLength + 1);
			file.Read(name, hSetting + sizeof(set), set.NameLength + 1);
			unsigned int hash = Hash(name, set.NameLength);

			file.Free(hSetting, sizeof(set) + set.NameLength + 1 + set.AllocSize);
			unsigned int tmp = file.Alloc(sizeof(set) + set.NameLength + 1 + neededblob);

			tree->_ChangeSetting(hash, hSetting, tmp);
			hSetting = tmp;

			file.Write(name, hSetting + sizeof(set), set.NameLength + 1);

			free(name);

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
		if (hset)
			return DB_INVALIDPARAM;
		else
			return 1;
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

		if ((Setting.Descriptor->pszSettingName) && (Setting.Descriptor->sSettingNameLength))
		{
			if (Setting.Descriptor->sSettingNameLength < set.NameLength + 1)
			{
				file.Read(Setting.Descriptor->pszSettingName, hSetting + sizeof(set), Setting.Descriptor->sSettingNameLength);
				Setting.Descriptor->pszSettingName[Setting.Descriptor->sSettingNameLength - 1] = 0;
			} else {
				file.Read(Setting.Descriptor->pszSettingName, hSetting + sizeof(set), set.NameLength + 1);
			}
		}
		Setting.Descriptor->sSettingNameLength = set.NameLength + 1;
	}

	m_Sync.EndRead();

	return set.Type;
}