#include "Events.h"

inline bool TEventKey::operator <  (const TEventKey & Other) const
{
	if (TimeStamp != Other.TimeStamp) return TimeStamp < Other.TimeStamp;
	if (Index != Other.Index) return Index < Other.Index;
	return false;
}

inline bool TEventKey::operator == (const TEventKey & Other) const
{
	return (TimeStamp == Other.TimeStamp) && (Index == Other.Index);
}

inline bool TEventKey::operator >  (const TEventKey & Other) const
{	
	if (TimeStamp != Other.TimeStamp) return TimeStamp > Other.TimeStamp;
	if (Index != Other.Index) return Index > Other.Index;
	return false;
}


inline bool TEventLinkKey::operator <  (const TEventLinkKey & Other) const
{
	if (Event != Other.Event) return Event < Other.Event;
	if (Contact != Other.Contact) return Contact < Other.Contact;
	return false;
}

inline bool TEventLinkKey::operator == (const TEventLinkKey & Other) const
{
	return (Event == Other.Event) && (Contact == Other.Contact);
}	

inline bool TEventLinkKey::operator >  (const TEventLinkKey & Other) const
{
	if (Event != Other.Event) return Event > Other.Event;
	if (Contact != Other.Contact) return Contact > Other.Contact;
	return false;
}



CEventsTree::CEventsTree(CBlockManager & BlockManager, TNodeRef RootNode, TDBContactHandle Contact)
:	CFileBTree(BlockManager, RootNode, cEventNodeSignature)
{
	m_Contact = Contact;
}
CEventsTree::~CEventsTree()
{

}

TDBContactHandle CEventsTree::getContact()
{
	return m_Contact;
};


CVirtualEventsTree::CVirtualEventsTree(TDBContactHandle Contact)
:	CBTree(0)
{
	m_Contact = Contact;
}
CVirtualEventsTree::~CVirtualEventsTree()
{

}

TDBContactHandle CVirtualEventsTree::getContact()
{
	return m_Contact;
};


CEventLinks::CEventLinks(CBlockManager & BlockManager, TNodeRef RootNode)
: CFileBTree(BlockManager, RootNode, cEventLinkNodeSignature)
{

}
CEventLinks::~CEventLinks()
{

}



CEventsTypeManager::CEventsTypeManager(CContacts & Contacts, CSettings & Settings)
:	m_Contacts(Contacts),
	m_Settings(Settings),
	m_Map()
{
	
}
CEventsTypeManager::~CEventsTypeManager()
{
	TTypeMap::iterator it = m_Map.begin();
	
	while (it != m_Map.end())
	{
		delete it->second->Description;
		delete it->second->ModuleName;
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

	PDBEventTypeDescriptor d = GetDescriptor(h);
	while ((d != NULL) || ((d->EventType != EventType) || (strcmp(d->ModuleName, Module) != 0)))
	{
		++h;
		d = GetDescriptor(h);
	}

	return h;
}
PDBEventTypeDescriptor CEventsTypeManager::GetDescriptor(uint32_t GlobalID)
{
	PDBEventTypeDescriptor res = NULL;

	TTypeMap::iterator it = m_Map.find(GlobalID);	

	if (it == m_Map.end())
	{
		char n[256];

		TDBSettingDescriptor d = {0};
		d.cbSize = sizeof(d);
		d.Contact = m_Contacts.getRootContact();
		d.pszSettingName = n;

		TDBSetting sid = {0};
		TDBSetting sname = {0};
		TDBSetting sdesc = {0};

		sid.cbSize = sizeof(sid);
		sid.Descriptor = &d;
		sid.Type = DB_ST_INT;

		sname.cbSize = sizeof(sname);
		sname.Descriptor = &d;
		sname.Type = DB_ST_ASCIIZ;

		sdesc.cbSize = sizeof(sdesc);
		sdesc.Descriptor = &d;
		sdesc.Type = DB_ST_ASCIIZ;

		sprintf_s(n, 256, "$EventTypes/%xModuleID", GlobalID);
		TDBSettingHandle h = m_Settings.ReadSetting(sid);
		
		if ((h != DB_INVALIDPARAM) && (h != 0))
		{
			sprintf_s(n, 256, "$EventTypes/%xModuleName", GlobalID);
			h = m_Settings.ReadSetting(sname);

			if ((h != DB_INVALIDPARAM) && (h != 0))
			{
				sprintf_s(n, 256, "$EventTypes/%xDescription", GlobalID);
				h = m_Settings.ReadSetting(sdesc);

				if ((h != DB_INVALIDPARAM) && (h != 0))
				{
					res = new TDBEventTypeDescriptor;
					res->cbSize = sizeof(TDBEventTypeDescriptor);
					res->EventType = sid.Value.Int;
					
					res->ModuleName = new char[sname.Value.Length];
					strcpy_s(res->ModuleName, sname.Value.Length, sname.Value.pAnsii);

					res->Description = new char[sdesc.Value.Length];
					strcpy_s(res->Description, sdesc.Value.Length, sdesc.Value.pAnsii);
					mir_free(sdesc.Value.pAnsii);

					m_Map.insert(std::make_pair(GlobalID, res));
				}

				mir_free(sname.Value.pAnsii);
			}
		}
	} else {
		res = it->second;

	}

	return res;
}

uint32_t CEventsTypeManager::EnsureIDExists(char* Module, uint32_t EventType, char* Description = NULL)
{
	uint32_t res = MakeGlobalID(Module, EventType);
	if (GetDescriptor(res) == NULL)
	{
		char n[256];

		TDBSettingDescriptor d = {0};
		d.cbSize = sizeof(d);
		d.pszSettingName = n;
		d.Contact = m_Contacts.getRootContact();

		TDBSetting s = {0};
		s.cbSize = sizeof(s);
		s.Descriptor = &d;

		sprintf_s(n, 256, "$EventTypes/%xModuleID", res);
		s.Type = DB_ST_INT;
		s.Value.Int = EventType;
		m_Settings.WriteSetting(s);

		sprintf_s(n, 256, "$EventTypes/%xModuleName", res);
		s.Type = DB_ST_ASCIIZ;
		s.Value.Length = strlen(Module) + 1;
		s.Value.pAnsii = Module;
		m_Settings.WriteSetting(s);

		sprintf_s(n, 256, "$EventTypes/%xDescription", res);
		if (Description)
		{
			s.Value.Length = strlen(Module) + 1;
			s.Value.pAnsii = Description;
			m_Settings.WriteSetting(s);
		} else {
			char tmp[256];
			sprintf_s(tmp, 256, "??? %s - %x ???", Module, EventType);
			s.Value.Length = strlen(Module);
			s.Value.pAnsii = tmp;
			m_Settings.WriteSetting(s);
		}
	}

	return res;
}


CEvents::CEvents(CBlockManager & BlockManager, CEventLinks::TNodeRef LinkRootNode, CMultiReadExclusiveWriteSynchronizer & Synchronize, CContacts & Contacts, CSettings & Settings)
:	m_Sync(Synchronize),
	m_BlockManager(BlockManager),	
	m_Contacts(Contacts),
	m_Types(Contacts, Settings),
	m_Links(BlockManager, LinkRootNode),
	m_EventsMap(),
	m_VirtualEventsMap(),
	m_VirtualOwnerMap()
{
	srand(_time32(NULL) + GetTickCount() + GetCurrentThreadId());
	m_Counter = rand() & 0xffff;

	m_IterAllocSize = 1;
	m_Iterations = (PEventIteration*) malloc(sizeof(PEventIteration));

	m_Cipher = NULL;
}
CEvents::~CEvents()
{
	m_Sync.BeginWrite();

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

	for (unsigned int i = 0; i < m_IterAllocSize; i++)
	{
		if (m_Iterations[i])
			IterationClose(i + 1);
	}
	free(m_Iterations);
	m_Sync.EndWrite();

}

void CEvents::SetCipher(CCipher * Cipher)
{
  m_Cipher = Cipher;
}
CEventLinks::TOnRootChanged & CEvents::sigLinkRootChanged() 
{
	return m_Links.sigRootChanged();
}


void CEvents::onRootChanged(void* EventsTree, CEventsTree::TNodeRef NewRoot)
{
	m_Contacts._setEventsRoot(((CEventsTree*)EventsTree)->getContact(), NewRoot);
}

CEventsTree * CEvents::getEventsTree(TDBContactHandle hContact)
{
	TEventsTreeMap::iterator i = m_EventsMap.find(hContact);
	if (i != m_EventsMap.end())
		return i->second;

	uint32_t root = m_Contacts._getEventsRoot(hContact);
	if (root == DB_INVALIDPARAM)
		return NULL;

	CEventsTree * tree = new CEventsTree(m_BlockManager, root, hContact);
	tree->sigRootChanged().connect(this, &CEvents::onRootChanged);
	m_EventsMap.insert(std::make_pair(hContact, tree));

	return tree;	
}
CVirtualEventsTree * CEvents::getVirtualEventsTree(TDBContactHandle hContact)
{
	TVirtualEventsTreeMap::iterator i = m_VirtualEventsMap.find(hContact);
	if (i != m_VirtualEventsMap.end())
		return i->second;

	CVirtualEventsTree * tree = new CVirtualEventsTree(hContact);
	tree->sigRootChanged().connect(this, &CEvents::onRootChanged);
	m_VirtualEventsMap.insert(std::make_pair(hContact, tree));

	return tree;		
}



unsigned int CEvents::TypeRegister(TDBEventTypeDescriptor & Type)
{
	return m_Types.EnsureIDExists(Type.ModuleName, Type.EventType, Type.Description);
}
PDBEventTypeDescriptor CEvents::TypeGet(char * ModuleName, uint32_t EventType)
{
	return m_Types.GetDescriptor(m_Types.MakeGlobalID(ModuleName, EventType));
}

unsigned int CEvents::GetBlobSize(TDBEventHandle hEvent)
{
	uint32_t sig = cEventSignature;
	uint32_t s;

	m_Sync.BeginRead();
	if (!m_BlockManager.ReadPart(hEvent, &s, offsetof(TEvent, DataLength), sizeof(s), sig))
		s = DB_INVALIDPARAM;

	m_Sync.EndRead();

	return s;
}
unsigned int CEvents::Get(TDBEventHandle hEvent, TDBEvent & Event)
{
	uint32_t sig = cEventSignature;
	uint32_t size = 0;
	void * buf = NULL;

	m_Sync.BeginRead();

	if (!m_BlockManager.ReadBlock(hEvent, buf, size, sig))
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
	}

	TEvent * ev = (TEvent*)buf;
	uint8_t * blob = (uint8_t *)(ev + 1);
	PDBEventTypeDescriptor d = m_Types.GetDescriptor(ev->Type);

	m_Sync.EndRead();  // we leave here.

	if (m_Cipher)
	{
		uint32_t cryptsize = ev->DataLength;
		if (cryptsize % m_Cipher->BlockSizeBytes() > 0)
			cryptsize = cryptsize - cryptsize % m_Cipher->BlockSizeBytes() + m_Cipher->BlockSizeBytes();

		m_Cipher->Decrypt(blob, cryptsize, hEvent); 
	}

	if (d)
	{
		Event.EventType = d->EventType;
		Event.ModuleName = d->ModuleName;
	} else {
		Event.EventType = ev->Type;
		Event.ModuleName = "???";
	}

	Event.Flags = ev->Flags;
	if (m_BlockManager.IsForcedVirtual(hEvent))
		Event.Flags |= DB_EF_VIRTUAL;

	Event.Timestamp = ev->TimeStamp;

	if (Event.pBlob)
	{
		if (Event.cbBlob >= ev->DataLength)
			memcpy(Event.pBlob, ev + 1, ev->DataLength);
		else
			memcpy(Event.pBlob, ev + 1, Event.cbBlob);		
	} else {
		Event.pBlob = (uint8_t*)mir_alloc(ev->DataLength);
		memcpy(Event.pBlob, ev + 1, ev->DataLength);
	}

	Event.cbBlob = ev->DataLength;

	free(buf);
	return 0;
}
unsigned int CEvents::Delete(TDBContactHandle hContact, TDBEventHandle hEvent)
{
	uint32_t sig = cEventSignature;
	uint32_t flags;
	TEventKey key;

	m_Sync.BeginWrite();
	if (!m_BlockManager.ReadPart(hEvent, &flags, offsetof(TEvent, Flags), sizeof(flags), sig) ||
		  !m_BlockManager.ReadPart(hEvent, &key.TimeStamp, offsetof(TEvent, TimeStamp), sizeof(key.TimeStamp), sig) ||
		  !m_BlockManager.ReadPart(hEvent, &key.Index, offsetof(TEvent, Index), sizeof(key.Index), sig))
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	TVirtualOwnerMap::iterator mit = m_VirtualOwnerMap.find(hEvent);
	TVirtualOwnerSet::iterator sit;
	if (mit != m_VirtualOwnerMap.end())
		sit = mit->second->find(hContact);

	if ((mit != m_VirtualOwnerMap.end()) && (sit != mit->second->end())) 
	{ // virtual event
		CVirtualEventsTree* tree = getVirtualEventsTree(hContact);

		if (tree == NULL)
		{
			m_Sync.EndWrite();
			return DB_INVALIDPARAM;
		}		
		tree->Delete(key);

		mit->second->erase(sit);
		if (mit->second->empty())
		{
			delete mit->second;
			m_VirtualOwnerMap.erase(mit);

			if (m_BlockManager.IsForcedVirtual(hEvent))
				m_BlockManager.DeleteBlock(hEvent);
		}

	} else { // real event
		CEventsTree* tree = getEventsTree(hContact);

		if (tree == NULL)
		{
			m_Sync.EndWrite();
			return DB_INVALIDPARAM;
		}
		tree->Delete(key);

		if (flags & DB_EF_REFERENCECOUNTING)
		{
			uint32_t ref;
			m_BlockManager.ReadPart(hEvent, &ref, offsetof(TEvent, ReferenceCount), sizeof(ref), sig);
			--ref;

			TEventLinkKey lkey = {0};
			lkey.Event = hEvent;
			lkey.Contact = hContact;
			m_Links.Delete(lkey);

			if (ref == 1)
			{
				flags = flags & ~DB_EF_REFERENCECOUNTING;
				m_BlockManager.WritePart(hEvent, &flags, offsetof(TEvent, Flags), sizeof(flags));

				lkey.Contact = 0;
				CEventLinks::iterator it = m_Links.LowerBound(lkey);
				lkey = it.Key();
				m_Links.Delete(it);
				m_BlockManager.WritePart(hEvent, &lkey.Contact, offsetof(TEvent, Contact), sizeof(lkey.Contact));
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

	m_Sync.EndWrite();
	return 0;
}
TDBEventHandle CEvents::Add(TDBContactHandle hContact, TDBEvent & Event)
{
	uint32_t sig = cEventSignature;
	CVirtualEventsTree* vtree;
	CEventsTree* tree;
	TDBEventHandle res = 0;

	uint32_t cryptsize = Event.cbBlob;
	uint8_t *blobdata = Event.pBlob;
	if (m_Cipher)
	{
		if (cryptsize % m_Cipher->BlockSizeBytes() > 0)
			cryptsize = cryptsize - cryptsize % m_Cipher->BlockSizeBytes() + m_Cipher->BlockSizeBytes();

		blobdata = (uint8_t*) malloc(cryptsize);
		memset(blobdata, 0, cryptsize);
		memcpy(blobdata, Event.pBlob, Event.cbBlob);
	}

	m_Sync.BeginWrite();

	if (Event.Flags & DB_EF_VIRTUAL)
	{
		vtree = getVirtualEventsTree(hContact);
		if (vtree != NULL)
			res = m_BlockManager.CreateBlockVirtual(sizeof(TEvent) + cryptsize, cEventSignature);
		
	} else {
		tree = getEventsTree(hContact);
		if (tree != NULL)
			res = m_BlockManager.CreateBlock(sizeof(TEvent) + cryptsize, cEventSignature);
	}

	if (res == 0)
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

  if (m_Cipher)
		m_Cipher->Encrypt(blobdata, cryptsize, res);

	TEvent ev = {0};
	TEventKey key = {0};

	ev.TimeStamp = Event.Timestamp;
	ev.Index = m_Counter;
	m_Counter += rand() & 0xffff + 1;

	key.TimeStamp = ev.TimeStamp;
	key.Index = ev.Index;


	ev.Flags = Event.Flags & ~(DB_EF_VIRTUAL | DB_EF_REFERENCECOUNTING); 
	ev.Type = m_Types.EnsureIDExists(Event.ModuleName, Event.EventType, NULL);
	ev.DataLength = Event.cbBlob;
	ev.Contact = hContact;

	m_BlockManager.WritePart(res, &ev, 0, sizeof(ev));
	m_BlockManager.WritePart(res, blobdata, sizeof(ev), cryptsize);

	if (m_Cipher)
		free(blobdata);

	if (Event.Flags & DB_EF_VIRTUAL)
		vtree->Insert(key, res);
	else
		tree->Insert(key, res);

	m_Sync.EndWrite();

	return res;
}
unsigned int CEvents::MarkRead(TDBContactHandle hContact, TDBEventHandle hEvent)
{
	uint32_t sig = cEventSignature;
	TEventKey key = {0};
	
	m_Sync.BeginWrite();

	if (!m_BlockManager.ReadPart(hEvent, &key.TimeStamp, offsetof(TEvent, TimeStamp), sizeof(key.TimeStamp), sig) ||
		  !m_BlockManager.ReadPart(hEvent, &key.Index, offsetof(TEvent, Index), sizeof(key.Index), sig))
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	CVirtualEventsTree* vtree = getVirtualEventsTree(hContact);
	CEventsTree* tree = getEventsTree(hContact);

	if ((vtree == NULL) || (tree == NULL))
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

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
			TDBEventHandle ev = heap.Top().Data();
			uint32_t flags;

			heap.Pop();

			if (m_BlockManager.ReadPart(ev, &flags, offsetof(TEvent, Flags), sizeof(flags), sig))
			{
				if ((flags & DB_EF_READ) == 0)
				{
					b = true;
					flags = flags | DB_EF_READ;
					if (res == 0)
						res = flags;

					m_BlockManager.WritePart(ev, &flags, offsetof(TEvent, Flags), sizeof(flags));
				}
			}
		}
	} while (b);

	m_Sync.EndWrite();

	return res;
}
unsigned int CEvents::WriteToDisk(TDBContactHandle hContact, TDBEventHandle hEvent)
{
	uint32_t sig = cEventSignature;
	TEventKey key;
	uint32_t flags;

	m_Sync.BeginWrite();

	CVirtualEventsTree * vtree = getVirtualEventsTree(hContact);
	CEventsTree * tree = getEventsTree(hContact);

	if ((vtree == NULL) || (tree == NULL) ||
		  !m_BlockManager.ReadPart(hEvent, &flags, offsetof(TEvent, Flags), sizeof(flags), sig) ||
		  !m_BlockManager.ReadPart(hEvent, &key.TimeStamp, offsetof(TEvent, TimeStamp), sizeof(key.TimeStamp), sig) ||
		  !m_BlockManager.ReadPart(hEvent, &key.Index, offsetof(TEvent, Index), sizeof(key.Index), sig))
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	TVirtualOwnerMap::iterator mit = m_VirtualOwnerMap.find(hEvent);
	if (mit == m_VirtualOwnerMap.end())
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	TVirtualOwnerSet::iterator sit = mit->second->find(hContact);
	if (sit == mit->second->end())
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	mit->second->erase(sit);
	if (mit->second->empty())
	{
		delete mit->second;
		m_VirtualOwnerMap.erase(mit);
	}

	vtree->Delete(key);
	tree->Insert(key, hEvent);

	if (m_BlockManager.IsForcedVirtual(hEvent))
	{
		m_BlockManager.WritePart(hEvent, &hContact, offsetof(TEvent, Contact), sizeof(hContact));
		m_BlockManager.WriteBlockToDisk(hEvent);
	} else {
		TEventLinkKey lkey;
		lkey.Event = hEvent;

		if ((flags & DB_EF_REFERENCECOUNTING) == 0)
		{
			TDBContactHandle tmp;
			m_BlockManager.ReadPart(hEvent, &tmp, offsetof(TEvent, Contact), sizeof(tmp), sig);
			
			lkey.Contact = tmp;
			m_Links.Insert(lkey, TEmpty());

			uint32_t ref = 2;
			flags = flags | DB_EF_REFERENCECOUNTING;

			m_BlockManager.WritePart(hEvent, &flags, offsetof(TEvent, Flags), sizeof(flags));
			m_BlockManager.WritePart(hEvent, &ref, offsetof(TEvent, ReferenceCount), sizeof(ref));
			
		} else {
			uint32_t ref;
			m_BlockManager.ReadPart(hEvent, &ref, offsetof(TEvent, ReferenceCount), sizeof(ref), sig);
			++ref;
			m_BlockManager.WritePart(hEvent, &ref, offsetof(TEvent, ReferenceCount), sizeof(ref));
		}

		lkey.Contact = hContact;
		m_Links.Insert(lkey, TEmpty());
	}

	m_Sync.EndWrite();

	return flags;
}

TDBContactHandle CEvents::GetContact(TDBEventHandle hEvent)
{
	uint32_t sig = cEventSignature;
	uint32_t flags;
	TDBContactHandle res = 0;

	m_Sync.BeginRead();

	if (!m_BlockManager.ReadPart(hEvent, &flags, offsetof(TEvent, Flags), sizeof(flags), sig))
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
	}

	if (m_BlockManager.IsForcedVirtual(hEvent))
	{		
		TVirtualOwnerMap::iterator mit = m_VirtualOwnerMap.find(hEvent);
		if (mit != m_VirtualOwnerMap.end())
			res = *mit->second->begin();	
	} else {
		if (flags & DB_EF_REFERENCECOUNTING)
		{
			TEventLinkKey lkey;
			lkey.Event = hEvent;
			lkey.Contact = 0;
			CEventLinks::iterator it = m_Links.LowerBound(lkey);
			if ((it) && (it.Key().Event == hEvent))
				res = it.Key().Contact;
		} else {
			m_BlockManager.ReadPart(hEvent, &res, offsetof(TEvent, Contact), sizeof(res), sig);
		}
	}

	m_Sync.EndRead();
	return res;
}

unsigned int CEvents::CreateHardLink(TDBEventHardLink & HardLink)
{
	uint32_t sig = cEventSignature;
	uint32_t flags;
	TEventKey key;

	m_Sync.BeginWrite();
	
	if (!m_BlockManager.ReadPart(HardLink.hEvent, &flags, offsetof(TEvent, Flags), sizeof(flags), sig) ||
		  !m_BlockManager.ReadPart(HardLink.hEvent, &key.TimeStamp, offsetof(TEvent, TimeStamp), sizeof(key.TimeStamp), sig) ||
			!m_BlockManager.ReadPart(HardLink.hEvent, &key.Index, offsetof(TEvent, Index), sizeof(key.Index), sig))
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	if (HardLink.Flags & DB_EF_VIRTUAL)
	{
		CVirtualEventsTree * vtree = getVirtualEventsTree(HardLink.hContact);
		if (vtree == NULL)
		{
			m_Sync.EndWrite();
			return DB_INVALIDPARAM;
		}

		vtree->Insert(key, HardLink.hEvent);

		TVirtualOwnerMap::iterator mit = m_VirtualOwnerMap.find(HardLink.hContact);
		if (mit == m_VirtualOwnerMap.end())
		{
			TVirtualOwnerSet * tmp = new TVirtualOwnerSet();
			tmp->insert(HardLink.hContact);
			m_VirtualOwnerMap.insert(std::make_pair(HardLink.hEvent, tmp));
			
		} else {
			mit->second->insert(HardLink.hContact);
		}

	} else {
		CEventsTree * tree = getEventsTree(HardLink.hContact);

		if (tree == NULL)
		{
			m_Sync.EndWrite();
			return DB_INVALIDPARAM;
		}

		tree->Insert(key, HardLink.hEvent);
		if (m_BlockManager.IsForcedVirtual(HardLink.hEvent))
		{
			m_BlockManager.WritePart(HardLink.hEvent, &HardLink.hContact, offsetof(TEvent, Contact), sizeof(HardLink.hContact));
			if (flags & DB_EF_REFERENCECOUNTING)
			{
				flags = flags & ~DB_EF_REFERENCECOUNTING;
				m_BlockManager.WritePart(HardLink.hEvent, &flags, offsetof(TEvent, Flags), sizeof(flags));
			}
			m_BlockManager.WriteBlockToDisk(HardLink.hEvent);
		} else {
			TEventLinkKey lkey;
			uint32_t ref;
			lkey.Event = HardLink.hEvent;

			if ((flags & DB_EF_REFERENCECOUNTING) == 0)
			{
				flags = flags | DB_EF_REFERENCECOUNTING;
				m_BlockManager.WritePart(HardLink.hEvent, &flags, offsetof(TEvent, Flags), sizeof(flags));

				m_BlockManager.ReadPart(HardLink.hEvent, &lkey.Contact, offsetof(TEvent, Contact), sizeof(lkey.Contact), sig);
				m_Links.Insert(lkey, TEmpty());

				ref = 1;
			} else {
				m_BlockManager.ReadPart(HardLink.hEvent, &ref, offsetof(TEvent, ReferenceCount), sizeof(ref), sig); 
			}

			lkey.Contact = HardLink.hContact;
			m_Links.Insert(lkey, TEmpty());

			m_BlockManager.WritePart(HardLink.hEvent, &ref, offsetof(TEvent, ReferenceCount), sizeof(ref));
		}
	}

	m_Sync.EndWrite();

	return 0;
}

TDBEventIterationHandle CEvents::IterationInit(TDBEventIterFilter & Filter)
{
	m_Sync.BeginWrite();

	unsigned int i = 0;

	while ((i < m_IterAllocSize) && (m_Iterations[i] != NULL))
		i++;

	if (i == m_IterAllocSize)
	{
		m_IterAllocSize = m_IterAllocSize << 1;
		m_Iterations = (PEventIteration*)realloc(m_Iterations, sizeof(PEventIteration*) * m_IterAllocSize);
	}

	CEventsTree * tree = getEventsTree(Filter.hContact);
	CVirtualEventsTree * vtree = getVirtualEventsTree(Filter.hContact);

	if ((tree == NULL) || (vtree == NULL))
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	std::queue<TEventBase * > q;
	q.push(tree);
	q.push(vtree);

	TDBContactIterFilter f = {0};
	f.cbSize = sizeof(f);
	f.Options = Filter.Options;
	
	TDBContactIterationHandle citer = m_Contacts.IterationInit(f, Filter.hContact);
	if (citer != DB_INVALIDPARAM)
	{
		m_Contacts.IterationNext(citer);
		TDBContactHandle c = m_Contacts.IterationNext(citer);
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

			c = m_Contacts.IterationNext(citer);
		}

		m_Contacts.IterationClose(citer);
	}

	for (unsigned int j = 0; j < Filter.ExtraCount; ++j)
	{
		tree = getEventsTree(Filter.ExtraContacts[j]);
		if (tree)
		{
			q.push(tree);
			
			vtree = getVirtualEventsTree(Filter.ExtraContacts[j]);
			if (vtree)
				q.push(vtree);
		}
	}	

	PEventIteration iter = new TEventIteration;
	iter->Filter = Filter;
	iter->LastEvent = 0;

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
		i = DB_INVALIDPARAM - 1;
	} else {
		m_Iterations[i] = iter;
	}

	m_Sync.EndWrite();

	return i + 1;
}
TDBEventHandle CEvents::IterationNext(TDBEventIterationHandle Iteration)
{
	m_Sync.BeginRead();

	if (Iteration == 0)
		return 0;

	if ((Iteration > m_IterAllocSize) || (m_Iterations[Iteration - 1] == NULL))
	{
		m_Sync.EndRead();
		return DB_INVALIDPARAM;
	}

	PEventIteration iter = m_Iterations[Iteration - 1];
	uint32_t sig = cEventSignature;

	TDBEventHandle res = 0;
	TEventBase::iterator it = iter->Heap->Top();
	
	while ((it) && (it.Key().TimeStamp <= iter->Filter.tTill) && (it.Data() == iter->LastEvent))
	{
		iter->Heap->Pop();
		it = iter->Heap->Top();
	}

	if ((it) && (it.Key().TimeStamp <= iter->Filter.tTill))
	{
		res = it.Data();
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

	m_Sync.EndRead();

	return res;
}
unsigned int CEvents::IterationClose(TDBEventIterationHandle Iteration)
{
	m_Sync.BeginWrite();

	if ((Iteration > m_IterAllocSize) || (Iteration == 0) || (m_Iterations[Iteration - 1] == NULL))
	{
		m_Sync.EndWrite();
		return DB_INVALIDPARAM;
	}

	delete m_Iterations[Iteration - 1]->Heap;
	delete m_Iterations[Iteration - 1];

	m_Iterations[Iteration - 1] = NULL;

	m_Sync.EndWrite();

	return 0;
}