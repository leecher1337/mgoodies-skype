#include "Virtuals.h"
#include "Entries.h"

__forceinline bool TVirtualKey::operator <  (const TVirtualKey & Other) const
{
	if (RealEntry != Other.RealEntry) return RealEntry < Other.RealEntry;
	if (Virtual != Other.Virtual) return Virtual < Other.Virtual;
	return false;
}

__forceinline bool TVirtualKey::operator == (const TVirtualKey & Other) const
{
	return (RealEntry == Other.RealEntry) && (Virtual == Other.Virtual);
}

__forceinline bool TVirtualKey::operator >  (const TVirtualKey & Other) const
{	
	if (RealEntry != Other.RealEntry) return RealEntry > Other.RealEntry;
	if (Virtual != Other.Virtual) return Virtual > Other.Virtual;
	return false;
}

CVirtuals::CVirtuals(CFileAccess & FileAccess, CMultiReadExclusiveWriteSynchronizer & Synchronize, unsigned int RootNode)
: CFileBTree(FileAccess, RootNode),
	m_Sync(Synchronize)
{

}

CVirtuals::~CVirtuals()
{

}

TDBEntryHandle CVirtuals::DeleteRealEntry(TDBEntryHandle hRealEntry)
{
	TDBEntryHandle result;
	TVirtualKey key;
	TEntry entry;
	bool copies = false;

	key.RealEntry = hRealEntry;
	key.Virtual = 0;

	m_Sync.BeginWrite();

	iterator i = LowerBound(key);
	result = i.Key().RealEntry;
	i.setManaged();
	Delete(i);

	while ((i) && (i.Key().RealEntry == hRealEntry))
	{
		key = i.Key();
		Delete(i);

		key.RealEntry = result;		
		Insert(key, TEmpty());		

		entry.VParent = result;
		m_FileAccess.Write(&entry.VParent, result + offsetof(TEntry, VParent), sizeof(TDBEntryHandle));
	
		copies = true;
	}

	m_FileAccess.Read(&entry.Flags, result + offsetof(TEntry, Flags), sizeof(unsigned int));
	entry.Flags = entry.Flags & ~(DB_EF_HasVirtuals | DB_EF_IsVirtual);
	if (copies)
		entry.Flags |= DB_EF_HasVirtuals;

	m_FileAccess.Write(&entry.Flags, result + offsetof(TEntry, Flags), sizeof(unsigned int));

	m_Sync.EndWrite();

	return result;
}

unsigned int CVirtuals::InsertVirtual(TDBEntryHandle hRealEntry, TDBEntryHandle hVirtual)
{
	m_Sync.BeginWrite();

	TVirtualKey key;
	key.RealEntry = hRealEntry;
	key.Virtual = hVirtual;

	Insert(key, TEmpty());

	m_Sync.EndWrite();

	return 0;
}
void CVirtuals::DeleteVirtual(TDBEntryHandle hRealEntry, TDBEntryHandle hVirtual)
{
	m_Sync.BeginWrite();

	TVirtualKey key;
	key.RealEntry = hRealEntry;
	key.Virtual = hVirtual;

	Delete(key);

	m_Sync.EndWrite();
}
TDBEntryHandle CVirtuals::getParent(TDBEntryHandle hVirtual)
{
	TEntry entry;

	m_Sync.BeginRead();
	m_FileAccess.Read(&entry, hVirtual, sizeof(entry));
	m_Sync.EndRead();

	if ((entry.Signature != cEntrySignature) && ((entry.Flags & DB_EF_IsVirtual) == 0))
		return DB_INVALIDPARAM;
	
	return entry.VParent;
}
TDBEntryHandle CVirtuals::getFirst(TDBEntryHandle hRealEntry)
{
	TEntry entry;

	m_Sync.BeginRead();
	m_FileAccess.Read(&entry, hRealEntry, sizeof(entry));

	if ((entry.Signature != cEntrySignature) || ((entry.Flags & DB_EF_HasVirtuals) == 0))
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
	}
	
	TVirtualKey key;
	key.RealEntry = hRealEntry;
	key.Virtual = 0;

	iterator i = LowerBound(key);

	if ((i) && (i.Key().RealEntry == hRealEntry))
		key.Virtual = i.Key().Virtual;
	else
		key.Virtual = 0;

	m_Sync.EndRead();

	return key.Virtual;
}
TDBEntryHandle CVirtuals::getNext(TDBEntryHandle hVirtual)
{
	TEntry entry;

	m_Sync.BeginRead();
	m_FileAccess.Read(&entry, hVirtual, sizeof(entry));

	if ((entry.Signature != cEntrySignature) || ((entry.Flags & DB_EF_IsVirtual) == 0))
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
	}
	
	TVirtualKey key;
	key.RealEntry = entry.VParent;
	key.Virtual = hVirtual + 1;

	iterator i = LowerBound(key);

	
	if ((i) && (i.Key().RealEntry == entry.VParent))
		key.Virtual = i.Key().Virtual;
	else
		key.Virtual = 0;

	m_Sync.EndRead();

	return key.Virtual;
}