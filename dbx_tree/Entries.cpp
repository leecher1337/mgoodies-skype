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

CEntries::CEntries(CFileAccess & FileAccess, CMultiReadExclusiveWriteSynchronizer & Synchronize, CVirtuals & Virtuals, unsigned int RootNode)
: CFileBTree(FileAccess, RootNode),
	m_Sync(Synchronize),
	m_Virtuals(Virtuals)
{
	m_IterAllocSize = 1;
	m_Iterations = new PEntryIteration[1];
}

CEntries::~CEntries()
{
	for (unsigned int i = 0; i < m_IterAllocSize; i++)
	{
		if (m_Iterations[i])
			IterationClose(i + 1);
	}
	delete [] m_Iterations;


}

TDBEntryHandle CEntries::getParent(TDBEntryHandle hEntry)
{
	TEntry entry;

	m_Sync.BeginRead();	
	m_FileAccess.Read(&entry, hEntry, sizeof(entry));
	m_Sync.EndRead();

	if (entry.Signature != cEntrySignature)
		return DB_INVALIDPARAM;
	
	return entry.ParentEntry;
}
TDBEntryHandle CEntries::setParent(TDBEntryHandle hEntry, TDBEntryHandle hParent)
{
	TEntry entry, parententry;
	
	m_Sync.BeginWrite();
	m_FileAccess.Read(&entry, hEntry, sizeof(entry));

	if (entry.Signature != cEntrySignature)
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	m_FileAccess.Read(&parententry, hParent, sizeof(parententry));

	if (parententry.Signature != cEntrySignature)
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	TEntryKey key;

	key.Level = entry.Level;
	key.Parent = entry.ParentEntry;
	key.Entry = hEntry;
	Delete(key);

	key.Level = parententry.Level + 1;
	key.Parent = hParent;
	Insert(key, TEmpty());

	m_Sync.EndWrite();
	/// TODO raise event

	return entry.ParentEntry;
}

unsigned int CEntries::getChildCount(TDBEntryHandle hEntry)
{
	TEntry entry;
	
	m_Sync.BeginRead();
	m_FileAccess.Read(&entry, hEntry, sizeof(entry));
	m_Sync.EndRead();

	if (entry.Signature != cEntrySignature)
		return DB_INVALIDPARAM;
	
	return entry.ChildCount;
}
TDBEntryHandle CEntries::getFirstChild(TDBEntryHandle hParent)
{
	TEntry entry;

	m_Sync.BeginRead();
	m_FileAccess.Read(&entry, hParent, sizeof(entry));

	if (entry.Signature != cEntrySignature)
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
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
TDBEntryHandle CEntries::getLastChild(TDBEntryHandle hParent)
{
	TEntry entry;

	m_Sync.BeginRead();
	m_FileAccess.Read(&entry, hParent, sizeof(entry));

	if (entry.Signature != cEntrySignature)
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
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
TDBEntryHandle CEntries::getNextSilbing(TDBEntryHandle hEntry)
{
	TEntry entry;

	m_Sync.BeginRead();
	m_FileAccess.Read(&entry, hEntry, sizeof(entry));

	if (entry.Signature != cEntrySignature)
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
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
TDBEntryHandle CEntries::getPrevSilbing(TDBEntryHandle hEntry)
{
	TEntry entry;

	m_Sync.BeginRead();
	m_FileAccess.Read(&entry, hEntry, sizeof(entry));

	if (entry.Signature != cEntrySignature)
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
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

unsigned int CEntries::getFlags(TDBEntryHandle hEntry)
{
	TEntry entry;

	m_Sync.BeginRead();
	m_FileAccess.Read(&entry, hEntry, sizeof(entry));
	m_Sync.EndRead();

	if (entry.Signature != cEntrySignature)
		return DB_INVALIDPARAM;

	return entry.Flags;
}

TDBEntryHandle CEntries::CreateEntry(TDBEntryHandle hParent, unsigned int Flags)
{
	TEntry entry;

	m_Sync.BeginWrite();
	m_FileAccess.Read(&entry, hParent, sizeof(entry));

	if (entry.Signature != cEntrySignature)
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
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

TDBEntryHandle CEntries::CreateVirtualEntry(TDBEntryHandle hRealEntry, TDBEntryHandle hParent)
{
	TEntry entry, parent;

	m_Sync.BeginWrite();

	m_FileAccess.Read(&entry, hRealEntry, sizeof(entry));
	
	if ((entry.Signature != cEntrySignature) || (entry.Flags & DB_EF_IsGroup))
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	m_FileAccess.Read(&parent, hParent, sizeof(parent));
	
	if (parent.Signature != cEntrySignature)
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	if (entry.Flags & DB_EF_IsVirtual)
	{
		hRealEntry = entry.VParent;
		m_FileAccess.Read(&entry.Flags, hRealEntry + offsetof(TEntry, Flags), sizeof(unsigned int));
	}

	if ((entry.Flags & DB_EF_HasVirtuals) == 0)
	{
		entry.Flags |= DB_EF_HasVirtuals;
		m_FileAccess.Write(&entry.Flags, hRealEntry + offsetof(TEntry, Flags), sizeof(unsigned int));
	}


	parent.ChildCount++;
	parent.Flags = entry.Flags | DB_EF_HasChilds;
	m_FileAccess.Write(&parent, hParent, sizeof(entry));

	unsigned int hentry = m_FileAccess.Alloc(sizeof(TEntry));
	TEntryKey key;
	
	parent.Level++;
	parent.ParentEntry = hParent;
	parent.VParent = hRealEntry;
	parent.Flags = DB_EF_IsVirtual;
	parent.Settings = 0;
	parent.Events = 0;
	parent.ChildCount = 0;
	parent.EventCount = 0;

	m_FileAccess.Write(&parent, hentry, sizeof(parent));

	key.Level = parent.Level;
	key.Parent = hParent;
	key.Entry = hentry;

	Insert(key, TEmpty());

	m_Virtuals.InsertVirtual(hRealEntry, hentry);

	m_Sync.EndWrite();
	return hentry;
}
unsigned int CEntries::DeleteEntry(TDBEntryHandle hEntry)
{
	TEntry entry, parent, z = {0};
	TEntryKey key;

	m_Sync.BeginWrite();
	m_FileAccess.Read(&entry, hEntry, sizeof(entry));

	if (entry.Signature != cEntrySignature)
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}
	
	m_FileAccess.Read(&parent, entry.ParentEntry, sizeof(TEntry));

	if (entry.Flags & DB_EF_HasVirtuals)
	{
		// move virtuals and make one of them real
		TDBEntryHandle newreal = m_Virtuals.DeleteRealEntry(hEntry);
		TEntry real;

		m_FileAccess.Read(&real, newreal, sizeof(real));
		real.EventCount = entry.EventCount;
		real.Events = entry.Events;
		real.Settings = entry.Settings;

		m_FileAccess.Write(&real, newreal, sizeof(real));
	} else {
		// TODO delete settings and events
	}

	if (entry.Flags & DB_EF_HasChilds) // keep the children
	{
		key.Level = entry.Level + 1;
		key.Parent = hEntry;
		key.Entry = 0;

		iterator i = LowerBound(key);
		i.setManaged();

		key.Level = entry.Level;
		key.Parent = entry.ParentEntry;
		while ((i) && (i.Key().Parent == hEntry))
		{
			key.Entry = i.Key().Entry;
			Delete(i);
			Insert(key, TEmpty());
			
			m_FileAccess.Write(&entry.ParentEntry, key.Entry + offsetof(TEntry, ParentEntry), sizeof(TDBEntryHandle));

			parent.ChildCount++;
		}
	}

	m_FileAccess.Write(&z, hEntry, sizeof(z));
	m_FileAccess.Free(hEntry, sizeof(TEntry));

	key.Level = entry.Level;
	key.Parent = entry.ParentEntry;
	key.Entry = hEntry;

	Delete(key);

	parent.ChildCount--;
	if (parent.ChildCount == 0)
		entry.Flags = entry.Flags & (~DB_EF_HasChilds);

	m_FileAccess.Write(&parent, entry.ParentEntry, sizeof(parent));	

	m_Sync.EndWrite();
	return 0;
}



TDBEntryIterationHandle CEntries::IterationInit(const TDBEntryIterFilter & Filter)
{
	TEntry entry;

	m_Sync.BeginWrite();
	m_FileAccess.Read(&entry, Filter.hParentEntry, sizeof(entry));

	if (entry.Signature != cEntrySignature)
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	if (entry.Flags & DB_EF_HasChilds)
	{
		m_Sync.EndWrite();
		return 0;	
	}

	unsigned int i = 0;

	while ((i < m_IterAllocSize) && (m_Iterations[i] != NULL))
		i++;

	if (i == m_IterAllocSize)
	{
		PEntryIteration * backup = m_Iterations;
		m_IterAllocSize = m_IterAllocSize << 1;
		m_Iterations = new PEntryIteration[m_IterAllocSize];
		memset(m_Iterations, 0, sizeof(PEntryIteration*) * m_IterAllocSize);
		memcpy(m_Iterations, backup, sizeof(PEntryIteration*) * (m_IterAllocSize >> 1));
		delete [] backup;
	}

	PEntryIteration iter = new TEntryIteration;
	TEntryKey key;
	
	key.Level = entry.Level;
	key.Parent = entry.ParentEntry;
	key.Entry = Filter.hParentEntry;
	
	iter->filter = Filter;
	iter->it = new iterator(Find(key));
	iter->it->setManaged();
	iter->MinLevel = entry.Level + 1;
	iter->EndMark = Filter.hParentEntry;		
	iter->LastEntryWithChilds = Filter.hParentEntry;
	
	m_Iterations[i] = iter;

	m_Sync.EndWrite();
	return i+1;
}
TDBEntryHandle CEntries::IterationNext(TDBEntryIterationHandle Iteration)
{
	m_Sync.BeginRead();

	if ((Iteration > m_IterAllocSize) || (Iteration == 0) || (m_Iterations[Iteration - 1] == NULL))
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
	}

	PEntryIteration iter = m_Iterations[Iteration - 1];

	if (!iter->it)
	{
		IterationClose(Iteration);
		m_Sync.EndRead();
		return 0;
	}

	TDBEntryHandle result = 0;
	
	TEntry entry = {0};
	TEntryKey key = {0};
	
	do {
		
		if (iter->filter.bDepthFirst)
		{
			m_FileAccess.Read(&entry.Flags, iter->it->Key().Entry + offsetof(TEntry, Flags), sizeof(unsigned int));

			if (entry.Flags & DB_EF_HasChilds) // Entry has childs. go down to first
			{
				key.Level = iter->it->Key().Level + 1;
				key.Parent = iter->it->Key().Entry;
				key.Entry = 0;

				(*iter->it) = LowerBound(key);
				result = iter->it->Key().Entry;

			} else { // Entry has no childs
				entry.Level = iter->it->Key().Level;
				entry.ParentEntry = iter->it->Key().Parent;

				++(*iter->it);	// go to next one

				while ((*iter->it) && (entry.ParentEntry != iter->it->Key().Parent))
				{
					result = entry.ParentEntry;
					m_FileAccess.Read(&entry.ParentEntry, entry.ParentEntry + offsetof(TEntry, ParentEntry), sizeof(TDBEntryHandle));
					key.Level = entry.Level + 1;
					key.Parent = entry.ParentEntry;
					key.Entry = result + 1;

					entry.Level--;

					(*iter->it) = LowerBound(key);
				}

				if ((*iter->it) && (iter->it->Key().Level >= iter->MinLevel))
				{
					result = iter->it->Key().Entry;
				} else {
					result = 0;
				}
				
			}
			
		} else { // breath first search
			if (iter->EndMark == iter->it->Key().Entry)  // go a level down
			{
				if (iter->LastEntryWithChilds != 0)
				{
					key.Level = iter->it->Key().Level + 1;
					key.Parent = iter->filter.hParentEntry;
					key.Entry = 0;

					(*iter->it) = LowerBound(key);
					
					key.Parent = iter->LastEntryWithChilds;
					key.Entry = 0xffffffff;				

					iter->EndMark = UpperBound(key).Key().Entry;

					iter->LastEntryWithChilds = 0;

					result = iter->it->Key().Entry;
				} else {
					result = 0;
				}
			} else {
				++(*iter->it);
				
				result = iter->it->Key().Entry;
			}

			if (result != 0)
			{
				m_FileAccess.Read(&entry.Flags, result + offsetof(TEntry, Flags), sizeof(unsigned int));

				if (entry.Flags & DB_EF_HasChilds)
				{ 
					if(iter->LastEntryWithChilds == 0)  // the first entry with childs in this level
						iter->filter.hParentEntry = result;
					
					iter->LastEntryWithChilds = result;					
				}
			}
		}

		if ((result != 0) && ((iter->filter.fHasFlags != 0) || (iter->filter.fDontHasFlags != 0)))
			m_FileAccess.Read(&entry.Flags, result + offsetof(TEntry, Flags), sizeof(unsigned int));

	} while ((result != 0) && 
	         ((iter->filter.fHasFlags == 0) || ((entry.Flags & iter->filter.fHasFlags) != 0)) && 
	         ((iter->filter.fDontHasFlags == 0) || (entry.Flags & iter->filter.fDontHasFlags) == 0));

	m_Sync.EndRead();

	return result;
}
unsigned int CEntries::IterationClose(TDBEntryIterationHandle Iteration)
{
	m_Sync.BeginWrite();

	if ((Iteration > m_IterAllocSize) || (Iteration == 0) || (m_Iterations[Iteration - 1] == NULL))
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	delete m_Iterations[Iteration - 1]->it;
	delete m_Iterations[Iteration - 1];
	m_Iterations[Iteration - 1] = NULL;

	return 0;
}