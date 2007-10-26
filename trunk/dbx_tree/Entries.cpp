#include "Entries.h"

__forceinline bool TEntryKey::operator <  (const TEntryKey & Other) const
{
	if (Level != Other.Level) return Level < Other.Level;
	if (Parent != Other.Parent) return Parent < Other.Parent;
	if (Entry != Other.Entry) return Entry < Other.Entry;
	return false;
}

__forceinline bool TEntryKey::operator == (const TEntryKey & Other) const
{
	return (Level == Other.Level) && (Parent == Other.Parent) && (Entry == Other.Entry);
}

__forceinline bool TEntryKey::operator >  (const TEntryKey & Other) const
{	
	if (Level != Other.Level) return Level > Other.Level;
	if (Parent != Other.Parent) return Parent > Other.Parent;
	if (Entry != Other.Entry) return Entry > Other.Entry;
	return false;
}

CEntries::CEntries(CFileAccess & FileAccess, CMultiReadExclusiveWriteSynchronizer & Synchronize, unsigned int RootNode)
: CFileBTree(FileAccess, RootNode),
	m_Sync(Synchronize)
{

}

CEntries::~CEntries()
{

}

TEntryHandle CEntries::getParent(TEntryHandle hEntry)
{
	TEntry entry;

	m_Sync.BeginRead();	
	m_FileAccess.Read(&entry, hEntry, sizeof(entry));
	m_Sync.EndRead();

	if (entry.Signature != cEntrySignature)
		return -1;

	if (entry.Flags & DB_EF_IsVirtual)
		return getParent(entry.VParent);
	
	return entry.ParentEntry;
}
TEntryHandle CEntries::setParent(TEntryHandle hEntry, TEntryHandle hParent)
{
	return 0;	
}
unsigned int CEntries::getChildCount(TEntryHandle hEntry)
{
	TEntry entry;
	
	m_Sync.BeginRead();
	m_FileAccess.Read(&entry, hEntry, sizeof(entry));
	m_Sync.EndRead();

	if (entry.Signature != cEntrySignature)
		return -1;
	
	return entry.ChildCount;
}
TEntryHandle CEntries::getFirstChild(TEntryHandle hParent)
{
	TEntry entry;

	m_Sync.BeginRead();
	m_FileAccess.Read(&entry, hParent, sizeof(entry));

	if (entry.Signature != cEntrySignature)
	{
		m_Sync.EndRead();
		return -1;
	}

	TEntryKey key;

	key.Level = entry.Level + 1;
	key.Parent = hParent;
	key.Entry = 0;

	iterator it = LowerBound(key);
	
	if ((!it) || (it.Key().Parent != hParent))
	{
		m_Sync.EndRead();	
		return 0;
	}

	m_Sync.EndRead();
	return it.Key().Entry;
}
TEntryHandle CEntries::getLastChild(TEntryHandle hParent)
{
	TEntry entry;

	m_Sync.BeginRead();
	m_FileAccess.Read(&entry, hParent, sizeof(entry));

	if (entry.Signature != cEntrySignature)
	{
		m_Sync.EndRead();
		return -1;
	}

	TEntryKey key;

	key.Level = entry.Level + 1;
	key.Parent = hParent;
	key.Entry = 0xFFFFFFFF;

	iterator it = UpperBound(key);
	
	if ((!it) || (it.Key().Parent != hParent))
	{
		m_Sync.EndRead();	
		return 0;
	}

	m_Sync.EndRead();
	return it.Key().Entry;
}
TEntryHandle CEntries::getNextSilbing(TEntryHandle hEntry)
{
	TEntry entry;

	m_Sync.BeginRead();
	m_FileAccess.Read(&entry, hEntry, sizeof(entry));

	if (entry.Signature != cEntrySignature)
	{
		m_Sync.EndRead();
		return -1;
	}

	TEntryKey key;

	key.Level = entry.Level;
	key.Parent = entry.ParentEntry;
	key.Entry = hEntry + 1;

	iterator it = LowerBound(key);
	
	if ((!it) || (it.Key().Parent != entry.ParentEntry))
	{
		m_Sync.EndRead();
		return 0;
	}

	m_Sync.EndRead();
	return it.Key().Entry;
}
TEntryHandle CEntries::getPrevSilbing(TEntryHandle hEntry)
{
	TEntry entry;

	m_Sync.BeginRead();
	m_FileAccess.Read(&entry, hEntry, sizeof(entry));

	if (entry.Signature != cEntrySignature)
	{
		m_Sync.EndRead();
		return -1;
	}

	TEntryKey key;

	key.Level = entry.Level;
	key.Parent = entry.ParentEntry;
	key.Entry = hEntry - 1;

	iterator it = UpperBound(key);
	
	if ((!it) || (it.Key().Parent != entry.ParentEntry))
	{
		m_Sync.EndRead();
		return 0;
	}

	m_Sync.EndRead();
	return it.Key().Entry;
}

TEntryHandle CEntries::CreateEntry(TEntryHandle hParent, unsigned int Flags)
{
	TEntry entry;

	m_Sync.BeginWrite();
	m_FileAccess.Read(&entry, hParent, sizeof(entry));

	if (entry.Signature != cEntrySignature)
	{
		m_Sync.EndWrite();
		return -1;
	}

	entry.ChildCount++;
	entry.Flags = entry.Flags | DB_EF_HasChilds;
	m_FileAccess.Write(&entry, hParent, sizeof(entry));

	unsigned int hentry = m_FileAccess.Alloc(sizeof(TEntry));
	TEntryKey key;
	
	entry.Level++;
	entry.ParentEntry = hParent;
	entry.VParent = 0;
	entry.Flags = Flags;
	entry.Settings = 0;
	entry.Events = 0;
	entry.ChildCount = 0;
	entry.EventCount = 0;

	m_FileAccess.Write(&entry, hentry, sizeof(entry));

	key.Level = entry.Level;
	key.Parent = hParent;
	key.Entry = hentry;

	Insert(key, TEmpty());

	m_Sync.EndWrite();
	return hentry;
}
unsigned int CEntries::DeleteEntry(TEntryHandle hEntry)
{
	/// TODO delete settings and events
	TEntry entry;

	m_Sync.BeginWrite();
	m_FileAccess.Read(&entry, hEntry, sizeof(entry));

	if (entry.Signature != cEntrySignature)
	{
		m_Sync.EndWrite();
		return -1;
	}

	m_FileAccess.Free(hEntry, sizeof(TEntry));

	TEntryKey key;

	key.Level = entry.Level;
	key.Parent = entry.ParentEntry;
	key.Entry = hEntry;

	Delete(key);

	m_FileAccess.Read(&entry, key.Parent, sizeof(entry));

	entry.ChildCount--;
	if (entry.ChildCount == 0)
		entry.Flags = entry.Flags & (~DB_EF_HasChilds);

	m_FileAccess.Write(&entry, key.Parent, sizeof(entry));	

	m_Sync.EndWrite();
	return 0;
}