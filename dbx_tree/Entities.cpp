#include "Entities.h"

inline bool TVirtualKey::operator <  (const TVirtualKey & Other) const
{
	if (RealEntity != Other.RealEntity) return RealEntity < Other.RealEntity;
	if (Virtual != Other.Virtual) return Virtual < Other.Virtual;
	return false;
}

inline bool TVirtualKey::operator == (const TVirtualKey & Other) const
{
	return (RealEntity == Other.RealEntity) && (Virtual == Other.Virtual);
}

inline bool TVirtualKey::operator >  (const TVirtualKey & Other) const
{	
	if (RealEntity != Other.RealEntity) return RealEntity > Other.RealEntity;
	if (Virtual != Other.Virtual) return Virtual > Other.Virtual;
	return false;
}


inline bool TEntityKey::operator <  (const TEntityKey & Other) const
{
	if (Level != Other.Level) return Level < Other.Level;
	if (Parent != Other.Parent) return Parent < Other.Parent;
	if (Entity != Other.Entity) return Entity < Other.Entity;
	return false;
}

inline bool TEntityKey::operator == (const TEntityKey & Other) const
{
	return (Level == Other.Level) && (Parent == Other.Parent) && (Entity == Other.Entity);
}

inline bool TEntityKey::operator >  (const TEntityKey & Other) const
{	
	if (Level != Other.Level) return Level > Other.Level;
	if (Parent != Other.Parent) return Parent > Other.Parent;
	if (Entity != Other.Entity) return Entity > Other.Entity;
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

TDBTEntityHandle CVirtuals::_DeleteRealEntity(TDBTEntityHandle hRealEntity)
{
	TDBTEntityHandle result;
	TVirtualKey key;
	TEntity Entity;
	bool copies = false;
	uint32_t sig = cEntitySignature;

	key.RealEntity = hRealEntity;
	key.Virtual = 0;

	iterator i = LowerBound(key);
	result = i->Virtual;
	i.setManaged();
	Delete(*i);

	while ((i) && (i->RealEntity == hRealEntity))
	{
		key = *i;
		Delete(*i);

		key.RealEntity = result;		
		Insert(key);		

		Entity.VParent = result;
		m_BlockManager.WritePart(key.Virtual, &Entity.VParent, offsetof(TEntity, VParent), sizeof(TDBTEntityHandle));
	
		copies = true;
	}

	m_BlockManager.ReadPart(result, &Entity.Flags, offsetof(TEntity, Flags), sizeof(uint32_t), sig);
	Entity.Flags = Entity.Flags & ~(DBT_NF_HasVirtuals | DBT_NF_IsVirtual);
	if (copies)
		Entity.Flags |= DBT_NF_HasVirtuals;

	m_BlockManager.WritePart(result, &Entity.Flags, offsetof(TEntity, Flags), sizeof(uint32_t));

	return result;
}

bool CVirtuals::_InsertVirtual(TDBTEntityHandle hRealEntity, TDBTEntityHandle hVirtual)
{
	TVirtualKey key;
	key.RealEntity = hRealEntity;
	key.Virtual = hVirtual;

	Insert(key);

	return true;
}
void CVirtuals::_DeleteVirtual(TDBTEntityHandle hRealEntity, TDBTEntityHandle hVirtual)
{
	TVirtualKey key;
	key.RealEntity = hRealEntity;
	key.Virtual = hVirtual;

	Delete(key);
}
TDBTEntityHandle CVirtuals::getParent(TDBTEntityHandle hVirtual)
{
	TEntity Entity;
	void* p = &Entity;
	uint32_t size = sizeof(Entity);
	uint32_t sig = cEntitySignature;

	SYNC_BEGINREAD(m_Sync);
	if (!m_BlockManager.ReadBlock(hVirtual, p, size, sig) || 
	   ((Entity.Flags & DBT_NF_IsVirtual) == 0))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	SYNC_ENDREAD(m_Sync);	
	return Entity.VParent;
}
TDBTEntityHandle CVirtuals::getFirst(TDBTEntityHandle hRealEntity)
{
	TEntity Entity;
	void* p = &Entity;
	uint32_t size = sizeof(Entity);
	uint32_t sig = cEntitySignature;

	SYNC_BEGINREAD(m_Sync);
	

	if (!m_BlockManager.ReadBlock(hRealEntity, p, size, sig) || 
	   ((Entity.Flags & DBT_NF_HasVirtuals) == 0))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}
	
	TVirtualKey key;
	key.RealEntity = hRealEntity;
	key.Virtual = 0;

	iterator i = LowerBound(key);

	if ((i) && (i->RealEntity == hRealEntity))
		key.Virtual = i->Virtual;
	else
		key.Virtual = 0;

	SYNC_ENDREAD(m_Sync);

	return key.Virtual;
}
TDBTEntityHandle CVirtuals::getNext(TDBTEntityHandle hVirtual)
{
	TEntity Entity;
	void* p = &Entity;
	uint32_t size = sizeof(Entity);
	uint32_t sig = cEntitySignature;

	SYNC_BEGINREAD(m_Sync);
	
	if (!m_BlockManager.ReadBlock(hVirtual, p, size, sig) || 
	   ((Entity.Flags & DBT_NF_IsVirtual) == 0))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}
	
	TVirtualKey key;
	key.RealEntity = Entity.VParent;
	key.Virtual = hVirtual + 1;

	iterator i = LowerBound(key);
	
	if ((i) && (i->RealEntity == Entity.VParent))
		key.Virtual = i->Virtual;
	else
		key.Virtual = 0;

	SYNC_ENDREAD(m_Sync);

	return key.Virtual;
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CEntities::CEntities(CBlockManager & BlockManager, CMultiReadExclusiveWriteSynchronizer & Synchronize, TDBTEntityHandle RootEntity, TNodeRef EntityRoot, CVirtuals::TNodeRef VirtualRoot)
: CFileBTree(BlockManager, EntityRoot, cEntityNodeSignature),
	m_Sync(Synchronize),
	m_Virtuals(BlockManager, Synchronize, VirtualRoot),

	m_sigEntityDelete(),
	m_sigInternalDeleteEvents(),
	m_sigInternalDeleteSettings(),
	m_sigInternalMergeSettings(),
	m_sigInternalTransferEvents()
{
	if (RootEntity == 0)
		m_RootEntity = _CreateRootEntity();
	else
		m_RootEntity = RootEntity;
	
}

CEntities::~CEntities()
{

}

TDBTEntityHandle CEntities::_CreateRootEntity()
{
	TEntity Entity = {0};
	TEntityKey key = {0};

	Entity.Flags = DBT_NF_IsGroup | DBT_NF_IsRoot;
	key.Entity = m_BlockManager.CreateBlock(sizeof(Entity), cEntitySignature);
	m_BlockManager.WriteBlock(key.Entity, &Entity, sizeof(Entity), cEntitySignature);
	Insert(key);
	return key.Entity;
}

void CEntities::_InternalTransferContacts(TDBTEntityHandle OldAccount, TDBTEntityHandle NewAccount)
{
	uint32_t sig = cEntitySignature;

	TDBTEntityHandle acc;
	TEntityKey key = {0};
	iterator i = LowerBound(key);

	while (i)
	{
		sig = cEntitySignature;
		if (m_BlockManager.ReadPart(i->Entity, &acc, offsetof(TEntity, Account), sizeof(acc), sig) &&
			  (acc == OldAccount))
		{
			m_BlockManager.WritePart(i->Entity, &NewAccount, offsetof(TEntity, Account), sizeof(NewAccount));
		}

		++i;
	}
}

CVirtuals::TOnRootChanged & CEntities::sigVirtualRootChanged()
{
	return m_Virtuals.sigRootChanged();
}

CEntities::TOnEntityDelete &          CEntities::sigEntityDelete()
{
	return m_sigEntityDelete;
}
CEntities::TOnInternalDeleteEvents &   CEntities::_sigDeleteEvents()  
{
	return m_sigInternalDeleteEvents;
}
CEntities::TOnInternalDeleteSettings & CEntities::_sigDeleteSettings()
{
	return m_sigInternalDeleteSettings;
}
CEntities::TOnInternalMergeSettings &  CEntities::_sigMergeSettings() 
{
	return m_sigInternalMergeSettings;
}
CEntities::TOnInternalTransferEvents & CEntities::_sigTransferEvents()
{
	return m_sigInternalTransferEvents;
}


uint32_t CEntities::_getSettingsRoot(TDBTEntityHandle hEntity)
{
	/*CSettingsTree::TNodeRef*/
	uint32_t set;
	uint32_t sig = cEntitySignature;

	if (!m_BlockManager.ReadPart(hEntity, &set, offsetof(TEntity, Settings), sizeof(set), sig))
		return DBT_INVALIDPARAM;

	return set;
}
bool CEntities::_setSettingsRoot(TDBTEntityHandle hEntity, /*CSettingsTree::TNodeRef*/ uint32_t NewRoot)
{
	uint32_t sig = cEntitySignature;

	if (!m_BlockManager.WritePartCheck(hEntity, &NewRoot, offsetof(TEntity, Settings), sizeof(NewRoot), sig))
		return false;
	
	return true;
}

uint32_t CEntities::_getEventsRoot(TDBTEntityHandle hEntity)
{
	/*CEventsTree::TNodeRef*/
	uint32_t ev;
	uint32_t sig = cEntitySignature;

	if (!m_BlockManager.ReadPart(hEntity, &ev, offsetof(TEntity, Events), sizeof(ev), sig))
		return DBT_INVALIDPARAM;

	return ev;
}
bool CEntities::_setEventsRoot(TDBTEntityHandle hEntity, /*CEventsTree::TNodeRef*/ uint32_t NewRoot)
{
	uint32_t sig = cEntitySignature;

	if (!m_BlockManager.WritePartCheck(hEntity, &NewRoot, offsetof(TEntity, Events), sizeof(NewRoot), sig))
		return false;
	
	return true;
}

uint32_t CEntities::_getEventCount(TDBTEntityHandle hEntity)
{
	uint32_t res = 0;
	uint32_t sig = cEntitySignature;

	if (!m_BlockManager.ReadPart(hEntity, &res, offsetof(TEntity, EventCount), sizeof(res), sig))
		return DBT_INVALIDPARAM;
	
	return res;
}

uint32_t CEntities::_adjustEventCount(TDBTEntityHandle hEntity, int32_t Adjust)
{
	uint32_t sig = cEntitySignature;
	uint32_t c;

	if (m_BlockManager.ReadPart(hEntity, &c, offsetof(TEntity, EventCount), sizeof(c), sig))
	{
		if (((Adjust < 0) && ((uint32_t)(-Adjust) <= c)) || 
			  ((Adjust > 0) && ((0xffffffff - c) > (uint32_t)Adjust)))
		{
			c += Adjust;
			m_BlockManager.WritePart(hEntity, &c, offsetof(TEntity, EventCount), sizeof(c));
		}
	}

	return c;
}
CVirtuals & CEntities::_getVirtuals()
{
	return m_Virtuals;
}

TDBTEntityHandle CEntities::getRootEntity()
{
	return m_RootEntity;
}

TDBTEntityHandle CEntities::getParent(TDBTEntityHandle hEntity)
{
	TDBTEntityHandle par;
	uint32_t sig = cEntitySignature;

	SYNC_BEGINREAD(m_Sync);	
	if (!m_BlockManager.ReadPart(hEntity, &par, offsetof(TEntity, ParentEntity), sizeof(par), sig))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	SYNC_ENDREAD(m_Sync);	
	return par;
}
TDBTEntityHandle CEntities::setParent(TDBTEntityHandle hEntity, TDBTEntityHandle hParent)
{
	TEntity Entity;
	void* pEntity = &Entity;
	uint32_t size = sizeof(TEntity);
	uint32_t sig = cEntitySignature;
	uint16_t cn, co;
	uint32_t fn, fo;
	uint16_t l;
	
	SYNC_BEGINWRITE(m_Sync);
	if (!m_BlockManager.ReadBlock(hEntity, pEntity, size, sig) ||
		  !m_BlockManager.ReadPart(hParent, &cn, offsetof(TEntity,ChildCount), sizeof(cn), sig) ||
			!m_BlockManager.ReadPart(Entity.ParentEntity, &co, offsetof(TEntity, ChildCount), sizeof(co), sig) ||
			!m_BlockManager.ReadPart(hParent, &l, offsetof(TEntity, Level), sizeof(l), sig))
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	// update parents
	--co;
	++cn;

	m_BlockManager.WritePart(Entity.ParentEntity, &co, offsetof(TEntity, ChildCount),sizeof(co));
	if (co == 0)
	{
		m_BlockManager.ReadPart(Entity.ParentEntity, &fo, offsetof(TEntity, Flags), sizeof(fo), sig);
		fo = fo & ~DBT_NF_HasChildren;
		m_BlockManager.WritePart(Entity.ParentEntity, &fo, offsetof(TEntity, Flags), sizeof(fo));
	}
	
	m_BlockManager.WritePart(hParent, &cn, offsetof(TEntity, ChildCount), sizeof(cn));
	if (cn == 1)
	{
		m_BlockManager.ReadPart(hParent, &fn, offsetof(TEntity, Flags), sizeof(fn), sig);
		fn = fn | DBT_NF_HasChildren;
		m_BlockManager.WritePart(hParent, &fn, offsetof(TEntity, Flags), sizeof(fn));
	}	
	
	// update rest

	TEntityKey key;
	int dif = l - Entity.Level + 1;

	if (dif == 0) // no level difference, update only moved Entity
	{
		key.Entity = hEntity;
		key.Level = Entity.Level;
		key.Parent = Entity.ParentEntity;
		Delete(key);
		key.Parent = hParent;
		Insert(key);
		m_BlockManager.WritePart(hEntity, &key.Parent, offsetof(TEntity, ParentEntity), sizeof(key.Parent));
		
	} else {
		TDBTEntityIterFilter filter = {0};
		filter.cbSize = sizeof(filter);
		filter.Options = DBT_NIFO_OSC_AC | DBT_NIFO_OC_AC;

		TDBTEntityIterationHandle iter = IterationInit(filter, hEntity);

		key.Entity = IterationNext(iter);

		while ((key.Entity != 0) && (key.Entity != DBT_INVALIDPARAM))
		{
			if (m_BlockManager.ReadPart(key.Entity, &key.Parent, offsetof(TEntity, ParentEntity), sizeof(key.Parent), sig) &&
					m_BlockManager.ReadPart(key.Entity, &key.Level, offsetof(TEntity, Level), sizeof(key.Level), sig))
			{
				Delete(key);

				if (key.Entity == hEntity)
				{
					key.Parent = hParent;
					m_BlockManager.WritePart(key.Entity, &key.Parent, offsetof(TEntity, ParentEntity), sizeof(key.Parent));
				}
				
				key.Level = key.Level + dif;
				m_BlockManager.WritePart(key.Entity, &key.Level, offsetof(TEntity, Level), sizeof(key.Level));
				
				Insert(key);
			}
			key.Entity = IterationNext(iter);
		}

		IterationClose(iter);
	}

	SYNC_ENDWRITE(m_Sync);

	/// TODO raise event

	return Entity.ParentEntity;
}

uint32_t CEntities::getChildCount(TDBTEntityHandle hEntity)
{
	uint32_t c;
	uint32_t sig = cEntitySignature;
	
	SYNC_BEGINREAD(m_Sync);
	if (!m_BlockManager.ReadPart(hEntity, &c, offsetof(TEntity, ChildCount), sizeof(c), sig))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}
	
	SYNC_ENDREAD(m_Sync);
	return c;
}

uint32_t CEntities::getFlags(TDBTEntityHandle hEntity)
{
	uint32_t f;
	uint32_t sig = cEntitySignature;

	SYNC_BEGINREAD(m_Sync);
	if (!m_BlockManager.ReadPart(hEntity, &f, offsetof(TEntity, Flags), sizeof(f), sig))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	SYNC_ENDREAD(m_Sync);
	return f;
}

uint32_t CEntities::getAccount(TDBTEntityHandle hEntity)
{
	uint32_t f, a;
	uint32_t sig = cEntitySignature;

	SYNC_BEGINREAD(m_Sync);
	if (!m_BlockManager.ReadPart(hEntity, &f, offsetof(TEntity, Flags), sizeof(f), sig) ||
		  !m_BlockManager.ReadPart(hEntity, &a, offsetof(TEntity, Account), sizeof(a), sig))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	if (f & DBT_NF_IsVirtual)
		a = getAccount(a); // we can do this, because VParent and Account occupies the same memory
	else if (f & (DBT_NF_IsAccount | DBT_NF_IsGroup | DBT_NF_IsRoot))
		a = 0;

	SYNC_ENDREAD(m_Sync);
	return f;
}

TDBTEntityHandle CEntities::CreateEntity(const TDBTEntity & Entity)
{
	TEntity en = {0}, parent;
	void* pparent = &parent;
	uint32_t size = sizeof(en);
	uint32_t sig = cEntitySignature;

	SYNC_BEGINWRITE(m_Sync);
	if (!m_BlockManager.ReadBlock(Entity.hParentEntity, pparent, size, sig))
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	// check account specification
	if ((Entity.fFlags == 0) && (Entity.hAccountEntity != m_RootEntity)) // TODO disable root account thing, after conversion
	{
		uint32_t f = 0;
		if (!m_BlockManager.ReadPart(Entity.hAccountEntity, &f, offsetof(TEntity, Flags), sizeof(f), sig) ||
			  !(f & DBT_NF_IsAccount))
		{
			SYNC_ENDWRITE(m_Sync);
			return DBT_INVALIDPARAM;	
		}

		if (f & DBT_NF_IsVirtual)
		{
			en.Account = VirtualGetParent(Entity.hAccountEntity);
		} else {
			en.Account = Entity.hAccountEntity;
		}
	}		

	TDBTEntityHandle hEntity = m_BlockManager.CreateBlock(sizeof(TEntity), cEntitySignature);
	TEntityKey key;
	
	en.Level = parent.Level + 1;
	en.ParentEntity = Entity.hParentEntity;
	en.Flags = Entity.fFlags;
		
	m_BlockManager.WriteBlock(hEntity, &en, sizeof(TEntity), cEntitySignature);
	
	key.Level = en.Level;
	key.Parent = en.ParentEntity;
	key.Entity = hEntity;

	Insert(key);
	
	if (parent.ChildCount == 0)
	{
		parent.Flags = parent.Flags | DBT_NF_HasChildren;
		m_BlockManager.WritePart(Entity.hParentEntity, &parent.Flags, offsetof(TEntity, Flags), sizeof(uint32_t));
	}
	++parent.ChildCount;
	m_BlockManager.WritePart(Entity.hParentEntity, &parent.ChildCount, offsetof(TEntity, ChildCount), sizeof(uint16_t));
	
	SYNC_ENDWRITE(m_Sync);
	return hEntity;
}

unsigned int CEntities::DeleteEntity(TDBTEntityHandle hEntity)
{
	TEntity Entity;
	void* pEntity = &Entity;
	uint32_t size = sizeof(Entity);
	uint32_t sig = cEntitySignature;
	uint16_t parentcc;
	uint32_t parentf;

	TEntityKey key;

	SYNC_BEGINWRITE(m_Sync);
	if (!m_BlockManager.ReadBlock(hEntity, pEntity, size, sig) ||
		  !m_BlockManager.ReadPart(Entity.ParentEntity, &parentcc, offsetof(TEntity, ChildCount), sizeof(parentcc), sig) ||
			!m_BlockManager.ReadPart(Entity.ParentEntity, &parentf, offsetof(TEntity, Flags), sizeof(parentf), sig))
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	m_sigEntityDelete.emit(this, hEntity);

	if (Entity.Flags & DBT_NF_HasVirtuals)
	{
		// move virtuals and make one of them real
		TDBTEntityHandle newreal = m_Virtuals._DeleteRealEntity(hEntity);

		m_BlockManager.WritePartCheck(newreal, &Entity.EventCount, offsetof(TEntity, EventCount), sizeof(uint32_t), sig);		
		m_BlockManager.WritePart(newreal, &Entity.Events, offsetof(TEntity, Events), sizeof(uint32_t));

		m_sigInternalTransferEvents.emit(this, hEntity, newreal);
		m_sigInternalMergeSettings.emit(this, hEntity, newreal);

		if (Entity.Flags & DBT_NF_IsAccount)
		{
			_InternalTransferContacts(hEntity, newreal);
		}
	} else {
		m_sigInternalDeleteEvents.emit(this, hEntity);
		m_sigInternalDeleteSettings.emit(this, hEntity);

		if ((Entity.Flags & DBT_NF_IsAccount) && !(Entity.Flags & DBT_NF_IsVirtual))
		{
			_InternalTransferContacts(hEntity, m_RootEntity);
		}
	}

	key.Level = Entity.Level;
	key.Parent = Entity.ParentEntity;
	key.Entity = hEntity;
	Delete(key);
	
	if (Entity.Flags & DBT_NF_HasChildren) // keep the children
	{
		parentf = parentf | DBT_NF_HasChildren;
		parentcc += Entity.ChildCount;

		TDBTEntityIterFilter filter = {0};
		filter.cbSize = sizeof(filter);
		filter.Options = DBT_NIFO_OSC_AC | DBT_NIFO_OC_AC;

		TDBTEntityIterationHandle iter = IterationInit(filter, hEntity);
		if (iter != DBT_INVALIDPARAM)
		{
			IterationNext(iter);
			key.Entity = IterationNext(iter);
			
			while ((key.Entity != 0) && (key.Entity != DBT_INVALIDPARAM))
			{
				if (m_BlockManager.ReadPart(key.Entity, &key.Parent, offsetof(TEntity, ParentEntity), sizeof(key.Parent), sig) &&
					m_BlockManager.ReadPart(key.Entity, &key.Level, offsetof(TEntity, Level), sizeof(key.Level), sig))
				{
					Delete(key);

					if (key.Parent == hEntity)
					{
						key.Parent = Entity.ParentEntity;
						m_BlockManager.WritePart(key.Entity, &key.Parent, offsetof(TEntity, ParentEntity), sizeof(key.Parent));
					}
					
					key.Level--;
					m_BlockManager.WritePart(key.Entity, &key.Level, offsetof(TEntity, Level), sizeof(key.Level));
					Insert(key);

				}
				key.Entity = IterationNext(iter);
			}

			IterationClose(iter);
		}
	}

	m_BlockManager.DeleteBlock(hEntity); // we need this block to start iteration, delete it here
	--parentcc;

	if (parentcc == 0)
		Entity.Flags = Entity.Flags & (~DBT_NF_HasChildren);

	m_BlockManager.WritePartCheck(Entity.ParentEntity, &parentcc, offsetof(TEntity, ChildCount), sizeof(parentcc), sig);
	m_BlockManager.WritePart(Entity.ParentEntity, &parentf, offsetof(TEntity, Flags), sizeof(parentf));

	SYNC_ENDWRITE(m_Sync);
	return 0;
}



TDBTEntityIterationHandle CEntities::IterationInit(const TDBTEntityIterFilter & Filter, TDBTEntityHandle hParent)
{
	uint16_t l;
	uint32_t f;
	uint32_t sig = cEntitySignature;

	SYNC_BEGINREAD(m_Sync);
	
	if (!m_BlockManager.ReadPart(hParent, &l, offsetof(TEntity, Level), sizeof(l), sig) ||
	    !m_BlockManager.ReadPart(hParent, &f, offsetof(TEntity, Flags), sizeof(f), sig))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	PEntityIteration iter = new TEntityIteration;
	iter->filter = Filter;
	iter->q = new std::deque<TEntityIterationItem>;
	iter->parents = new std::deque<TEntityIterationItem>;
	iter->accounts = new std::deque<TEntityIterationItem>;
	iter->returned = new stdext::hash_set<TDBTEntityHandle>;
	iter->returned->insert(hParent);
	
	TEntityIterationItem it;
	it.Flags = f;
	it.Handle = hParent;
	it.Level = l;
	it.Options = Filter.Options & 0x000000ff;
	it.LookupDepth = 0;

	iter->q->push_back(it);

	SYNC_ENDREAD(m_Sync);

	return (TDBTEntityIterationHandle)iter;
}
TDBTEntityHandle CEntities::IterationNext(TDBTEntityIterationHandle Iteration)
{
	uint32_t sig = cEntitySignature;

	SYNC_BEGINREAD(m_Sync);

	PEntityIteration iter = (PEntityIteration)Iteration;
	TEntityIterationItem item;
	TDBTEntityHandle result = 0;

	if (iter->q->empty())
	{
		std::deque <TEntityIterationItem> * tmp = iter->q;
		iter->q = iter->parents;
		iter->parents = tmp;
	}

	if (iter->q->empty())
	{
		std::deque <TEntityIterationItem> * tmp = iter->q;
		iter->q = iter->accounts;
		iter->accounts = tmp;
	}

	if (iter->q->empty() && 
		  (iter->filter.Options & DBT_NIFO_GF_USEROOT) &&
			(iter->returned->find(m_RootEntity) == iter->returned->end()))
	{
		item.Handle = m_RootEntity;
		item.Level = 0;
		item.Options = 0;
		item.Flags = 0;
		item.LookupDepth = 255;

		iter->filter.Options = iter->filter.Options & ~DBT_NIFO_GF_USEROOT;

		iter->q->push_back(item);
	}

	if (iter->q->empty())
	{
		SYNC_ENDREAD(m_Sync);
		return 0;
	}
	
	do {
		item = iter->q->front();
		iter->q->pop_front();

		std::deque<TEntityIterationItem> tmp;
		TEntityIterationItem newitem;

		// children
		if ((item.Flags & DBT_NF_HasChildren) &&
		  	(item.Options & DBT_NIFO_OSC_AC))
		{
			TEntityKey key;
			key.Parent = item.Handle;
			key.Level = item.Level + 1;

			newitem.Level = item.Level + 1;
			newitem.LookupDepth = item.LookupDepth;
			newitem.Options = (iter->filter.Options / DBT_NIFO_OC_AC * DBT_NIFO_OSC_AC) & (DBT_NIFO_OSC_AC | DBT_NIFO_OSC_AO | DBT_NIFO_OSC_AOC | DBT_NIFO_OSC_AOP);

			if (iter->filter.Options & DBT_NIFO_GF_DEPTHFIRST)
			{
				key.Entity = 0xffffffff;

				iterator c = UpperBound(key);
				while ((c) && (c->Parent == item.Handle))
				{
					newitem.Handle = c->Entity;
					
					if ((iter->returned->find(newitem.Handle) == iter->returned->end()) &&
						  m_BlockManager.ReadPart(newitem.Handle, &newitem.Flags, offsetof(TEntity, Flags), sizeof(newitem.Flags), sig) &&
						  (((newitem.Flags & DBT_NF_IsGroup) == 0) || ((DBT_NF_IsGroup & iter->filter.fHasFlags) == 0))) // if we want only groups, we don't need to trace down Entities...
					{
						iter->q->push_front(newitem);
						iter->returned->insert(newitem.Handle);
					}

					--c;
				}
			} else {
				key.Entity = 0;

				iterator c = LowerBound(key);
				while ((c) && (c->Parent == item.Handle))
				{
					newitem.Handle = c->Entity;
					
					if ((iter->returned->find(newitem.Handle) == iter->returned->end()) &&
						  m_BlockManager.ReadPart(newitem.Handle, &newitem.Flags, offsetof(TEntity, Flags), sizeof(newitem.Flags), sig) &&
						  (((newitem.Flags & DBT_NF_IsGroup) == 0) || ((DBT_NF_IsGroup & iter->filter.fHasFlags) == 0))) // if we want only groups, we don't need to trace down Entities...
					{
						iter->q->push_back(newitem);
						iter->returned->insert(newitem.Handle);
					}

					++c;
				}

			}
		}

		// parent...
		if ((item.Options & DBT_NIFO_OSC_AP) && (item.Handle != m_RootEntity))
		{
			newitem.Handle = getParent(item.Handle);
			if ((iter->returned->find(newitem.Handle) == iter->returned->end()) &&
				  (newitem.Handle != DBT_INVALIDPARAM) &&
				  m_BlockManager.ReadPart(newitem.Handle, &newitem.Flags, offsetof(TEntity, Flags), sizeof(newitem.Flags), sig))
			{
				newitem.Level = item.Level - 1;
				newitem.LookupDepth = item.LookupDepth;
				newitem.Options = (iter->filter.Options / DBT_NIFO_OP_AC * DBT_NIFO_OSC_AC) & (DBT_NIFO_OSC_AC | DBT_NIFO_OSC_AP | DBT_NIFO_OSC_AO | DBT_NIFO_OSC_AOC | DBT_NIFO_OSC_AOP);

				if ((newitem.Flags & iter->filter.fDontHasFlags & DBT_NF_IsGroup) == 0) // if we don't want groups, stop it
				{
					iter->parents->push_back(newitem);
					iter->returned->insert(newitem.Handle);
				}
			}
		}

		// virtual lookup, original Entity is the next one
		if ((item.Flags & DBT_NF_IsVirtual) && 
			  (item.Options & DBT_NIFO_OSC_AO) && 
				(((iter->filter.Options >> 28) >= item.LookupDepth) || ((iter->filter.Options >> 28) == 0)))
		{
			newitem.Handle = VirtualGetParent(item.Handle);

			if ((iter->returned->find(newitem.Handle) == iter->returned->end()) &&
				  (newitem.Handle != DBT_INVALIDPARAM) &&
				   m_BlockManager.ReadPart(newitem.Handle, &newitem.Flags, offsetof(TEntity, Flags), sizeof(newitem.Flags), sig) &&
           m_BlockManager.ReadPart(newitem.Handle, &newitem.Level, offsetof(TEntity, Level), sizeof(newitem.Level), sig))
			{
				newitem.Options  = 0;
				if ((item.Options & DBT_NIFO_OSC_AOC) == DBT_NIFO_OSC_AOC)
					newitem.Options |= DBT_NIFO_OSC_AC;
				if ((item.Options & DBT_NIFO_OSC_AOP) == DBT_NIFO_OSC_AOP)
					newitem.Options |= DBT_NIFO_OSC_AP;

				newitem.LookupDepth = item.LookupDepth + 1;

				iter->q->push_front(newitem);
				iter->returned->insert(newitem.Handle);
			}
		}

		if (((iter->filter.fHasFlags & item.Flags) == iter->filter.fHasFlags) &&
		    ((iter->filter.fDontHasFlags & item.Flags) == 0))
		{
			result = item.Handle;
			
			// account lookup
			if (((item.Flags & (DBT_NF_IsAccount | DBT_NF_IsGroup | DBT_NF_IsRoot)) == 0) && 
			    ((item.Options & DBT_NIFO_OC_USEACCOUNT) == DBT_NIFO_OC_USEACCOUNT))
			{
				TDBTEntityHandle acc = item.Handle;
				if (item.Flags & DBT_NF_IsVirtual)
					acc = VirtualGetParent(item.Handle);

				acc = getAccount(acc);

				std::deque<TEntityIterationItem>::iterator acci = iter->accounts->begin();
				
				while ((acci != iter->accounts->end()) && (acc != 0))
				{
					if (acci->Handle == acc)
						acc = 0;
					acci++;
				}
				if ((acc != 0) &&
				    m_BlockManager.ReadPart(acc, &newitem.Flags, offsetof(TEntity, Flags), sizeof(newitem.Flags), sig) &&
            m_BlockManager.ReadPart(acc, &newitem.Level, offsetof(TEntity, Level), sizeof(newitem.Level), sig))
				{
					newitem.Options = 0;
					newitem.LookupDepth = 0;
					newitem.Handle = acc;
					iter->accounts->push_back(newitem);
				}
			}
		}

	} while ((result == 0) && !iter->q->empty());

	if (result == 0)
		result = IterationNext(Iteration);

	SYNC_ENDREAD(m_Sync);

	return result;
}
unsigned int CEntities::IterationClose(TDBTEntityIterationHandle Iteration)
{
//	SYNC_BEGINREAD(m_Sync); // no sync needed

	PEntityIteration iter = (PEntityIteration) Iteration;

	delete iter->q;
	delete iter->parents;
	delete iter->accounts;
	delete iter->returned;
	delete iter;

//	SYNC_ENDREAD(m_Sync);
	return 0;
}



TDBTEntityHandle CEntities::VirtualCreate(TDBTEntityHandle hRealEntity, TDBTEntityHandle hParent)
{
	uint32_t f;
	uint32_t sig = cEntitySignature;

	SYNC_BEGINWRITE(m_Sync);
	
	if (!m_BlockManager.ReadPart(hRealEntity, &f, offsetof(TEntity, Flags), sizeof(f), sig) || 
		  (f & (DBT_NF_IsGroup | DBT_NF_IsRoot)))
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	TDBTEntity entity = {0};
	entity.hParentEntity = hParent;
	entity.fFlags = DBT_NF_IsVirtual | (f & DBT_NF_IsAccount);
	entity.hAccountEntity = 0;

	TDBTEntityHandle result = CreateEntity(entity);
	if (result == DBT_INVALIDPARAM)
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	if (f & DBT_NF_IsVirtual)
	{		
		m_BlockManager.ReadPart(hRealEntity, &hRealEntity, offsetof(TEntity, VParent), sizeof(hRealEntity), sig);		
		m_BlockManager.ReadPart(hRealEntity, &f, offsetof(TEntity, Flags), sizeof(f), sig);
	}

	m_BlockManager.WritePart(result, &hRealEntity, offsetof(TEntity, VParent), sizeof(hRealEntity));

	if ((f & DBT_NF_HasVirtuals) == 0)
	{
		f |= DBT_NF_HasVirtuals;
		m_BlockManager.WritePart(hRealEntity, &f, offsetof(TEntity, Flags), sizeof(f));
	}

	m_Virtuals._InsertVirtual(hRealEntity, result);

	SYNC_ENDWRITE(m_Sync);
	return result;
}

TDBTEntityHandle CEntities::VirtualGetParent(TDBTEntityHandle hVirtual)
{
	return m_Virtuals.getParent(hVirtual);
}
TDBTEntityHandle CEntities::VirtualGetFirst(TDBTEntityHandle hRealEntity)
{
	return m_Virtuals.getFirst(hRealEntity);
}
TDBTEntityHandle CEntities::VirtualGetNext(TDBTEntityHandle hVirtual)
{	
	return m_Virtuals.getNext(hVirtual);
}


TDBTEntityHandle CEntities::compFirstContact()
{
	uint32_t sig = cEntitySignature;

	SYNC_BEGINREAD(m_Sync);
	TEntityKey key = {0};
	iterator i = LowerBound(key);
	TDBTEntityHandle res = 0;

	while (i && (res == 0))
	{
		uint32_t f = 0;
		if (m_BlockManager.ReadPart(i->Entity, &f, offsetof(TEntity, Flags), sizeof(f), sig))
		{
			if ((f & DBT_NFM_SpecialEntity) == 0)
				res = i->Entity;
		}
		if (res == 0)
			++i;
	}
	SYNC_ENDREAD(m_Sync);

	return res;
}
TDBTEntityHandle CEntities::compNextContact(TDBTEntityHandle hEntity)
{
	uint32_t sig = cEntitySignature;

	SYNC_BEGINREAD(m_Sync);

	TEntityKey key;
	key.Entity = hEntity;
	TDBTEntityHandle res = 0;

	if (m_BlockManager.ReadPart(hEntity, &key.Level, offsetof(TEntity, Level), sizeof(key.Level), sig) &&
			m_BlockManager.ReadPart(hEntity, &key.Parent, offsetof(TEntity, ParentEntity), sizeof(key.Parent), sig))
	{
		key.Entity++;
		iterator i = LowerBound(key);
		
		while (i && (res == 0))
		{
			uint32_t f = 0;
			if (m_BlockManager.ReadPart(i->Entity, &f, offsetof(TEntity, Flags), sizeof(f), sig))
			{
				if ((f & DBT_NFM_SpecialEntity) == 0)
					res = i->Entity;
			}
			if (res == 0)
				++i;
		}
	}

	SYNC_ENDREAD(m_Sync);

	return res;
}
