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






CVirtuals::CVirtuals(CFileAccess & FileAccess, CMultiReadExclusiveWriteSynchronizer & Synchronize, unsigned int RootNode)
: CFileBTree(FileAccess, RootNode),
	m_Sync(Synchronize)
{

}

CVirtuals::~CVirtuals()
{

}

TDBEntryHandle CVirtuals::_DeleteRealEntry(TDBEntryHandle hRealEntry)
{
	TDBEntryHandle result;
	TVirtualKey key;
	TEntry entry;
	bool copies = false;

	key.RealEntry = hRealEntry;
	key.Virtual = 0;

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

	return result;
}

unsigned int CVirtuals::_InsertVirtual(TDBEntryHandle hRealEntry, TDBEntryHandle hVirtual)
{
	TVirtualKey key;
	key.RealEntry = hRealEntry;
	key.Virtual = hVirtual;

	Insert(key, TEmpty());

	return 0;
}
void CVirtuals::_DeleteVirtual(TDBEntryHandle hRealEntry, TDBEntryHandle hVirtual)
{
	TVirtualKey key;
	key.RealEntry = hRealEntry;
	key.Virtual = hVirtual;

	Delete(key);
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




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CEntries::CEntries(CFileAccess & FileAccess, CMultiReadExclusiveWriteSynchronizer & Synchronize, TDBEntryHandle RootEntry, unsigned int EntryRoot, unsigned int VirtualRoot)
: CFileBTree(FileAccess, EntryRoot),
	m_Sync(Synchronize),
	m_Virtuals(FileAccess, Synchronize, VirtualRoot)
{
	m_RootEntry = RootEntry;
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


CVirtuals::TOnRootChanged & CEntries::sigVirtualRootChanged()
{
	return m_Virtuals.sigRootChanged();
}


unsigned int CEntries::_getSettingsRoot(TDBEntryHandle hEntry)
{
	TEntry entry;

	m_FileAccess.Read(&entry + offsetof(TEntry, Signature), hEntry + offsetof(TEntry, Signature), sizeof(unsigned int));

	if (entry.Signature != cEntrySignature)
		return DB_INVALIDPARAM;

	m_FileAccess.Read(&entry + offsetof(TEntry, Settings), hEntry + offsetof(TEntry, Settings), sizeof(unsigned int));
	
	return entry.Settings;
}
unsigned int CEntries::_setSettingsRoot(TDBEntryHandle hEntry, unsigned int NewRoot)
{
	TEntry entry;

	m_FileAccess.Read(&entry +  + offsetof(TEntry, Signature), hEntry + offsetof(TEntry, Signature), sizeof(unsigned int));

	if (entry.Signature != cEntrySignature)
		return DB_INVALIDPARAM;
	
	entry.Settings = NewRoot;
	m_FileAccess.Write(&entry +  + offsetof(TEntry, Settings), hEntry + offsetof(TEntry, Settings), sizeof(unsigned int));

	return 0;
}







TDBEntryHandle CEntries::getRootEntry()
{
	return m_RootEntry;
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
	entry.Flags = entry.Flags | DB_EF_HasChildren;
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

unsigned int CEntries::DeleteEntry(TDBEntryHandle hEntry)
{
	TEntry entry, parent;
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
		TDBEntryHandle newreal = m_Virtuals._DeleteRealEntry(hEntry);
		TEntry real;

		m_FileAccess.Read(&real, newreal, sizeof(real));
		real.EventCount = entry.EventCount;
		real.Events = entry.Events;
		real.Settings = entry.Settings;

		m_FileAccess.Write(&real, newreal, sizeof(real));
	} else {
		// TODO delete settings and events
	}

	if (entry.Flags & DB_EF_HasChildren) // keep the children
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

	m_FileAccess.Free(hEntry, sizeof(TEntry));

	key.Level = entry.Level;
	key.Parent = entry.ParentEntry;
	key.Entry = hEntry;

	Delete(key);

	parent.ChildCount--;
	if (parent.ChildCount == 0)
		entry.Flags = entry.Flags & (~DB_EF_HasChildren);

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

	if (entry.Flags & DB_EF_HasChildren)
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
	iter->filter = Filter;
	iter->q = new std::deque<iterator *>;

	TEntryKey key;
	key.Level = entry.Level + 1;
	key.Parent = Filter.hParentEntry;
	key.Entry = 0;

	iterator * tmp = new iterator(LowerBound(key));
	tmp->setManaged();
	iter->q->push_back(tmp);
		
	m_Iterations[i] = iter;

	m_Sync.EndWrite();
	return i + 1;
}
TDBEntryHandle CEntries::IterationNext(TDBEntryIterationHandle Iteration)
{
	m_Sync.BeginRead();

	if (Iteration == 0)
		return 0;

	if ((Iteration > m_IterAllocSize) || (m_Iterations[Iteration - 1] == NULL))
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
	}

	PEntryIteration iter = m_Iterations[Iteration - 1];

	if ((!iter->q) || (iter->q->empty()))
	{
		IterationClose(Iteration);
		m_Sync.EndRead();
		return 0;
	}

	TDBEntryHandle result = 0;
	
	TEntry entry = {0};
	TEntryKey key = {0};
	
	do {
		
		if (iter->filter.Options & DB_EIFO_DEPTHFIRST)
		{
			iterator * candidate = iter->q->back();
			iterator * tmp = NULL;

			if (!candidate->wasDeleted())
			{
				result = candidate->Key().Entry;
				
				m_FileAccess.Read(&entry.Flags, result + offsetof(TEntry, Flags), sizeof(unsigned int)); 
				if (entry.Flags & DB_EF_HasChildren)
				{
					key.Level = candidate->Key().Level + 1;
					key.Parent = result;
					key.Entry = 0;

					tmp = new iterator(LowerBound(key));
					tmp->setManaged();
				}
			}

			key = candidate->Key();
			++(*candidate);
			if ((!candidate) || (candidate->Key().Parent != key.Parent))
			{
				iter->q->pop_back();
				delete candidate;
			}

			if (tmp)
				iter->q->push_back(tmp);
		} else { // breath first search
			iterator * candidate = iter->q->front();
			if (!candidate->wasDeleted())
			{
				result = candidate->Key().Entry;

				m_FileAccess.Read(&entry.Flags, result + offsetof(TEntry, Flags), sizeof(unsigned int)); 
				if (entry.Flags & DB_EF_HasChildren)
				{
					key.Level = candidate->Key().Level + 1;
					key.Parent = result;
					key.Entry = 0;

					iterator * tmp = new iterator(LowerBound(key));
					tmp->setManaged();
					iter->q->push_back(tmp);
				}
			}

			key = candidate->Key();
			++(*candidate);
			if ((!candidate) || (candidate->Key().Parent != key.Parent))
			{
				iter->q->pop_front();
				delete candidate;
			}
		}

/*		if ((result != 0) && ((iter->filter.fHasFlags != 0) || (iter->filter.fDontHasFlags != 0)))
			m_FileAccess.Read(&entry.Flags, result + offsetof(TEntry, Flags), sizeof(unsigned int));
	*/		

	} while ((result == 0) && (!iter->q->empty()) &&
	        ((iter->filter.fHasFlags == 0)     || ((entry.Flags & iter->filter.fHasFlags) != 0)) && 
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

	while (!m_Iterations[Iteration - 1]->q->empty())
	{
		delete m_Iterations[Iteration - 1]->q->front();
		m_Iterations[Iteration - 1]->q->pop_front();
	}

	delete m_Iterations[Iteration - 1]->q;
	delete m_Iterations[Iteration - 1];
	m_Iterations[Iteration - 1] = NULL;

	return 0;
}



TDBEntryHandle CEntries::VirtualCreate(TDBEntryHandle hRealEntry, TDBEntryHandle hParent)
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
	parent.Flags = entry.Flags | DB_EF_HasChildren;
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

	m_Virtuals._InsertVirtual(hRealEntry, hentry);

	m_Sync.EndWrite();
	return hentry;
}

TDBEntryHandle CEntries::VirtualGetParent(TDBEntryHandle hVirtual)
{
	return m_Virtuals.getParent(hVirtual);
}
TDBEntryHandle CEntries::VirtualGetFirst(TDBEntryHandle hRealEntry)
{
	return m_Virtuals.getFirst(hRealEntry);
}
TDBEntryHandle CEntries::VirtualGetNext(TDBEntryHandle hVirtual)
{	
	return m_Virtuals.getNext(hVirtual);
}