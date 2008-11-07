#include "Events.h"

inline bool TEventKey::operator <  (const TEventKey & Other) const
{
	if (TimeStamp != Other.TimeStamp) return TimeStamp < Other.TimeStamp;
	if (Index != Other.Index) return Index < Other.Index;
	if (Event != Other.Event) return Event < Other.Event;
	return false;
}

inline bool TEventKey::operator == (const TEventKey & Other) const
{
	return (TimeStamp == Other.TimeStamp) && (Index == Other.Index) && (Event == Other.Event);
}

inline bool TEventKey::operator >  (const TEventKey & Other) const
{	
	if (TimeStamp != Other.TimeStamp) return TimeStamp > Other.TimeStamp;
	if (Index != Other.Index) return Index > Other.Index;
	if (Event != Other.Event) return Event > Other.Event;
	return false;
}


inline bool TEventLinkKey::operator <  (const TEventLinkKey & Other) const
{
	if (Event != Other.Event) return Event < Other.Event;
	if (Entity != Other.Entity) return Entity < Other.Entity;
	return false;
}

inline bool TEventLinkKey::operator == (const TEventLinkKey & Other) const
{
	return (Event == Other.Event) && (Entity == Other.Entity);
}	

inline bool TEventLinkKey::operator >  (const TEventLinkKey & Other) const
{
	if (Event != Other.Event) return Event > Other.Event;
	if (Entity != Other.Entity) return Entity > Other.Entity;
	return false;
}



CEventsTree::CEventsTree(CBlockManager & BlockManager, TNodeRef RootNode, TDBTEntityHandle Entity)
:	CFileBTree(BlockManager, RootNode, cEventNodeSignature)
{
	m_Entity = Entity;
}
CEventsTree::~CEventsTree()
{

}

TDBTEntityHandle CEventsTree::getEntity()
{
	return m_Entity;
}
void CEventsTree::setEntity(TDBTEntityHandle NewEntity)
{
	m_Entity = NewEntity;
}

CVirtualEventsTree::CVirtualEventsTree(TDBTEntityHandle Entity)
:	CBTree(0)
{
	m_Entity = Entity;
}
CVirtualEventsTree::~CVirtualEventsTree()
{

}

TDBTEntityHandle CVirtualEventsTree::getEntity()
{
	return m_Entity;
}
void CVirtualEventsTree::setEntity(TDBTEntityHandle NewEntity)
{
	m_Entity = NewEntity;
}



CEventLinks::CEventLinks(CBlockManager & BlockManager, TNodeRef RootNode)
: CFileBTree(BlockManager, RootNode, cEventLinkNodeSignature)
{

}
CEventLinks::~CEventLinks()
{

}



CEventsTypeManager::CEventsTypeManager(CEntities & Entities, CSettings & Settings)
:	m_Entities(Entities),
	m_Settings(Settings),
	m_Map()
{
	m_Settings._EnsureModuleExists("$EventTypes");	
}
CEventsTypeManager::~CEventsTypeManager()
{
	TTypeMap::iterator it = m_Map.begin();
	
	while (it != m_Map.end())
	{
		delete [] it->second->ModuleName;
		delete it->second;
		++it;
	}
}

uint32_t CEventsTypeManager::MakeGlobalID(char* Module, uint32_t EventType)
{
	unsigned int l = strlen(Module);
	void * buf = malloc(l + sizeof(uint32_t));
	memcpy(buf, Module, l);
	memcpy(((char*)buf) + l, &EventType, sizeof(uint32_t));

	uint32_t h = Hash(buf, l + sizeof(uint32_t));

	char * m;
	uint32_t t;
	while (GetType(h, m, t) && ((t != EventType) || (strcmp(m, Module) != 0)))
	{
		++h;
	}

	return h;
}
bool CEventsTypeManager::GetType(uint32_t GlobalID, char * & Module, uint32_t & EventType)
{
	TTypeMap::iterator it = m_Map.find(GlobalID);	

	if (it == m_Map.end())
	{
		char n[256];

		TDBTSettingDescriptor d = {0};
		d.cbSize = sizeof(d);
		d.Entity = m_Entities.getRootEntity();
		d.pszSettingName = n;

		TDBTSetting sid = {0};
		TDBTSetting sname = {0};

		sid.cbSize = sizeof(sid);
		sid.Descriptor = &d;
		sid.Type = DBT_ST_INT;

		sname.cbSize = sizeof(sname);
		sname.Descriptor = &d;
		sname.Type = DBT_ST_ANSI;

		sprintf_s(n, 256, "$EventTypes/%08x/ModuleID", GlobalID);
		TDBTSettingHandle h = m_Settings.ReadSetting(sid);

		if ((h != DBT_INVALIDPARAM) && (h != 0))
		{
			sprintf_s(n, 256, "$EventTypes/%08x/ModuleName", GlobalID);
			d.Flags = 0;
			h = m_Settings.ReadSetting(sname);

			if ((h != DBT_INVALIDPARAM) && (h != 0))
			{
				PEventType t = new TEventType;

				t->EventType = sid.Value.Int;
					
				t->ModuleName = new char[sname.Value.Length];
				strcpy_s(t->ModuleName, sname.Value.Length, sname.Value.pAnsi);
				
				m_Map.insert(std::make_pair(GlobalID, t));
				
				mir_free(sname.Value.pAnsi);

				Module = t->ModuleName;
				EventType  = t->EventType;

				return true;
			}
		}
	} else {
		Module = it->second->ModuleName;
		EventType  = it->second->EventType;

		return true;
	}

	return false;
}

uint32_t CEventsTypeManager::EnsureIDExists(char* Module, uint32_t EventType)
{
	uint32_t res = MakeGlobalID(Module, EventType);
	char *   m;
	uint32_t t;
	if (!GetType(res, m, t))
	{
		char n[256];

		TDBTSettingDescriptor d = {0};
		d.cbSize = sizeof(d);
		d.pszSettingName = n;
		d.Entity = m_Entities.getRootEntity();

		TDBTSetting s = {0};
		s.cbSize = sizeof(s);
		s.Descriptor = &d;

		sprintf_s(n, 256, "$EventTypes/%08x/ModuleID", res);
		s.Type = DBT_ST_INT;
		s.Value.Int = EventType;
		m_Settings.WriteSetting(s);

		sprintf_s(n, 256, "$EventTypes/%08x/ModuleName", res);
		d.Flags = 0;
		s.Type = DBT_ST_ANSI;
		s.Value.Length = strlen(Module) + 1;
		s.Value.pAnsi = Module;
		m_Settings.WriteSetting(s);

		m_Settings._EnsureModuleExists(Module);
	}

	return res;
}


CEvents::CEvents(
	CBlockManager & BlockManager, 
	CEncryptionManager & EncryptionManager,
	CEventLinks::TNodeRef LinkRootNode, 
	CMultiReadExclusiveWriteSynchronizer & Synchronize,
	CEntities & Entities, 
	CSettings & Settings,
	uint32_t IndexCounter
)
:	m_Sync(Synchronize),
	m_BlockManager(BlockManager),	
	m_EncryptionManager(EncryptionManager),
	m_Entities(Entities),
	m_Types(Entities, Settings),
	m_Links(BlockManager, LinkRootNode),
	m_EventsMap(),
	m_VirtualEventsMap(),
	m_VirtualOwnerMap(),

	m_sigIndexCounterChanged()
{
	m_Counter = IndexCounter + 0x20;

	m_Entities._sigDeleteEvents().connect(this, &CEvents::onDeleteEvents);
	m_Entities._sigTransferEvents().connect(this, &CEvents::onTransferEvents);

}
CEvents::~CEvents()
{
	SYNC_BEGINWRITE(m_Sync);
	
	TEventsTreeMap::iterator it1 = m_EventsMap.begin();
	while (it1 != m_EventsMap.end())
	{
		delete it1->second;
		++it1;
	}
	m_EventsMap.clear();

	TVirtualEventsTreeMap::iterator it2 = m_VirtualEventsMap.begin();
	while (it2 != m_VirtualEventsMap.end())
	{
		delete it2->second;
		++it2;
	}
	m_VirtualEventsMap.clear();

	SYNC_ENDWRITE(m_Sync);

}

CEventLinks::TOnRootChanged & CEvents::sigLinkRootChanged() 
{
	return m_Links.sigRootChanged();
}
CEvents::TOnIndexCounterChanged & CEvents::_sigIndexCounterChanged()
{
	return m_sigIndexCounterChanged;
}


void CEvents::onRootChanged(void* EventsTree, CEventsTree::TNodeRef NewRoot)
{
	m_Entities._setEventsRoot(((CEventsTree*)EventsTree)->getEntity(), NewRoot);
}


void CEvents::onDeleteEventCallback(void * Tree, const TEventKey & Key, uint32_t Param)
{
	uint32_t sig = cEventSignature;
	uint32_t f;
	if (m_BlockManager.ReadPart(Key.Event, &f, offsetof(TEvent, Flags), sizeof(f), sig))
	{
		if (f & DBT_EF_REFERENCECOUNTING)
		{
			TEventLinkKey lkey;
			lkey.Entity = Param;
			lkey.Event = Key.Event;
			m_Links.Delete(lkey);

			uint32_t rc;
			m_BlockManager.ReadPart(Key.Event, &rc, offsetof(TEvent, ReferenceCount), sizeof(rc), sig);
			--rc;

			if (rc == 1)
			{
				lkey.Entity = 0;

				CEventLinks::iterator i = m_Links.LowerBound(lkey);
				if (!i || (i->Event != Key.Event))
					throwException("Event reference tree corrupt!");

				f = f & (~DBT_EF_REFERENCECOUNTING);
				TDBTEntityHandle c = i->Entity;

				m_BlockManager.WritePart(Key.Event, &rc, offsetof(TEvent, Entity), sizeof(c));
				m_BlockManager.WritePart(Key.Event, &f, offsetof(TEvent, Flags), sizeof(f));

				m_Links.Delete(*i);
			} else {
				m_BlockManager.WritePart(Key.Event, &rc, offsetof(TEvent, ReferenceCount), sizeof(rc));
			}

		} else {
			TVirtualOwnerMap::iterator mit = m_VirtualOwnerMap.find(Key.Event);

			if (mit != m_VirtualOwnerMap.end())
			{
				m_BlockManager.MakeBlockVirtual(Key.Event);
			} else {
				m_BlockManager.DeleteBlock(Key.Event);
			}
		}
	}
}

void CEvents::onDeleteVirtualEventCallback(void * Tree, const TEventKey & Key, uint32_t Param)
{
	TVirtualOwnerMap::iterator mit = m_VirtualOwnerMap.find(Key.Event);
	if (mit != m_VirtualOwnerMap.end())
	{
		TVirtualOwnerSet::iterator sit = mit->second->find(Param);

		if (sit != mit->second->end())
		{
			mit->second->erase(sit);
			if (mit->second->empty())
			{
				delete mit->second;
				m_VirtualOwnerMap.erase(mit);

				if (m_BlockManager.IsForcedVirtual(Key.Event))
					m_BlockManager.DeleteBlock(Key.Event);
			}
		}
	}
}
void CEvents::onDeleteEvents(CEntities * Entities, TDBTEntityHandle hEntity)
{
	CEventsTree * tree = getEventsTree(hEntity);

	if (tree == NULL)
		return;

	CVirtualEventsTree * vtree = getVirtualEventsTree(hEntity);

	m_Entities._setEventsRoot(hEntity, 0);

	if (vtree)
	{
		CVirtualEventsTree::TDeleteCallback callback;
		callback.connect(this, &CEvents::onDeleteVirtualEventCallback);

		vtree->DeleteTree(&callback, hEntity);

		TVirtualEventsTreeMap::iterator i = m_VirtualEventsMap.find(hEntity);
		delete i->second; // vtree
		m_VirtualEventsMap.erase(i);
	}

	if (tree)
	{
		CEventsTree::TDeleteCallback callback;
		callback.connect(this, &CEvents::onDeleteEventCallback);

		tree->DeleteTree(&callback, hEntity);

		TEventsTreeMap::iterator i = m_EventsMap.find(hEntity);
		delete i->second; // tree
		m_EventsMap.erase(i);
	}
}
void CEvents::onTransferEvents(CEntities * Entities, TDBTEntityHandle Source, TDBTEntityHandle Dest)
{
	CEventsTree * tree = getEventsTree(Source);

	if (tree == NULL)
		return;

	CVirtualEventsTree * vtree = getVirtualEventsTree(Source);

	if (vtree)
	{
		TEventKey key = {0};

		CVirtualEventsTree::iterator i = vtree->LowerBound(key);

		while (i)
		{
			TVirtualOwnerMap::iterator mit = m_VirtualOwnerMap.find(i->Event);
			TVirtualOwnerSet * s = NULL;
			if (mit == m_VirtualOwnerMap.end())
			{
				s = new TVirtualOwnerSet();
				m_VirtualOwnerMap.insert(std::make_pair(i->Event, s));
			} else {
				s = mit->second;
				mit->second->erase(Source);
			}
			
			s->insert(Dest);

			++i;
		}

		TVirtualEventsTreeMap::iterator tit = m_VirtualEventsMap.find(Source);
		m_VirtualEventsMap.erase(tit);
		m_VirtualEventsMap.insert(std::make_pair(Dest, vtree));
		vtree->setEntity(Dest);

		TVirtualEventsCountMap::iterator cit = m_VirtualCountMap.find(Source);
		m_VirtualCountMap.insert(std::make_pair(Dest, cit->second));
		m_VirtualCountMap.erase(cit);
	}

	if (tree)
	{
		TEventKey key = {0};

		CEventsTree::iterator i = tree->LowerBound(key);
		while (i)
		{
			uint32_t f;
			uint32_t sig = cEventSignature;

			if (m_BlockManager.ReadPart(i->Event, &f, offsetof(TEvent, Flags), sizeof(f), sig))
			{
				if (f & DBT_EF_REFERENCECOUNTING)
				{
					TEventLinkKey lkey;

					lkey.Entity = Source;
					lkey.Event = i->Event;
					
					m_Links.Delete(lkey);

					lkey.Entity = Dest;
					m_Links.Insert(lkey);

				} else {
					m_BlockManager.WritePart(i->Event, &Dest, offsetof(TEvent, Entity), sizeof(Dest));
				}
			}

			++i;
		}

		m_Entities._setEventsRoot(Source, 0);
		m_Entities._setEventsRoot(Dest, tree->getRoot());

		TEventsTreeMap::iterator tit = m_EventsMap.find(Source);
		m_EventsMap.erase(tit);
		m_EventsMap.insert(std::make_pair(Dest, tree));
		tree->setEntity(Dest);
	}
}

CEventsTree * CEvents::getEventsTree(TDBTEntityHandle hEntity)
{
	TEventsTreeMap::iterator i = m_EventsMap.find(hEntity);
	if (i != m_EventsMap.end())
		return i->second;

	uint32_t root = m_Entities._getEventsRoot(hEntity);
	if (root == DBT_INVALIDPARAM)
		return NULL;

	CEventsTree * tree = new CEventsTree(m_BlockManager, root, hEntity);
	tree->sigRootChanged().connect(this, &CEvents::onRootChanged);
	m_EventsMap.insert(std::make_pair(hEntity, tree));

	return tree;	
}
CVirtualEventsTree * CEvents::getVirtualEventsTree(TDBTEntityHandle hEntity)
{
	TVirtualEventsTreeMap::iterator i = m_VirtualEventsMap.find(hEntity);
	if (i != m_VirtualEventsMap.end())
		return i->second;

	CVirtualEventsTree * tree = new CVirtualEventsTree(hEntity);
	tree->sigRootChanged().connect(this, &CEvents::onRootChanged);
	m_VirtualEventsMap.insert(std::make_pair(hEntity, tree));

	m_VirtualCountMap.insert(std::make_pair(hEntity, 0));

	return tree;		
}

inline uint32_t CEvents::adjustVirtualEventCount(TDBTEntityHandle hEntity, int32_t Adjust)
{
	TVirtualEventsCountMap::iterator i = m_VirtualCountMap.find(hEntity);
	
	if (i == m_VirtualCountMap.end())
	{
		m_VirtualCountMap.insert(std::make_pair(hEntity, 0));
		i = m_VirtualCountMap.find(hEntity);
	}

	if (((Adjust < 0) && ((uint32_t)(-Adjust) <= i->second)) || 
		  ((Adjust > 0) && ((0xffffffff - i->second) > (uint32_t)Adjust)))
	{
		i->second += Adjust;
	}

	return i->second;
}

unsigned int CEvents::GetBlobSize(TDBTEventHandle hEvent)
{
	uint32_t sig = cEventSignature;
	uint32_t s;

	SYNC_BEGINREAD(m_Sync);
	if (!m_BlockManager.ReadPart(hEvent, &s, offsetof(TEvent, DataLength), sizeof(s), sig))
		s = DBT_INVALIDPARAM;

	SYNC_ENDREAD(m_Sync);

	return s;
}
unsigned int CEvents::Get(TDBTEventHandle hEvent, TDBTEvent & Event)
{
	uint32_t sig = cEventSignature;
	uint32_t size = 0;
	void * buf = NULL;

	SYNC_BEGINREAD(m_Sync);

	if (!m_BlockManager.ReadBlock(hEvent, buf, size, sig))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	TEvent * ev = (TEvent*)buf;
	uint8_t * blob = (uint8_t *)(ev + 1);
	
	if (!m_Types.GetType(ev->Type, Event.ModuleName, Event.EventType))
	{
		Event.EventType = ev->Type;
		Event.ModuleName = "???";
	}

	// encryption
	if (m_EncryptionManager.IsEncrypted(hEvent, ET_DATA))
	{
		uint32_t size = ev->DataLength;
		m_EncryptionManager.AlignSize(hEvent, ET_DATA, size);
		m_EncryptionManager.Decrypt(blob, size, ET_DATA, hEvent, 0);
	}

	SYNC_ENDREAD(m_Sync);  // we leave here. we cannot leave earlier due to encryption change thread

	Event.Flags = ev->Flags;
	if (m_BlockManager.IsForcedVirtual(hEvent))
		Event.Flags |= DBT_EF_VIRTUAL;

	Event.Timestamp = ev->TimeStamp;

	if (Event.cbBlob < ev->DataLength)
		Event.pBlob = (uint8_t*) mir_realloc(Event.pBlob, ev->DataLength);
	
	memcpy(Event.pBlob, ev + 1, ev->DataLength);	
	Event.cbBlob = ev->DataLength;

	free(buf);
	return 0;
}

unsigned int CEvents::GetCount(TDBTEntityHandle hEntity)
{
	SYNC_BEGINREAD(m_Sync);

	uint32_t res = m_Entities._getEventCount(hEntity);
	if (res == DBT_INVALIDPARAM)
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	TVirtualEventsCountMap::iterator i = m_VirtualCountMap.find(hEntity);
	if (i != m_VirtualCountMap.end())
		res += i->second;

	SYNC_ENDREAD(m_Sync);
	return res;
}

unsigned int CEvents::Delete(TDBTEntityHandle hEntity, TDBTEventHandle hEvent)
{
	uint32_t sig = cEventSignature;
	uint32_t flags;
	TEventKey key;

	SYNC_BEGINWRITE(m_Sync);
	if (!m_BlockManager.ReadPart(hEvent, &flags, offsetof(TEvent, Flags), sizeof(flags), sig) ||
		  !m_BlockManager.ReadPart(hEvent, &key.TimeStamp, offsetof(TEvent, TimeStamp), sizeof(key.TimeStamp), sig) ||
		  !m_BlockManager.ReadPart(hEvent, &key.Index, offsetof(TEvent, Index), sizeof(key.Index), sig))
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	TVirtualOwnerMap::iterator mit = m_VirtualOwnerMap.find(hEvent);
	TVirtualOwnerSet::iterator sit;
	if (mit != m_VirtualOwnerMap.end())
		sit = mit->second->find(hEntity);

	if ((mit != m_VirtualOwnerMap.end()) && (sit != mit->second->end())) 
	{ // virtual event
		CVirtualEventsTree* tree = getVirtualEventsTree(hEntity);

		if (tree == NULL)
		{
			SYNC_ENDWRITE(m_Sync);
			return DBT_INVALIDPARAM;
		}		
		tree->Delete(key);
		adjustVirtualEventCount(hEntity, -1);

		mit->second->erase(sit);
		if (mit->second->empty())
		{
			delete mit->second;
			m_VirtualOwnerMap.erase(mit);

			if (m_BlockManager.IsForcedVirtual(hEvent))
				m_BlockManager.DeleteBlock(hEvent);
		}

	} else { // real event
		CEventsTree* tree = getEventsTree(hEntity);

		if (tree == NULL)
		{
			SYNC_ENDWRITE(m_Sync);
			return DBT_INVALIDPARAM;
		}
		tree->Delete(key);
		m_Entities._adjustEventCount(hEntity, -1);

		if (flags & DBT_EF_REFERENCECOUNTING)
		{
			uint32_t ref;
			m_BlockManager.ReadPart(hEvent, &ref, offsetof(TEvent, ReferenceCount), sizeof(ref), sig);
			--ref;

			TEventLinkKey lkey = {0};
			lkey.Event = hEvent;
			lkey.Entity = hEntity;
			m_Links.Delete(lkey);

			if (ref == 1)
			{
				flags = flags & ~DBT_EF_REFERENCECOUNTING;
				m_BlockManager.WritePart(hEvent, &flags, offsetof(TEvent, Flags), sizeof(flags));

				lkey.Entity = 0;
				CEventLinks::iterator it = m_Links.LowerBound(lkey);
				lkey = *it;
				m_Links.Delete(*it);
				m_BlockManager.WritePart(hEvent, &lkey.Entity, offsetof(TEvent, Entity), sizeof(lkey.Entity));
			} else {
				m_BlockManager.WritePart(hEvent, &ref, offsetof(TEvent, ReferenceCount), sizeof(ref));
			}
		} else {
			if (mit != m_VirtualOwnerMap.end()) 
			{ // we have virtuals left
				m_BlockManager.MakeBlockVirtual(hEvent);
			} else {
				m_BlockManager.DeleteBlock(hEvent);
			}
		}	
	}

	SYNC_ENDWRITE(m_Sync);
	return 0;
}
TDBTEventHandle CEvents::Add(TDBTEntityHandle hEntity, TDBTEvent & Event)
{
	uint32_t sig = cEventSignature;
	CVirtualEventsTree* vtree;
	CEventsTree* tree;
	TDBTEventHandle res = 0;

	SYNC_BEGINWRITE(m_Sync);

	uint32_t cflags = m_Entities.getFlags(hEntity);

	if ((cflags == DBT_INVALIDPARAM) || (cflags & DBT_NF_IsGroup))
	{	
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	if (cflags & DBT_NF_IsVirtual)
	{
		hEntity = m_Entities.VirtualGetParent(hEntity);
	}

	uint8_t *blobdata = Event.pBlob;
	bool bloballocated = false;
	uint32_t cryptsize = m_EncryptionManager.AlignSize(0, ET_DATA, Event.cbBlob);

	if (Event.Flags & DBT_EF_VIRTUAL)
	{
		vtree = getVirtualEventsTree(hEntity);
		if (vtree != NULL)
			res = m_BlockManager.CreateBlockVirtual(sizeof(TEvent) + cryptsize, cEventSignature);
		
	} else {
		tree = getEventsTree(hEntity);
		if (tree != NULL)
			res = m_BlockManager.CreateBlock(sizeof(TEvent) + cryptsize, cEventSignature);
	}

	if (res == 0)
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	// encryption
	if (m_EncryptionManager.IsEncrypted(res, ET_DATA))
	{
		uint32_t realsize = m_EncryptionManager.AlignSize(res, ET_DATA, Event.cbBlob);
		if (realsize != cryptsize)
			m_BlockManager.ResizeBlock(res, realsize, false);

		blobdata = NULL;
		bloballocated = true;
		blobdata = (uint8_t*) malloc(realsize);
		memset(blobdata, 0, realsize);
		memcpy(blobdata, Event.pBlob, Event.cbBlob);
		
		m_EncryptionManager.Encrypt(blobdata, realsize, ET_DATA, res, 0);
		cryptsize = realsize;
	} else {
		if (Event.cbBlob != cryptsize)
			m_BlockManager.ResizeBlock(res, Event.cbBlob, false);

		cryptsize = Event.cbBlob;
	}

	TEvent ev = {0};
	TEventKey key = {0};

	ev.TimeStamp = Event.Timestamp;
	ev.Index = m_Counter;
	m_Counter++;
	if ((m_Counter & 0x1f) == 0)
		m_sigIndexCounterChanged.emit(this, m_Counter);

	key.TimeStamp = ev.TimeStamp;
	key.Index = ev.Index;
	key.Event = res;


	ev.Flags = Event.Flags & ~(DBT_EF_VIRTUAL | DBT_EF_REFERENCECOUNTING); 
	ev.Type = m_Types.EnsureIDExists(Event.ModuleName, Event.EventType);
	ev.DataLength = Event.cbBlob;
	ev.Entity = hEntity;

	m_BlockManager.WritePart(res, &ev, 0, sizeof(ev));
	m_BlockManager.WritePart(res, blobdata, sizeof(ev), cryptsize);

	if (bloballocated)
		free(blobdata);

	if (Event.Flags & DBT_EF_VIRTUAL)
	{
		vtree->Insert(key);

		TVirtualOwnerMap::iterator mit = m_VirtualOwnerMap.find(hEntity);
		if (mit == m_VirtualOwnerMap.end())
		{
			TVirtualOwnerSet * tmp = new TVirtualOwnerSet();
			tmp->insert(hEntity);
			m_VirtualOwnerMap.insert(std::make_pair(res, tmp));
			
		} else {
			mit->second->insert(hEntity);
		}
		adjustVirtualEventCount(hEntity, +1);
	} else {
		tree->Insert(key);
		m_Entities._adjustEventCount(hEntity, +1);
	}

	SYNC_ENDWRITE(m_Sync);

	return res;
}
unsigned int CEvents::MarkRead(TDBTEntityHandle hEntity, TDBTEventHandle hEvent)
{
	uint32_t sig = cEventSignature;
	TEventKey key = {0};
	
	SYNC_BEGINWRITE(m_Sync);

	if (!m_BlockManager.ReadPart(hEvent, &key.TimeStamp, offsetof(TEvent, TimeStamp), sizeof(key.TimeStamp), sig) ||
		  !m_BlockManager.ReadPart(hEvent, &key.Index, offsetof(TEvent, Index), sizeof(key.Index), sig))
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	CEventsTree* tree = getEventsTree(hEntity);

	if (tree == NULL)
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	CVirtualEventsTree* vtree = getVirtualEventsTree(hEntity);

	CEventsTree::iterator it(tree->UpperBound(key));
	CVirtualEventsTree::iterator vit(vtree->UpperBound(key));

	TEventsHeap heap(it, TEventsHeap::ITBackward, false);
	heap.Insert(vit);

	bool b;
	uint32_t res = 0;
	do {
		b = false;
		if (heap.Top())
		{
			TDBTEventHandle ev = heap.Top()->Event;
			uint32_t flags;

			heap.Pop();

			if (m_BlockManager.ReadPart(ev, &flags, offsetof(TEvent, Flags), sizeof(flags), sig))
			{
				if ((flags & DBT_EF_READ) == 0)
				{
					b = true;
					flags = flags | DBT_EF_READ;
					if (res == 0)
						res = flags;

					m_BlockManager.WritePart(ev, &flags, offsetof(TEvent, Flags), sizeof(flags));
				}
			}
		}
	} while (b);

	SYNC_ENDWRITE(m_Sync);

	return res;
}
unsigned int CEvents::WriteToDisk(TDBTEntityHandle hEntity, TDBTEventHandle hEvent)
{
	uint32_t sig = cEventSignature;
	TEventKey key;
	uint32_t flags;

	key.Event = hEvent;

	SYNC_BEGINWRITE(m_Sync);

	CEventsTree * tree = getEventsTree(hEntity);

	if ((tree == NULL) ||
		  !m_BlockManager.ReadPart(hEvent, &flags, offsetof(TEvent, Flags), sizeof(flags), sig) ||
		  !m_BlockManager.ReadPart(hEvent, &key.TimeStamp, offsetof(TEvent, TimeStamp), sizeof(key.TimeStamp), sig) ||
		  !m_BlockManager.ReadPart(hEvent, &key.Index, offsetof(TEvent, Index), sizeof(key.Index), sig))
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	CVirtualEventsTree * vtree = getVirtualEventsTree(hEntity);

	TVirtualOwnerMap::iterator mit = m_VirtualOwnerMap.find(hEvent);
	if (mit == m_VirtualOwnerMap.end())
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	TVirtualOwnerSet::iterator sit = mit->second->find(hEntity);
	if (sit == mit->second->end())
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	mit->second->erase(sit);
	if (mit->second->empty())
	{
		delete mit->second;
		m_VirtualOwnerMap.erase(mit);
	}

	vtree->Delete(key);
	tree->Insert(key);

	m_Entities._adjustEventCount(hEntity, +1);
	adjustVirtualEventCount(hEntity, -1);

	if (m_BlockManager.IsForcedVirtual(hEvent))
	{
		m_BlockManager.WritePart(hEvent, &hEntity, offsetof(TEvent, Entity), sizeof(hEntity));
		m_BlockManager.WriteBlockToDisk(hEvent);
	} else {
		TEventLinkKey lkey;
		lkey.Event = hEvent;

		if ((flags & DBT_EF_REFERENCECOUNTING) == 0)
		{
			TDBTEntityHandle tmp;
			m_BlockManager.ReadPart(hEvent, &tmp, offsetof(TEvent, Entity), sizeof(tmp), sig);
			
			lkey.Entity = tmp;
			m_Links.Insert(lkey);

			uint32_t ref = 2;
			flags = flags | DBT_EF_REFERENCECOUNTING;

			m_BlockManager.WritePart(hEvent, &flags, offsetof(TEvent, Flags), sizeof(flags));
			m_BlockManager.WritePart(hEvent, &ref, offsetof(TEvent, ReferenceCount), sizeof(ref));
			
		} else {
			uint32_t ref;
			m_BlockManager.ReadPart(hEvent, &ref, offsetof(TEvent, ReferenceCount), sizeof(ref), sig);
			++ref;
			m_BlockManager.WritePart(hEvent, &ref, offsetof(TEvent, ReferenceCount), sizeof(ref));
		}

		lkey.Entity = hEntity;
		m_Links.Insert(lkey);
	}

	SYNC_ENDWRITE(m_Sync);

	return flags;
}

TDBTEntityHandle CEvents::getEntity(TDBTEventHandle hEvent)
{
	uint32_t sig = cEventSignature;
	uint32_t flags;
	TDBTEntityHandle res = 0;

	SYNC_BEGINREAD(m_Sync);

	if (!m_BlockManager.ReadPart(hEvent, &flags, offsetof(TEvent, Flags), sizeof(flags), sig))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	if (m_BlockManager.IsForcedVirtual(hEvent))
	{		
		TVirtualOwnerMap::iterator mit = m_VirtualOwnerMap.find(hEvent);
		if (mit != m_VirtualOwnerMap.end())
			res = *mit->second->begin();	
	} else {
		if (flags & DBT_EF_REFERENCECOUNTING)
		{
			TEventLinkKey lkey;
			lkey.Event = hEvent;
			lkey.Entity = 0;
			CEventLinks::iterator it = m_Links.LowerBound(lkey);
			if ((it) && (it->Event == hEvent))
				res = it->Entity;
		} else {
			m_BlockManager.ReadPart(hEvent, &res, offsetof(TEvent, Entity), sizeof(res), sig);
		}
	}

	SYNC_ENDREAD(m_Sync);
	return res;
}

unsigned int CEvents::HardLink(TDBTEventHardLink & HardLink)
{
	uint32_t sig = cEventSignature;
	uint32_t flags;
	TEventKey key;

	key.Event = HardLink.hEvent;

	SYNC_BEGINWRITE(m_Sync);
	

	if (!m_BlockManager.ReadPart(HardLink.hEvent, &flags, offsetof(TEvent, Flags), sizeof(flags), sig) ||
		  !m_BlockManager.ReadPart(HardLink.hEvent, &key.TimeStamp, offsetof(TEvent, TimeStamp), sizeof(key.TimeStamp), sig) ||
			!m_BlockManager.ReadPart(HardLink.hEvent, &key.Index, offsetof(TEvent, Index), sizeof(key.Index), sig))
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	uint32_t cflags = m_Entities.getFlags(HardLink.hEntity);
	if ((cflags == DBT_INVALIDPARAM) || (cflags & DBT_NF_IsGroup))
	{	
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	if (cflags & DBT_NF_IsVirtual)
	{
		HardLink.hEntity = m_Entities.VirtualGetParent(HardLink.hEntity);
	}

	if (HardLink.Flags & DBT_EF_VIRTUAL)
	{
		CVirtualEventsTree * vtree = getVirtualEventsTree(HardLink.hEntity);
		if (vtree == NULL)
		{
			SYNC_ENDWRITE(m_Sync);
			return DBT_INVALIDPARAM;
		}

		vtree->Insert(key);
		adjustVirtualEventCount(HardLink.hEntity, +1);

		TVirtualOwnerMap::iterator mit = m_VirtualOwnerMap.find(HardLink.hEntity);
		if (mit == m_VirtualOwnerMap.end())
		{
			TVirtualOwnerSet * tmp = new TVirtualOwnerSet();
			tmp->insert(HardLink.hEntity);
			m_VirtualOwnerMap.insert(std::make_pair(HardLink.hEvent, tmp));
			
		} else {
			mit->second->insert(HardLink.hEntity);
		}

	} else {
		CEventsTree * tree = getEventsTree(HardLink.hEntity);

		if (tree == NULL)
		{
			SYNC_ENDWRITE(m_Sync);
			return DBT_INVALIDPARAM;
		}

		tree->Insert(key);
		m_Entities._adjustEventCount(HardLink.hEntity, +1);

		if (m_BlockManager.IsForcedVirtual(HardLink.hEvent))
		{
			m_BlockManager.WritePart(HardLink.hEvent, &HardLink.hEntity, offsetof(TEvent, Entity), sizeof(HardLink.hEntity));
			if (flags & DBT_EF_REFERENCECOUNTING)
			{
				flags = flags & ~DBT_EF_REFERENCECOUNTING;
				m_BlockManager.WritePart(HardLink.hEvent, &flags, offsetof(TEvent, Flags), sizeof(flags));
			}
			m_BlockManager.WriteBlockToDisk(HardLink.hEvent);
		} else {
			TEventLinkKey lkey;
			uint32_t ref;
			lkey.Event = HardLink.hEvent;

			if ((flags & DBT_EF_REFERENCECOUNTING) == 0)
			{
				flags = flags | DBT_EF_REFERENCECOUNTING;
				m_BlockManager.WritePart(HardLink.hEvent, &flags, offsetof(TEvent, Flags), sizeof(flags));

				m_BlockManager.ReadPart(HardLink.hEvent, &lkey.Entity, offsetof(TEvent, Entity), sizeof(lkey.Entity), sig);
				m_Links.Insert(lkey);

				ref = 1;
			} else {
				m_BlockManager.ReadPart(HardLink.hEvent, &ref, offsetof(TEvent, ReferenceCount), sizeof(ref), sig); 
			}

			lkey.Entity = HardLink.hEntity;
			m_Links.Insert(lkey);

			m_BlockManager.WritePart(HardLink.hEvent, &ref, offsetof(TEvent, ReferenceCount), sizeof(ref));
		}
	}

	SYNC_ENDWRITE(m_Sync);

	return 0;
}

TDBTEventIterationHandle CEvents::IterationInit(TDBTEventIterFilter & Filter)
{
	SYNC_BEGINREAD(m_Sync);

	CEventsTree * tree = getEventsTree(Filter.hEntity);

	if (tree == NULL)
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	CVirtualEventsTree * vtree = getVirtualEventsTree(Filter.hEntity);

	std::queue<TEventBase * > q;
	q.push(tree);
	q.push(vtree);

	TDBTEntityIterFilter f = {0};
	f.cbSize = sizeof(f);
	f.Options = Filter.Options;
	
	TDBTEntityIterationHandle citer = m_Entities.IterationInit(f, Filter.hEntity);
	if (citer != DBT_INVALIDPARAM)
	{
		m_Entities.IterationNext(citer);
		TDBTEntityHandle c = m_Entities.IterationNext(citer);
		while (c != 0)
		{
			tree = getEventsTree(c);
			if (tree)
			{
				q.push(tree);
				
				vtree = getVirtualEventsTree(c);
				if (vtree)
					q.push(vtree);
			}

			c = m_Entities.IterationNext(citer);
		}

		m_Entities.IterationClose(citer);
	}

	for (unsigned int j = 0; j < Filter.ExtraCount; ++j)
	{
		tree = getEventsTree(Filter.ExtraEntities[j]);
		if (tree)
		{
			q.push(tree);
			
			vtree = getVirtualEventsTree(Filter.ExtraEntities[j]);
			if (vtree)
				q.push(vtree);
		}
	}	

	PEventIteration iter = new TEventIteration;
	iter->Filter = Filter;
	iter->LastEvent = 0;
	iter->Heap = NULL;

	TEventKey key;
	key.Index = 0;
	key.TimeStamp = Filter.tSince;

	while (!q.empty())
	{
		TEventBase * b = q.front();
		q.pop();

		TEventBase::iterator it = b->LowerBound(key);
		if (it)
		{
			TEventBase::iterator * it2 = new TEventBase::iterator(it);
			it2->setManaged();
			if (iter->Heap)
			{
				iter->Heap->Insert(*it2);
			} else {				
				iter->Heap = new TEventsHeap(*it2, TEventsHeap::ITForward, true);
			}
		}		
	}

	if (iter->Heap == NULL)
	{
		delete iter;
		iter = (PEventIteration)DBT_INVALIDPARAM;
	}

	SYNC_ENDREAD(m_Sync);

	return (TDBTEventIterationHandle)iter;
}
TDBTEventHandle CEvents::IterationNext(TDBTEventIterationHandle Iteration)
{
	SYNC_BEGINREAD(m_Sync);

	PEventIteration iter = (PEventIteration) Iteration;
	uint32_t sig = cEventSignature;

	TDBTEventHandle res = 0;
	TEventBase::iterator it = iter->Heap->Top();
	
	while ((it) && (it.wasDeleted() || ((it->TimeStamp <= iter->Filter.tTill) && (it->Event == iter->LastEvent))))
	{
		iter->Heap->Pop();
		it = iter->Heap->Top();
	}

	if ((it) && !it.wasDeleted() && (it->TimeStamp <= iter->Filter.tTill))
	{
		res = it->Event;
		iter->Heap->Pop();
	}

	if (res)
	{
		iter->LastEvent = res;
		if (iter->Filter.Event)
		{
			iter->Filter.Event->EventType = 0;
			Get(res, *iter->Filter.Event);
		}
	}

	SYNC_ENDREAD(m_Sync);

	return res;
}
unsigned int CEvents::IterationClose(TDBTEventIterationHandle Iteration)
{	
	PEventIteration iter = (PEventIteration) Iteration;
	
	SYNC_BEGINREAD(m_Sync);
	delete iter->Heap;
	SYNC_ENDREAD(m_Sync);
	
	delete iter;
	return 0;
}


TDBTEventHandle CEvents::compFirstEvent(TDBTEntityHandle hEntity)
{
	SYNC_BEGINREAD(m_Sync);

	TDBTEventHandle res = 0;

	CEventsTree * tree = getEventsTree(hEntity);
	if (tree == NULL)
	{
		SYNC_ENDREAD(m_Sync);
		return 0;
	}
	CVirtualEventsTree * vtree = getVirtualEventsTree(hEntity);

	TEventKey key = {0};
	CEventsTree::iterator i = tree->LowerBound(key);
	CVirtualEventsTree::iterator vi = vtree->LowerBound(key);

	if (i && vi)
	{
		if (*i < *vi)
		{
			res = i->Event;
		} else {
			res = vi->Event;
		}
	} else if (i)
	{
		res = i->Event;
	} else if (vi)
	{
		res = vi->Event;
	}

	SYNC_ENDREAD(m_Sync);

	return res;
}
TDBTEventHandle CEvents::compFirstUnreadEvent(TDBTEntityHandle hEntity)
{
	uint32_t sig = cEventSignature;

	SYNC_BEGINREAD(m_Sync);

	TDBTEventHandle res = 0;

	CEventsTree * tree = getEventsTree(hEntity);
	if (tree == NULL)
	{
		SYNC_ENDREAD(m_Sync);
		return 0;
	}
	CVirtualEventsTree * vtree = getVirtualEventsTree(hEntity);

	TEventKey key;
	key.TimeStamp = 0xffffffff;
	key.Index = 0xffffffff;
	
	CEventsTree::iterator i = tree->UpperBound(key);
	CVirtualEventsTree::iterator vi = vtree->UpperBound(key);

	TEventsHeap h(i, TEventsHeap::ITBackward, false);

	h.Insert(vi);

	TDBTEventHandle l = 0;
	while (h.Top() && (res == 0))
	{
		uint32_t f;
		if (m_BlockManager.ReadPart(h.Top()->Event, &f, offsetof(TEvent, Flags), sizeof(f), sig))
		{
			if ((f & DBT_EF_READ) == 0)
				res = l;
			else
				l = h.Top()->Event;
		}

		if (res == 0)
			h.Pop();

	}

	SYNC_ENDREAD(m_Sync);

	return res;
}
TDBTEventHandle CEvents::compLastEvent(TDBTEntityHandle hEntity)
{
	SYNC_BEGINREAD(m_Sync);

	TDBTEventHandle res = 0;

	CEventsTree * tree = getEventsTree(hEntity);
	if (tree == NULL)
	{
		SYNC_ENDREAD(m_Sync);
		return 0;
	}
	CVirtualEventsTree * vtree = getVirtualEventsTree(hEntity);

	TEventKey key;
	key.TimeStamp = 0xffffffff;
	key.Index = 0xffffffff;

	CEventsTree::iterator i = tree->UpperBound(key);
	CVirtualEventsTree::iterator vi = vtree->UpperBound(key);

	if (i && vi)
	{
		if (*i > *vi)
		{
			res = i->Event;
		} else {
			res = vi->Event;
		}
	} else if (i)
	{
		res = i->Event;
	} else if (vi)
	{
		res = vi->Event;
	}

	SYNC_ENDREAD(m_Sync);

	return res;
}
TDBTEventHandle CEvents::compNextEvent(TDBTEventHandle hEvent)
{
	uint32_t sig = cEventSignature;

	SYNC_BEGINREAD(m_Sync);

	TDBTEntityHandle c;
	TEventKey key;

	if (!m_BlockManager.ReadPart(hEvent, &c, offsetof(TEvent, Entity), sizeof(c), sig) ||
		  !m_BlockManager.ReadPart(hEvent, &key.TimeStamp, offsetof(TEvent, TimeStamp), sizeof(key.TimeStamp), sig) ||
			!m_BlockManager.ReadPart(hEvent, &key.Index, offsetof(TEvent, Index), sizeof(key.Index), sig))
	{
		SYNC_ENDREAD(m_Sync);
		return 0;
	}

	CEventsTree * tree = getEventsTree(c);

	if (tree == NULL)
	{
		SYNC_ENDREAD(m_Sync);
		return 0;
	}

	CVirtualEventsTree * vtree = getVirtualEventsTree(c);

	if (key.Index == 0xffffffff)
	{
		if (key.TimeStamp == 0xffffffff)
		{
			SYNC_ENDREAD(m_Sync);
			return 0;
		}
		++key.TimeStamp;
		key.Index = 0;
	} else {
		++key.Index;
	}

	CEventsTree::iterator i = tree->LowerBound(key);
	CVirtualEventsTree::iterator vi = vtree->LowerBound(key);

	TDBTEventHandle res = 0;
	if (i && vi)
	{
		if (*i < *vi)
		{
			res = i->Event;
		} else {
			res = vi->Event;
		}
	} else if (i)
	{
		res = i->Event;
	} else if (vi)
	{
		res = vi->Event;
	}

	SYNC_ENDREAD(m_Sync);

	return res;
}
TDBTEventHandle CEvents::compPrevEvent(TDBTEventHandle hEvent)
{
	uint32_t sig = cEventSignature;

	SYNC_BEGINREAD(m_Sync);

	TDBTEntityHandle c;
	TEventKey key;

	if (!m_BlockManager.ReadPart(hEvent, &c, offsetof(TEvent, Entity), sizeof(c), sig) ||
		  !m_BlockManager.ReadPart(hEvent, &key.TimeStamp, offsetof(TEvent, TimeStamp), sizeof(key.TimeStamp), sig) ||
			!m_BlockManager.ReadPart(hEvent, &key.Index, offsetof(TEvent, Index), sizeof(key.Index), sig))
	{
		SYNC_ENDREAD(m_Sync);
		return 0;
	}

	CEventsTree * tree = getEventsTree(c);

	if (tree == NULL)
	{
		SYNC_ENDREAD(m_Sync);
		return 0;
	}

	CVirtualEventsTree * vtree = getVirtualEventsTree(c);

	if (key.Index == 0)
	{
		if (key.TimeStamp == 0)
		{
			SYNC_ENDREAD(m_Sync);
			return 0;
		}
		--key.TimeStamp;
		key.Index = 0xffffffff;
	} else {
		--key.Index;
	}

	CEventsTree::iterator i = tree->UpperBound(key);
	CVirtualEventsTree::iterator vi = vtree->UpperBound(key);

	TDBTEventHandle res = 0;
	if (i && vi)
	{
		if (*i > *vi)
		{
			res = i->Event;
		} else {
			res = vi->Event;
		}
	} else if (i)
	{
		res = i->Event;
	} else if (vi)
	{
		res = vi->Event;
	}

	SYNC_ENDREAD(m_Sync);

	return res;
}	
