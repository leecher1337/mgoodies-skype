#include "Entries.h"



inline bool TVirtualKey::operator <  (const TVirtualKey & Other) const
{
	if (RealEntry != Other.RealEntry) return RealEntry < Other.RealEntry;
	if (Virtual != Other.Virtual) return Virtual < Other.Virtual;
	return false;
}

inline bool TVirtualKey::operator == (const TVirtualKey & Other) const
{
	return (RealEntry == Other.RealEntry) && (Virtual == Other.Virtual);
}

inline bool TVirtualKey::operator >  (const TVirtualKey & Other) const
{	
	if (RealEntry != Other.RealEntry) return RealEntry > Other.RealEntry;
	if (Virtual != Other.Virtual) return Virtual > Other.Virtual;
	return false;
}


inline bool TEntryKey::operator <  (const TEntryKey & Other) const
{
	if (Level != Other.Level) return Level < Other.Level;
	if (Parent != Other.Parent) return Parent < Other.Parent;
	if (Entry != Other.Entry) return Entry < Other.Entry;
	return false;
}

inline bool TEntryKey::operator == (const TEntryKey & Other) const
{
	return (Level == Other.Level) && (Parent == Other.Parent) && (Entry == Other.Entry);
}

inline bool TEntryKey::operator >  (const TEntryKey & Other) const
{	
	if (Level != Other.Level) return Level > Other.Level;
	if (Parent != Other.Parent) return Parent > Other.Parent;
	if (Entry != Other.Entry) return Entry > Other.Entry;
	return false;
}






CVirtuals::CVirtuals(CBlockManager & BlockManager, CMultiReadExclusiveWriteSynchronizer & Synchronize, TNodeRef RootNode)
: CFileBTree(BlockManager, RootNode, cVirtualNodeSignature),
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
	uint32_t sig = cEntrySignature;

	key.RealEntry = hRealEntry;
	key.Virtual = 0;

	iterator i = LowerBound(key);
	result = i.Key().Virtual;
	i.setManaged();
	Delete(i);

	while ((i) && (i.Key().RealEntry == hRealEntry))
	{
		key = i.Key();
		Delete(i);

		key.RealEntry = result;		
		Insert(key, TEmpty());		

		entry.VParent = result;
		m_BlockManager.WritePart(key.Virtual, &entry.VParent, offsetof(TEntry, VParent), sizeof(TDBEntryHandle));
	
		copies = true;
	}

	m_BlockManager.ReadPart(result, &entry.Flags, offsetof(TEntry, Flags), sizeof(uint32_t), sig);
	entry.Flags = entry.Flags & ~(DB_EF_HasVirtuals | DB_EF_IsVirtual);
	if (copies)
		entry.Flags |= DB_EF_HasVirtuals;

	m_BlockManager.WritePart(result, &entry.Flags, offsetof(TEntry, Flags), sizeof(uint32_t));

	return result;
}

bool CVirtuals::_InsertVirtual(TDBEntryHandle hRealEntry, TDBEntryHandle hVirtual)
{
	TVirtualKey key;
	key.RealEntry = hRealEntry;
	key.Virtual = hVirtual;

	Insert(key, TEmpty());

	return true;
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
	void* p = &entry;
	uint32_t size = sizeof(entry);
	uint32_t sig = cEntrySignature;

	m_Sync.BeginRead();
	if (!m_BlockManager.ReadBlock(hVirtual, p, size, sig) || 
	   ((entry.Flags & DB_EF_IsVirtual) == 0))
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
	}

	m_Sync.EndRead();	
	return entry.VParent;
}
TDBEntryHandle CVirtuals::getFirst(TDBEntryHandle hRealEntry)
{
	TEntry entry;
	void* p = &entry;
	uint32_t size = sizeof(entry);
	uint32_t sig = cEntrySignature;

	m_Sync.BeginRead();
	

	if (!m_BlockManager.ReadBlock(hRealEntry, p, size, sig) || 
	   ((entry.Flags & DB_EF_HasVirtuals) == 0))
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
	void* p = &entry;
	uint32_t size = sizeof(entry);
	uint32_t sig = cEntrySignature;

	m_Sync.BeginRead();
	
	if (!m_BlockManager.ReadBlock(hVirtual, p, size, sig) || 
	   ((entry.Flags & DB_EF_IsVirtual) == 0))
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

CEntries::CEntries(CBlockManager & BlockManager, CMultiReadExclusiveWriteSynchronizer & Synchronize, TDBEntryHandle RootEntry, TNodeRef EntryRoot, CVirtuals::TNodeRef VirtualRoot)
: CFileBTree(BlockManager, EntryRoot, cEntryNodeSignature),
	m_Sync(Synchronize),
	m_Virtuals(BlockManager, Synchronize, VirtualRoot)
{
	m_IterAllocSize = 1;
	m_Iterations = (PEntryIteration*)malloc(sizeof(PEntryIteration));

	if (RootEntry == 0)
		m_RootEntry = CreateRootEntry();
	else
		m_RootEntry = RootEntry;
	
}

CEntries::~CEntries()
{
	m_Sync.BeginWrite();
	for (unsigned int i = 0; i < m_IterAllocSize; i++)
	{
		if (m_Iterations[i])
			IterationClose(i + 1);
	}
	free(m_Iterations);
	m_Sync.EndWrite();
}

TDBEntryHandle CEntries::CreateRootEntry()
{
	TEntry entry = {0};
	TEntryKey key = {0};

	entry.Flags = DB_EF_IsGroup | DB_EF_IsRoot;
	key.Entry = m_BlockManager.CreateBlock(sizeof(entry), cEntrySignature);
	m_BlockManager.WriteBlock(key.Entry, &entry, sizeof(entry), cEntrySignature);
	Insert(key, TEmpty());
	return key.Entry;
}


CVirtuals::TOnRootChanged & CEntries::sigVirtualRootChanged()
{
	return m_Virtuals.sigRootChanged();
}


uint32_t CEntries::_getSettingsRoot(TDBEntryHandle hEntry)
{
	/*CSettingsTree::TNodeRef*/
	uint32_t set;
	uint32_t sig = cEntrySignature;

	if (!m_BlockManager.ReadPart(hEntry, &set, offsetof(TEntry, Settings), sizeof(set), sig))
		return DB_INVALIDPARAM;

	return set;
}
bool CEntries::_setSettingsRoot(TDBEntryHandle hEntry, /*CSettingsTree::TNodeRef*/ uint32_t NewRoot)
{
	uint32_t sig = cEntrySignature;

	if (!m_BlockManager.WritePartCheck(hEntry, &NewRoot, offsetof(TEntry, Settings), sizeof(NewRoot), sig))
		return false;
	
	return true;
}







TDBEntryHandle CEntries::getRootEntry()
{
	return m_RootEntry;
}

TDBEntryHandle CEntries::getParent(TDBEntryHandle hEntry)
{
	TDBEntryHandle par;
	uint32_t sig = cEntrySignature;

	m_Sync.BeginRead();	
	if (!m_BlockManager.ReadPart(hEntry, &par, offsetof(TEntry, ParentEntry), sizeof(par), sig))
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
	}

	m_Sync.EndRead();	
	return par;
}
TDBEntryHandle CEntries::setParent(TDBEntryHandle hEntry, TDBEntryHandle hParent)
{
	TEntry entry;
	void* pentry = &entry;
	uint32_t size = sizeof(TEntry);
	uint32_t sig = cEntrySignature;
	uint16_t cn, co;
	uint32_t fn, fo;
	uint16_t l;
	
	m_Sync.BeginWrite();
	if (!m_BlockManager.ReadBlock(hEntry, pentry, size, sig) ||
		  !m_BlockManager.ReadPart(hParent, &cn, offsetof(TEntry,ChildCount), sizeof(cn), sig) ||
			!m_BlockManager.ReadPart(entry.ParentEntry, &co, offsetof(TEntry, ChildCount), sizeof(co), sig) ||
			!m_BlockManager.ReadPart(hParent, &l, offsetof(TEntry, Level), sizeof(l), sig))
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	--co;
	++cn;

	m_BlockManager.WritePart(entry.ParentEntry, &co, offsetof(TEntry, ChildCount),sizeof(co));
	if (co == 0)
	{
		m_BlockManager.ReadPart(entry.ParentEntry, &fo, offsetof(TEntry, Flags), sizeof(fo), sig);
		fo = fo & ~DB_EF_HasChildren;
		m_BlockManager.WritePart(entry.ParentEntry, &fo, offsetof(TEntry, Flags), sizeof(fo));
	}

	TEntryKey key;

	key.Level = entry.Level;
	key.Parent = entry.ParentEntry;
	key.Entry = hEntry;
	Delete(key);

	++l;
	key.Level = l;
	key.Parent = hParent;
	Insert(key, TEmpty());

	
	m_BlockManager.WritePart(hEntry, &hParent, offsetof(TEntry, ParentEntry), sizeof(hParent));
	m_BlockManager.WritePart(hEntry, &l, offsetof(TEntry, Level), sizeof(l));
	m_BlockManager.WritePart(hParent, &cn, offsetof(TEntry, ChildCount), sizeof(cn));
	if (cn == 1)
	{
		m_BlockManager.ReadPart(hParent, &fn, offsetof(TEntry, Flags), sizeof(fn), sig);
		fn = fn | DB_EF_HasChildren;
		m_BlockManager.WritePart(hParent, &fn, offsetof(TEntry, Flags), sizeof(fn));
	}	
	
	m_Sync.EndWrite();

	/// TODO raise event

	return entry.ParentEntry;
}

unsigned int CEntries::getChildCount(TDBEntryHandle hEntry)
{
	uint32_t c;
	uint32_t sig = cEntrySignature;
	
	m_Sync.BeginRead();
	if (!m_BlockManager.ReadPart(hEntry, &c, offsetof(TEntry, ChildCount), sizeof(c), sig))
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
	}
	
	return c;
}
TDBEntryHandle CEntries::getFirstChild(TDBEntryHandle hParent)
{
	uint16_t l;
	uint32_t f;
	uint32_t sig = cEntrySignature;
	TDBEntryHandle result;

	m_Sync.BeginRead();
	if (!m_BlockManager.ReadPart(hParent, &f, offsetof(TEntry, Flags), sizeof(f), sig) ||
		  !m_BlockManager.ReadPart(hParent, &l, offsetof(TEntry, Level), sizeof(l), sig))
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
	}

	if ((f & DB_EF_HasChildren) == 0)
	{
		m_Sync.EndRead();
		return 0;
	}

	TEntryKey key;

	key.Level = l + 1;
	key.Parent = hParent;
	key.Entry = 0;

	iterator it = LowerBound(key);
	
	if ((!it) || (it.Key().Parent != hParent))
	{
		m_Sync.EndRead();	
		return 0;
	}
	result = it.Key().Entry;
	m_Sync.EndRead();

	return result;
}
TDBEntryHandle CEntries::getLastChild(TDBEntryHandle hParent)
{
	uint16_t l;
	uint32_t f;
	uint32_t sig = cEntrySignature;
	TDBEntryHandle result;

	m_Sync.BeginRead();
	if (!m_BlockManager.ReadPart(hParent, &f, offsetof(TEntry, Flags), sizeof(f), sig) ||
		  !m_BlockManager.ReadPart(hParent, &l, offsetof(TEntry, Level), sizeof(l), sig))
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
	}

	if ((f & DB_EF_HasChildren) == 0)
	{
		m_Sync.EndRead();
		return 0;
	}

	TEntryKey key;

	key.Level = l + 1;
	key.Parent = hParent;
	key.Entry = 0xFFFFFFFF;

	iterator it = UpperBound(key);
	
	if ((!it) || (it.Key().Parent != hParent))
	{
		m_Sync.EndRead();	
		return 0;
	}

	result = it.Key().Entry;
	m_Sync.EndRead();

	return result;
}
TDBEntryHandle CEntries::getNextSilbing(TDBEntryHandle hEntry)
{
	uint16_t l;
	uint32_t sig = cEntrySignature;
	TDBEntryHandle result, parent;

	m_Sync.BeginRead();
	if (!m_BlockManager.ReadPart(hEntry, &l, offsetof(TEntry, Level), sizeof(l), sig) ||
		  !m_BlockManager.ReadPart(hEntry, &parent, offsetof(TEntry, ParentEntry), sizeof(parent), sig))
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
	}

	TEntryKey key;

	key.Level = l;
	key.Parent = parent;
	key.Entry = hEntry + 1;

	iterator it = LowerBound(key);
	
	if ((!it) || (it.Key().Parent != parent))
	{
		m_Sync.EndRead();
		return 0;
	}
	result = it.Key().Entry;
	m_Sync.EndRead();

	return result;
}
TDBEntryHandle CEntries::getPrevSilbing(TDBEntryHandle hEntry)
{
	uint16_t l;
	uint32_t sig = cEntrySignature;
	TDBEntryHandle result, parent;

	m_Sync.BeginRead();
	if (!m_BlockManager.ReadPart(hEntry, &l, offsetof(TEntry, Level), sizeof(l), sig) ||
		  !m_BlockManager.ReadPart(hEntry, &parent, offsetof(TEntry, ParentEntry), sizeof(parent), sig))
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
	}

	TEntryKey key;

	key.Level = l;
	key.Parent = parent;
	key.Entry = hEntry - 1;

	iterator it = UpperBound(key);
	
	if ((!it) || (it.Key().Parent != parent))
	{
		m_Sync.EndRead();
		return 0;
	}
	result = it.Key().Entry;
	m_Sync.EndRead();

	return result;
}

unsigned int CEntries::getFlags(TDBEntryHandle hEntry)
{
	uint32_t f;
	uint32_t sig = cEntrySignature;

	m_Sync.BeginRead();
	if (!m_BlockManager.ReadPart(hEntry, &f, offsetof(TEntry, Flags), sizeof(f), sig))
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
	}

	m_Sync.EndRead();
	return f;
}

TDBEntryHandle CEntries::CreateEntry(TDBEntryHandle hParent, uint32_t Flags)
{
	TEntry entry = {0}, parent;
	void* pparent = &parent;
	uint32_t size = sizeof(entry);
	uint32_t sig = cEntrySignature;

	m_Sync.BeginWrite();
	if (!m_BlockManager.ReadBlock(hParent, pparent, size, sig))
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	TDBEntryHandle hentry = m_BlockManager.CreateBlock(sizeof(TEntry), cEntrySignature);
	TEntryKey key;
	
	entry.Level = parent.Level + 1;
	entry.ParentEntry = hParent;
	entry.Flags = Flags;
	
	m_BlockManager.WriteBlock(hentry, &entry, sizeof(entry), cEntrySignature);
	
	key.Level = entry.Level;
	key.Parent = hParent;
	key.Entry = hentry;

	Insert(key, TEmpty());
	
	if (parent.ChildCount == 0)
	{
		parent.Flags = entry.Flags | DB_EF_HasChildren;
		m_BlockManager.WritePart(hParent, &parent.Flags, offsetof(TEntry, Flags), sizeof(uint32_t));
	}
	++parent.ChildCount;
	m_BlockManager.WritePart(hParent, &parent.ChildCount, offsetof(TEntry, ChildCount), sizeof(uint16_t));
	
	m_Sync.EndWrite();
	return hentry;
}

unsigned int CEntries::DeleteEntry(TDBEntryHandle hEntry)
{
	TEntry entry;
	void* pentry = &entry;
	uint32_t size = sizeof(entry);
	uint32_t sig = cEntrySignature;
	uint16_t parentcc;
	uint32_t parentf;

	TEntryKey key;

	m_Sync.BeginWrite();
	if (!m_BlockManager.ReadBlock(hEntry, pentry, size, sig) ||
		  !m_BlockManager.ReadPart(entry.ParentEntry, &parentcc, offsetof(TEntry, ChildCount), sizeof(parentcc), sig) ||
			!m_BlockManager.ReadPart(entry.ParentEntry, &parentf, offsetof(TEntry, Flags), sizeof(parentf), sig))
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	if (entry.Flags & DB_EF_HasVirtuals)
	{
		// move virtuals and make one of them real
		TDBEntryHandle newreal = m_Virtuals._DeleteRealEntry(hEntry);
		
		m_BlockManager.WritePartCheck(newreal, &entry.EventCount, offsetof(TEntry, EventCount), sizeof(uint32_t), sig);
		m_BlockManager.WritePart(newreal, &entry.Events, offsetof(TEntry, Events), sizeof(uint32_t));
		m_BlockManager.WritePart(newreal, &entry.Settings, offsetof(TEntry, Settings), sizeof(/*CSettingsTree::TNodeRef*/ uint32_t));

	} else {
		// TODO delete settings and events
	}

	if (entry.Flags & DB_EF_HasChildren) // keep the children
	{
		parentf = parentf | DB_EF_HasChildren;
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
			
			m_BlockManager.WritePartCheck(key.Entry, &entry.ParentEntry, offsetof(TEntry, ParentEntry), sizeof(TDBEntryHandle), sig);

			++parentcc;
		}
	}

	key.Level = entry.Level;
	key.Parent = entry.ParentEntry;
	key.Entry = hEntry;
	Delete(key);

	m_BlockManager.DeleteBlock(hEntry);

	--parentcc;

	if (parentcc == 0)
		entry.Flags = entry.Flags & (~DB_EF_HasChildren);

	m_BlockManager.WritePartCheck(entry.ParentEntry, &parentcc, offsetof(TEntry, ChildCount), sizeof(parentcc), sig);
	m_BlockManager.WritePart(entry.ParentEntry, &parentf, offsetof(TEntry, Flags), sizeof(parentf));

	m_Sync.EndWrite();
	return 0;
}



TDBEntryIterationHandle CEntries::IterationInit(const TDBEntryIterFilter & Filter)
{
	uint16_t l;
	uint32_t f;
	uint32_t sig = cEntrySignature;

	m_Sync.BeginWrite();
	
	if (!m_BlockManager.ReadPart(Filter.hParentEntry, &l, offsetof(TEntry, Level), sizeof(l), sig) ||
	    !m_BlockManager.ReadPart(Filter.hParentEntry, &f, offsetof(TEntry, Flags), sizeof(f), sig))
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	if ((f & DB_EF_HasChildren) == 0)
	{
		m_Sync.EndWrite();
		return 0;	
	}

	unsigned int i = 0;

	while ((i < m_IterAllocSize) && (m_Iterations[i] != NULL))
		i++;

	if (i == m_IterAllocSize)
	{
		m_IterAllocSize = m_IterAllocSize << 1;
		m_Iterations = (PEntryIteration*) realloc(m_Iterations, sizeof(PEntryIteration*) * m_IterAllocSize);
	}

	PEntryIteration iter = new TEntryIteration;
	iter->filter = Filter;
	iter->q = new std::deque<iterator *>;

	TEntryKey key;
	key.Level = l + 1;
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
	
	uint32_t f = 0;
	uint32_t sig = cEntrySignature;
	TEntryKey key = {0};
	
	do {
		
		if (iter->filter.Options & DB_EIFO_DEPTHFIRST)
		{
			iterator * candidate = iter->q->back();
			iterator * tmp = NULL;

			if (!candidate->wasDeleted())
			{
				result = candidate->Key().Entry;
				
		
				if (m_BlockManager.ReadPart(result, &f, offsetof(TEntry, Flags), sizeof(f), sig) && 
					 (f & DB_EF_HasChildren))
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

				if (m_BlockManager.ReadPart(result, &f, offsetof(TEntry, Flags), sizeof(f), sig) && 
					 (f & DB_EF_HasChildren))
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
	        ((iter->filter.fHasFlags == 0)     || ((f & iter->filter.fHasFlags) != 0)) && 
	        ((iter->filter.fDontHasFlags == 0) || (f & iter->filter.fDontHasFlags) == 0));

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

	m_Sync.EndWrite();
	return 0;
}



TDBEntryHandle CEntries::VirtualCreate(TDBEntryHandle hRealEntry, TDBEntryHandle hParent)
{
	uint32_t f;
	uint32_t sig = cEntrySignature;

	m_Sync.BeginWrite();
	
	if (!m_BlockManager.ReadPart(hRealEntry, &f, offsetof(TEntry, Flags), sizeof(f), sig) || 
		 (f & DB_EF_IsGroup))
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	TDBEntryHandle result = CreateEntry(hParent, DB_EF_IsVirtual);
	if (result == DB_INVALIDPARAM)
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	if (f & DB_EF_IsVirtual)
	{		
		m_BlockManager.ReadPart(hRealEntry, &hRealEntry, offsetof(TEntry, VParent), sizeof(hRealEntry), sig);		
		m_BlockManager.ReadPart(hRealEntry, &f, offsetof(TEntry, Flags), sizeof(f), sig);
	}

	m_BlockManager.WritePart(result, &hRealEntry, offsetof(TEntry, VParent), sizeof(hRealEntry));

	if ((f & DB_EF_HasVirtuals) == 0)
	{
		f |= DB_EF_HasVirtuals;
		m_BlockManager.WritePart(hRealEntry, &f, offsetof(TEntry, Flags), sizeof(f));
	}

	m_Virtuals._InsertVirtual(hRealEntry, result);

	m_Sync.EndWrite();
	return result;
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