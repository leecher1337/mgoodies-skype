#include "Contacts.h"

inline bool TVirtualKey::operator <  (const TVirtualKey & Other) const
{
	if (RealContact != Other.RealContact) return RealContact < Other.RealContact;
	if (Virtual != Other.Virtual) return Virtual < Other.Virtual;
	return false;
}

inline bool TVirtualKey::operator == (const TVirtualKey & Other) const
{
	return (RealContact == Other.RealContact) && (Virtual == Other.Virtual);
}

inline bool TVirtualKey::operator >  (const TVirtualKey & Other) const
{	
	if (RealContact != Other.RealContact) return RealContact > Other.RealContact;
	if (Virtual != Other.Virtual) return Virtual > Other.Virtual;
	return false;
}


inline bool TContactKey::operator <  (const TContactKey & Other) const
{
	if (Level != Other.Level) return Level < Other.Level;
	if (Parent != Other.Parent) return Parent < Other.Parent;
	if (Contact != Other.Contact) return Contact < Other.Contact;
	return false;
}

inline bool TContactKey::operator == (const TContactKey & Other) const
{
	return (Level == Other.Level) && (Parent == Other.Parent) && (Contact == Other.Contact);
}

inline bool TContactKey::operator >  (const TContactKey & Other) const
{	
	if (Level != Other.Level) return Level > Other.Level;
	if (Parent != Other.Parent) return Parent > Other.Parent;
	if (Contact != Other.Contact) return Contact > Other.Contact;
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

TDBTContactHandle CVirtuals::_DeleteRealContact(TDBTContactHandle hRealContact)
{
	TDBTContactHandle result;
	TVirtualKey key;
	TContact Contact;
	bool copies = false;
	uint32_t sig = cContactSignature;

	key.RealContact = hRealContact;
	key.Virtual = 0;

	iterator i = LowerBound(key);
	result = i->Virtual;
	i.setManaged();
	Delete(*i);

	while ((i) && (i->RealContact == hRealContact))
	{
		key = *i;
		Delete(*i);

		key.RealContact = result;		
		Insert(key);		

		Contact.VParent = result;
		m_BlockManager.WritePart(key.Virtual, &Contact.VParent, offsetof(TContact, VParent), sizeof(TDBTContactHandle));
	
		copies = true;
	}

	m_BlockManager.ReadPart(result, &Contact.Flags, offsetof(TContact, Flags), sizeof(uint32_t), sig);
	Contact.Flags = Contact.Flags & ~(DBT_CF_HasVirtuals | DBT_CF_IsVirtual);
	if (copies)
		Contact.Flags |= DBT_CF_HasVirtuals;

	m_BlockManager.WritePart(result, &Contact.Flags, offsetof(TContact, Flags), sizeof(uint32_t));

	return result;
}

bool CVirtuals::_InsertVirtual(TDBTContactHandle hRealContact, TDBTContactHandle hVirtual)
{
	TVirtualKey key;
	key.RealContact = hRealContact;
	key.Virtual = hVirtual;

	Insert(key);

	return true;
}
void CVirtuals::_DeleteVirtual(TDBTContactHandle hRealContact, TDBTContactHandle hVirtual)
{
	TVirtualKey key;
	key.RealContact = hRealContact;
	key.Virtual = hVirtual;

	Delete(key);
}
TDBTContactHandle CVirtuals::getParent(TDBTContactHandle hVirtual)
{
	TContact Contact;
	void* p = &Contact;
	uint32_t size = sizeof(Contact);
	uint32_t sig = cContactSignature;

	SYNC_BEGINREAD(m_Sync);
	if (!m_BlockManager.ReadBlock(hVirtual, p, size, sig) || 
	   ((Contact.Flags & DBT_CF_IsVirtual) == 0))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	SYNC_ENDREAD(m_Sync);	
	return Contact.VParent;
}
TDBTContactHandle CVirtuals::getFirst(TDBTContactHandle hRealContact)
{
	TContact Contact;
	void* p = &Contact;
	uint32_t size = sizeof(Contact);
	uint32_t sig = cContactSignature;

	SYNC_BEGINREAD(m_Sync);
	

	if (!m_BlockManager.ReadBlock(hRealContact, p, size, sig) || 
	   ((Contact.Flags & DBT_CF_HasVirtuals) == 0))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}
	
	TVirtualKey key;
	key.RealContact = hRealContact;
	key.Virtual = 0;

	iterator i = LowerBound(key);

	if ((i) && (i->RealContact == hRealContact))
		key.Virtual = i->Virtual;
	else
		key.Virtual = 0;

	SYNC_ENDREAD(m_Sync);

	return key.Virtual;
}
TDBTContactHandle CVirtuals::getNext(TDBTContactHandle hVirtual)
{
	TContact Contact;
	void* p = &Contact;
	uint32_t size = sizeof(Contact);
	uint32_t sig = cContactSignature;

	SYNC_BEGINREAD(m_Sync);
	
	if (!m_BlockManager.ReadBlock(hVirtual, p, size, sig) || 
	   ((Contact.Flags & DBT_CF_IsVirtual) == 0))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}
	
	TVirtualKey key;
	key.RealContact = Contact.VParent;
	key.Virtual = hVirtual + 1;

	iterator i = LowerBound(key);
	
	if ((i) && (i->RealContact == Contact.VParent))
		key.Virtual = i->Virtual;
	else
		key.Virtual = 0;

	SYNC_ENDREAD(m_Sync);

	return key.Virtual;
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CContacts::CContacts(CBlockManager & BlockManager, CMultiReadExclusiveWriteSynchronizer & Synchronize, TDBTContactHandle RootContact, TNodeRef ContactRoot, CVirtuals::TNodeRef VirtualRoot)
: CFileBTree(BlockManager, ContactRoot, cContactNodeSignature),
	m_Sync(Synchronize),
	m_Virtuals(BlockManager, Synchronize, VirtualRoot),

	m_sigContactDelete(),
	m_sigInternalDeleteEvents(),
	m_sigInternalDeleteSettings(),
	m_sigInternalMergeSettings(),
	m_sigInternalTransferEvents()
{
	if (RootContact == 0)
		m_RootContact = CreateRootContact();
	else
		m_RootContact = RootContact;
	
}

CContacts::~CContacts()
{

}

TDBTContactHandle CContacts::CreateRootContact()
{
	TContact Contact = {0};
	TContactKey key = {0};

	Contact.Flags = DBT_CF_IsGroup | DBT_CF_IsRoot;
	key.Contact = m_BlockManager.CreateBlock(sizeof(Contact), cContactSignature);
	m_BlockManager.WriteBlock(key.Contact, &Contact, sizeof(Contact), cContactSignature);
	Insert(key);
	return key.Contact;
}


CVirtuals::TOnRootChanged & CContacts::sigVirtualRootChanged()
{
	return m_Virtuals.sigRootChanged();
}

CContacts::TOnContactDelete &          CContacts::sigContactDelete()
{
	return m_sigContactDelete;
}
CContacts::TOnInternalDeleteEvents &   CContacts::_sigDeleteEvents()  
{
	return m_sigInternalDeleteEvents;
}
CContacts::TOnInternalDeleteSettings & CContacts::_sigDeleteSettings()
{
	return m_sigInternalDeleteSettings;
}
CContacts::TOnInternalMergeSettings &  CContacts::_sigMergeSettings() 
{
	return m_sigInternalMergeSettings;
}
CContacts::TOnInternalTransferEvents & CContacts::_sigTransferEvents()
{
	return m_sigInternalTransferEvents;
}


uint32_t CContacts::_getSettingsRoot(TDBTContactHandle hContact)
{
	/*CSettingsTree::TNodeRef*/
	uint32_t set;
	uint32_t sig = cContactSignature;

	if (!m_BlockManager.ReadPart(hContact, &set, offsetof(TContact, Settings), sizeof(set), sig))
		return DBT_INVALIDPARAM;

	return set;
}
bool CContacts::_setSettingsRoot(TDBTContactHandle hContact, /*CSettingsTree::TNodeRef*/ uint32_t NewRoot)
{
	uint32_t sig = cContactSignature;

	if (!m_BlockManager.WritePartCheck(hContact, &NewRoot, offsetof(TContact, Settings), sizeof(NewRoot), sig))
		return false;
	
	return true;
}

uint32_t CContacts::_getEventsRoot(TDBTContactHandle hContact)
{
	/*CEventsTree::TNodeRef*/
	uint32_t ev;
	uint32_t sig = cContactSignature;

	if (!m_BlockManager.ReadPart(hContact, &ev, offsetof(TContact, Events), sizeof(ev), sig))
		return DBT_INVALIDPARAM;

	return ev;
}
bool CContacts::_setEventsRoot(TDBTContactHandle hContact, /*CEventsTree::TNodeRef*/ uint32_t NewRoot)
{
	uint32_t sig = cContactSignature;

	if (!m_BlockManager.WritePartCheck(hContact, &NewRoot, offsetof(TContact, Events), sizeof(NewRoot), sig))
		return false;
	
	return true;
}

uint32_t CContacts::_getEventCount(TDBTContactHandle hContact)
{
	uint32_t res = 0;
	uint32_t sig = cContactSignature;

	if (!m_BlockManager.ReadPart(hContact, &res, offsetof(TContact, EventCount), sizeof(res), sig))
		return DBT_INVALIDPARAM;
	
	return res;
}

uint32_t CContacts::_adjustEventCount(TDBTContactHandle hContact, int32_t Adjust)
{
	uint32_t sig = cContactSignature;
	uint32_t c;

	if (m_BlockManager.ReadPart(hContact, &c, offsetof(TContact, EventCount), sizeof(c), sig))
	{
		if (((Adjust < 0) && ((uint32_t)(-Adjust) <= c)) || 
			  ((Adjust > 0) && ((0xffffffff - c) > (uint32_t)Adjust)))
		{
			c += Adjust;
			m_BlockManager.WritePart(hContact, &c, offsetof(TContact, EventCount), sizeof(c));
		}
	}

	return c;
}
CVirtuals & CContacts::_getVirtuals()
{
	return m_Virtuals;
}

TDBTContactHandle CContacts::getRootContact()
{
	return m_RootContact;
}

TDBTContactHandle CContacts::getParent(TDBTContactHandle hContact)
{
	TDBTContactHandle par;
	uint32_t sig = cContactSignature;

	SYNC_BEGINREAD(m_Sync);	
	if (!m_BlockManager.ReadPart(hContact, &par, offsetof(TContact, ParentContact), sizeof(par), sig))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	SYNC_ENDREAD(m_Sync);	
	return par;
}
TDBTContactHandle CContacts::setParent(TDBTContactHandle hContact, TDBTContactHandle hParent)
{
	TContact Contact;
	void* pContact = &Contact;
	uint32_t size = sizeof(TContact);
	uint32_t sig = cContactSignature;
	uint16_t cn, co;
	uint32_t fn, fo;
	uint16_t l;
	
	SYNC_BEGINWRITE(m_Sync);
	if (!m_BlockManager.ReadBlock(hContact, pContact, size, sig) ||
		  !m_BlockManager.ReadPart(hParent, &cn, offsetof(TContact,ChildCount), sizeof(cn), sig) ||
			!m_BlockManager.ReadPart(Contact.ParentContact, &co, offsetof(TContact, ChildCount), sizeof(co), sig) ||
			!m_BlockManager.ReadPart(hParent, &l, offsetof(TContact, Level), sizeof(l), sig))
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	// update parents
	--co;
	++cn;

	m_BlockManager.WritePart(Contact.ParentContact, &co, offsetof(TContact, ChildCount),sizeof(co));
	if (co == 0)
	{
		m_BlockManager.ReadPart(Contact.ParentContact, &fo, offsetof(TContact, Flags), sizeof(fo), sig);
		fo = fo & ~DBT_CF_HasChildren;
		m_BlockManager.WritePart(Contact.ParentContact, &fo, offsetof(TContact, Flags), sizeof(fo));
	}
	
	m_BlockManager.WritePart(hParent, &cn, offsetof(TContact, ChildCount), sizeof(cn));
	if (cn == 1)
	{
		m_BlockManager.ReadPart(hParent, &fn, offsetof(TContact, Flags), sizeof(fn), sig);
		fn = fn | DBT_CF_HasChildren;
		m_BlockManager.WritePart(hParent, &fn, offsetof(TContact, Flags), sizeof(fn));
	}	
	
	// update rest

	TContactKey key;
	int dif = l - Contact.Level + 1;

	if (dif == 0) // no level difference, update only moved contact
	{
		key.Contact = hContact;
		key.Level = Contact.Level;
		key.Parent = Contact.ParentContact;
		Delete(key);
		key.Parent = hParent;
		Insert(key);
		m_BlockManager.WritePart(hContact, &key.Parent, offsetof(TContact, ParentContact), sizeof(key.Parent));
		
	} else {
		TDBTContactIterFilter filter = {0};
		filter.cbSize = sizeof(filter);
		filter.Options = DBT_CIFO_OSC_AC | DBT_CIFO_OC_AC;

		TDBTContactIterationHandle iter = IterationInit(filter, hContact);

		key.Contact = IterationNext(iter);

		while ((key.Contact != 0) && (key.Contact != DBT_INVALIDPARAM))
		{
			if (m_BlockManager.ReadPart(key.Contact, &key.Parent, offsetof(TContact, ParentContact), sizeof(key.Parent), sig) &&
					m_BlockManager.ReadPart(key.Contact, &key.Level, offsetof(TContact, Level), sizeof(key.Level), sig))
			{
				Delete(key);

				if (key.Contact == hContact)
				{
					key.Parent = hParent;
					m_BlockManager.WritePart(key.Contact, &key.Parent, offsetof(TContact, ParentContact), sizeof(key.Parent));
				}
				
				key.Level = key.Level + dif;
				m_BlockManager.WritePart(key.Contact, &key.Level, offsetof(TContact, Level), sizeof(key.Level));
				
				Insert(key);
			}
			key.Contact = IterationNext(iter);
		}

		IterationClose(iter);
	}

	SYNC_ENDWRITE(m_Sync);

	/// TODO raise event

	return Contact.ParentContact;
}

unsigned int CContacts::getChildCount(TDBTContactHandle hContact)
{
	uint32_t c;
	uint32_t sig = cContactSignature;
	
	SYNC_BEGINREAD(m_Sync);
	if (!m_BlockManager.ReadPart(hContact, &c, offsetof(TContact, ChildCount), sizeof(c), sig))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}
	
	SYNC_ENDREAD(m_Sync);
	return c;
}
TDBTContactHandle CContacts::getFirstChild(TDBTContactHandle hParent)
{
	uint16_t l;
	uint32_t f;
	uint32_t sig = cContactSignature;
	TDBTContactHandle result;

	SYNC_BEGINREAD(m_Sync);
	if (!m_BlockManager.ReadPart(hParent, &f, offsetof(TContact, Flags), sizeof(f), sig) ||
		  !m_BlockManager.ReadPart(hParent, &l, offsetof(TContact, Level), sizeof(l), sig))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	if ((f & DBT_CF_HasChildren) == 0)
	{
		SYNC_ENDREAD(m_Sync);
		return 0;
	}

	TContactKey key;

	key.Level = l + 1;
	key.Parent = hParent;
	key.Contact = 0;

	iterator it = LowerBound(key);
	
	if ((!it) || (it->Parent != hParent))
	{
		SYNC_ENDREAD(m_Sync);	
		return 0;
	}
	result = it->Contact;
	SYNC_ENDREAD(m_Sync);

	return result;
}
TDBTContactHandle CContacts::getLastChild(TDBTContactHandle hParent)
{
	uint16_t l;
	uint32_t f;
	uint32_t sig = cContactSignature;
	TDBTContactHandle result;

	SYNC_BEGINREAD(m_Sync);
	if (!m_BlockManager.ReadPart(hParent, &f, offsetof(TContact, Flags), sizeof(f), sig) ||
		  !m_BlockManager.ReadPart(hParent, &l, offsetof(TContact, Level), sizeof(l), sig))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	if ((f & DBT_CF_HasChildren) == 0)
	{
		SYNC_ENDREAD(m_Sync);
		return 0;
	}

	TContactKey key;

	key.Level = l + 1;
	key.Parent = hParent;
	key.Contact = 0xFFFFFFFF;

	iterator it = UpperBound(key);
	
	if ((!it) || (it->Parent != hParent))
	{
		SYNC_ENDREAD(m_Sync);	
		return 0;
	}

	result = it->Contact;
	SYNC_ENDREAD(m_Sync);

	return result;
}
TDBTContactHandle CContacts::getNextSilbing(TDBTContactHandle hContact)
{
	uint16_t l;
	uint32_t sig = cContactSignature;
	TDBTContactHandle result, parent;

	SYNC_BEGINREAD(m_Sync);
	if (!m_BlockManager.ReadPart(hContact, &l, offsetof(TContact, Level), sizeof(l), sig) ||
		  !m_BlockManager.ReadPart(hContact, &parent, offsetof(TContact, ParentContact), sizeof(parent), sig))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	TContactKey key;

	key.Level = l;
	key.Parent = parent;
	key.Contact = hContact + 1;

	iterator it = LowerBound(key);
	
	if ((!it) || (it->Parent != parent))
	{
		SYNC_ENDREAD(m_Sync);
		return 0;
	}
	result = it->Contact;
	SYNC_ENDREAD(m_Sync);

	return result;
}
TDBTContactHandle CContacts::getPrevSilbing(TDBTContactHandle hContact)
{
	uint16_t l;
	uint32_t sig = cContactSignature;
	TDBTContactHandle result, parent;

	SYNC_BEGINREAD(m_Sync);
	if (!m_BlockManager.ReadPart(hContact, &l, offsetof(TContact, Level), sizeof(l), sig) ||
		  !m_BlockManager.ReadPart(hContact, &parent, offsetof(TContact, ParentContact), sizeof(parent), sig))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	TContactKey key;

	key.Level = l;
	key.Parent = parent;
	key.Contact = hContact - 1;

	iterator it = UpperBound(key);
	
	if ((!it) || (it->Parent != parent))
	{
		SYNC_ENDREAD(m_Sync);
		return 0;
	}
	result = it->Contact;
	SYNC_ENDREAD(m_Sync);

	return result;
}

unsigned int CContacts::getFlags(TDBTContactHandle hContact)
{
	uint32_t f;
	uint32_t sig = cContactSignature;

	SYNC_BEGINREAD(m_Sync);
	if (!m_BlockManager.ReadPart(hContact, &f, offsetof(TContact, Flags), sizeof(f), sig))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	SYNC_ENDREAD(m_Sync);
	return f;
}

TDBTContactHandle CContacts::CreateContact(TDBTContactHandle hParent, uint32_t Flags)
{
	TContact Contact = {0}, parent;
	void* pparent = &parent;
	uint32_t size = sizeof(Contact);
	uint32_t sig = cContactSignature;

	SYNC_BEGINWRITE(m_Sync);
	if (!m_BlockManager.ReadBlock(hParent, pparent, size, sig))
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	TDBTContactHandle hContact = m_BlockManager.CreateBlock(sizeof(TContact), cContactSignature);
	TContactKey key;
	
	Contact.Level = parent.Level + 1;
	Contact.ParentContact = hParent;
	Contact.Flags = Flags;
	
	m_BlockManager.WriteBlock(hContact, &Contact, sizeof(Contact), cContactSignature);
	
	key.Level = Contact.Level;
	key.Parent = hParent;
	key.Contact = hContact;

	Insert(key);
	
	if (parent.ChildCount == 0)
	{
		parent.Flags = parent.Flags | DBT_CF_HasChildren;
		m_BlockManager.WritePart(hParent, &parent.Flags, offsetof(TContact, Flags), sizeof(uint32_t));
	}
	++parent.ChildCount;
	m_BlockManager.WritePart(hParent, &parent.ChildCount, offsetof(TContact, ChildCount), sizeof(uint16_t));
	
	SYNC_ENDWRITE(m_Sync);
	return hContact;
}

unsigned int CContacts::DeleteContact(TDBTContactHandle hContact)
{
	TContact Contact;
	void* pContact = &Contact;
	uint32_t size = sizeof(Contact);
	uint32_t sig = cContactSignature;
	uint16_t parentcc;
	uint32_t parentf;

	TContactKey key;

	SYNC_BEGINWRITE(m_Sync);
	if (!m_BlockManager.ReadBlock(hContact, pContact, size, sig) ||
		  !m_BlockManager.ReadPart(Contact.ParentContact, &parentcc, offsetof(TContact, ChildCount), sizeof(parentcc), sig) ||
			!m_BlockManager.ReadPart(Contact.ParentContact, &parentf, offsetof(TContact, Flags), sizeof(parentf), sig))
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	m_sigContactDelete.emit(this, hContact);

	if (Contact.Flags & DBT_CF_HasVirtuals)
	{
		// move virtuals and make one of them real
		TDBTContactHandle newreal = m_Virtuals._DeleteRealContact(hContact);

		m_BlockManager.WritePartCheck(newreal, &Contact.EventCount, offsetof(TContact, EventCount), sizeof(uint32_t), sig);		
		m_BlockManager.WritePart(newreal, &Contact.Events, offsetof(TContact, Events), sizeof(uint32_t));

		m_sigInternalTransferEvents.emit(this, hContact, newreal);
		m_sigInternalMergeSettings.emit(this, hContact, newreal);

	} else {
		m_sigInternalDeleteEvents.emit(this, hContact);
		m_sigInternalDeleteSettings.emit(this, hContact);
	}

	key.Level = Contact.Level;
	key.Parent = Contact.ParentContact;
	key.Contact = hContact;
	Delete(key);
	
	if (Contact.Flags & DBT_CF_HasChildren) // keep the children
	{
		parentf = parentf | DBT_CF_HasChildren;
		parentcc += Contact.ChildCount;

		TDBTContactIterFilter filter = {0};
		filter.cbSize = sizeof(filter);
		filter.Options = DBT_CIFO_OSC_AC | DBT_CIFO_OC_AC;

		TDBTContactIterationHandle iter = IterationInit(filter, hContact);
		if (iter != DBT_INVALIDPARAM)
		{
			IterationNext(iter);
			key.Contact = IterationNext(iter);
			
			while ((key.Contact != 0) && (key.Contact != DBT_INVALIDPARAM))
			{
				if (m_BlockManager.ReadPart(key.Contact, &key.Parent, offsetof(TContact, ParentContact), sizeof(key.Parent), sig) &&
					m_BlockManager.ReadPart(key.Contact, &key.Level, offsetof(TContact, Level), sizeof(key.Level), sig))
				{
					Delete(key);

					if (key.Parent == hContact)
					{
						key.Parent = Contact.ParentContact;
						m_BlockManager.WritePart(key.Contact, &key.Parent, offsetof(TContact, ParentContact), sizeof(key.Parent));
					}
					
					key.Level--;
					m_BlockManager.WritePart(key.Contact, &key.Level, offsetof(TContact, Level), sizeof(key.Level));
					Insert(key);

				}
				key.Contact = IterationNext(iter);
			}

			IterationClose(iter);
		}
	}

	m_BlockManager.DeleteBlock(hContact); // we need this block to start iteration, delete it here
	--parentcc;

	if (parentcc == 0)
		Contact.Flags = Contact.Flags & (~DBT_CF_HasChildren);

	m_BlockManager.WritePartCheck(Contact.ParentContact, &parentcc, offsetof(TContact, ChildCount), sizeof(parentcc), sig);
	m_BlockManager.WritePart(Contact.ParentContact, &parentf, offsetof(TContact, Flags), sizeof(parentf));

	SYNC_ENDWRITE(m_Sync);
	return 0;
}



TDBTContactIterationHandle CContacts::IterationInit(const TDBTContactIterFilter & Filter, TDBTContactHandle hParent)
{
	uint16_t l;
	uint32_t f;
	uint32_t sig = cContactSignature;

	SYNC_BEGINREAD(m_Sync);
	
	if (!m_BlockManager.ReadPart(hParent, &l, offsetof(TContact, Level), sizeof(l), sig) ||
	    !m_BlockManager.ReadPart(hParent, &f, offsetof(TContact, Flags), sizeof(f), sig))
	{
		SYNC_ENDREAD(m_Sync);
		return DBT_INVALIDPARAM;
	}

	PContactIteration iter = new TContactIteration;
	iter->filter = Filter;
	iter->q = new std::deque<TContactIterationItem>;
	iter->parents = new std::deque<TContactIterationItem>;
	iter->returned = new stdext::hash_set<TDBTContactHandle>;
	iter->returned->insert(hParent);
	
	TContactIterationItem it;
	it.Flags = f;
	it.Handle = hParent;
	it.Level = l;
	it.Options = Filter.Options & 0x000000ff;
	it.LookupDepth = 0;

	iter->q->push_back(it);

	SYNC_ENDREAD(m_Sync);

	return (TDBTContactIterationHandle)iter;
}
TDBTContactHandle CContacts::IterationNext(TDBTContactIterationHandle Iteration)
{
	uint32_t sig = cContactSignature;

	SYNC_BEGINREAD(m_Sync);

	PContactIteration iter = (PContactIteration)Iteration;
	TContactIterationItem item;
	TDBTContactHandle result = 0;

	if (iter->q->empty())
	{
		std::deque <TContactIterationItem> * tmp = iter->q;
		iter->q = iter->parents;
		iter->parents = tmp;
	}

	if (iter->q->empty() && 
		  (iter->filter.Options & DBT_CIFO_GF_USEROOT) &&
			(iter->returned->find(m_RootContact) == iter->returned->end()))
	{
		item.Handle = m_RootContact;
		item.Level = 0;
		item.Options = 0;
		item.Flags = 0;
		item.LookupDepth = 255;

		iter->filter.Options = iter->filter.Options & ~DBT_CIFO_GF_USEROOT;

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

		std::deque<TContactIterationItem> tmp;
		TContactIterationItem newitem;

		// children
		if ((item.Flags & DBT_CF_HasChildren) &&
		  	(item.Options & DBT_CIFO_OSC_AC))
		{
			TContactKey key;
			key.Parent = item.Handle;
			key.Level = item.Level + 1;

			newitem.Level = item.Level + 1;
			newitem.LookupDepth = item.LookupDepth;
			newitem.Options = (iter->filter.Options / DBT_CIFO_OC_AC * DBT_CIFO_OSC_AC) & (DBT_CIFO_OSC_AC | DBT_CIFO_OSC_AO | DBT_CIFO_OSC_AOC | DBT_CIFO_OSC_AOP);

			if (iter->filter.Options & DBT_CIFO_GF_DEPTHFIRST)
			{
				key.Contact = 0xffffffff;

				iterator c = UpperBound(key);
				while ((c) && (c->Parent == item.Handle))
				{
					newitem.Handle = c->Contact;
					
					if ((iter->returned->find(newitem.Handle) == iter->returned->end()) &&
						  m_BlockManager.ReadPart(newitem.Handle, &newitem.Flags, offsetof(TContact, Flags), sizeof(newitem.Flags), sig) &&
						  (((newitem.Flags & DBT_CF_IsGroup) == 0) || ((DBT_CF_IsGroup & iter->filter.fHasFlags) == 0))) // if we want only groups, we don't need to trace down contacts...
					{
						iter->q->push_front(newitem);
						iter->returned->insert(newitem.Handle);
					}

					--c;
				}
			} else {
				key.Contact = 0;

				iterator c = LowerBound(key);
				while ((c) && (c->Parent == item.Handle))
				{
					newitem.Handle = c->Contact;
					
					if ((iter->returned->find(newitem.Handle) == iter->returned->end()) &&
						  m_BlockManager.ReadPart(newitem.Handle, &newitem.Flags, offsetof(TContact, Flags), sizeof(newitem.Flags), sig) &&
						  (((newitem.Flags & DBT_CF_IsGroup) == 0) || ((DBT_CF_IsGroup & iter->filter.fHasFlags) == 0))) // if we want only groups, we don't need to trace down contacts...
					{
						iter->q->push_back(newitem);
						iter->returned->insert(newitem.Handle);
					}

					++c;
				}

			}
		}

		// parent...
		if ((item.Options & DBT_CIFO_OSC_AP) && (item.Handle != m_RootContact))
		{
			newitem.Handle = getParent(item.Handle);
			if ((iter->returned->find(newitem.Handle) == iter->returned->end()) &&
				  (newitem.Handle != DBT_INVALIDPARAM) &&
				  m_BlockManager.ReadPart(newitem.Handle, &newitem.Flags, offsetof(TContact, Flags), sizeof(newitem.Flags), sig))
			{
				newitem.Level = item.Level - 1;
				newitem.LookupDepth = item.LookupDepth;
				newitem.Options = (iter->filter.Options / DBT_CIFO_OP_AC * DBT_CIFO_OSC_AC) & (DBT_CIFO_OSC_AC | DBT_CIFO_OSC_AP | DBT_CIFO_OSC_AO | DBT_CIFO_OSC_AOC | DBT_CIFO_OSC_AOP);

				if ((newitem.Flags & iter->filter.fDontHasFlags & DBT_CF_IsGroup) == 0) // if we don't want groups, stop it
				{
					iter->parents->push_back(newitem);
					iter->returned->insert(newitem.Handle);
				}
			}
		}

		// virtual lookup, original contact is the next one
		if ((item.Flags & DBT_CF_IsVirtual) && 
			  (item.Options & DBT_CIFO_OSC_AO) && 
				(((iter->filter.Options >> 28) >= item.LookupDepth) || ((iter->filter.Options >> 28) == 0)))
		{
			newitem.Handle = VirtualGetParent(item.Handle);

			if ((iter->returned->find(newitem.Handle) == iter->returned->end()) &&
				  (newitem.Handle != DBT_INVALIDPARAM) &&
				   m_BlockManager.ReadPart(newitem.Handle, &newitem.Flags, offsetof(TContact, Flags), sizeof(newitem.Flags), sig) &&
           m_BlockManager.ReadPart(newitem.Handle, &newitem.Level, offsetof(TContact, Level), sizeof(newitem.Level), sig))
			{
				newitem.Options  = 0;
				if ((item.Options & DBT_CIFO_OSC_AOC) == DBT_CIFO_OSC_AOC)
					newitem.Options |= DBT_CIFO_OSC_AC;
				if ((item.Options & DBT_CIFO_OSC_AOP) == DBT_CIFO_OSC_AOP)
					newitem.Options |= DBT_CIFO_OSC_AP;

				newitem.LookupDepth = item.LookupDepth + 1;

				iter->q->push_front(newitem);
				iter->returned->insert(newitem.Handle);
			}
		}

		if (((iter->filter.fHasFlags & item.Flags) == iter->filter.fHasFlags) &&
		    ((iter->filter.fDontHasFlags & item.Flags) == 0))
		{
			result = item.Handle;
		}

	} while ((result == 0) && !iter->q->empty());

	if (result == 0)
		result = IterationNext(Iteration);

	SYNC_ENDREAD(m_Sync);

	return result;
}
unsigned int CContacts::IterationClose(TDBTContactIterationHandle Iteration)
{
//	SYNC_BEGINREAD(m_Sync); // no sync needed

	PContactIteration iter = (PContactIteration) Iteration;

	delete iter->q;
	delete iter->parents;
	delete iter->returned;
	delete iter;

//	SYNC_ENDREAD(m_Sync);
	return 0;
}



TDBTContactHandle CContacts::VirtualCreate(TDBTContactHandle hRealContact, TDBTContactHandle hParent)
{
	uint32_t f;
	uint32_t sig = cContactSignature;

	SYNC_BEGINWRITE(m_Sync);
	
	if (!m_BlockManager.ReadPart(hRealContact, &f, offsetof(TContact, Flags), sizeof(f), sig) || 
		 (f & DBT_CF_IsGroup))
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	TDBTContactHandle result = CreateContact(hParent, DBT_CF_IsVirtual);
	if (result == DBT_INVALIDPARAM)
	{
		SYNC_ENDWRITE(m_Sync);
		return DBT_INVALIDPARAM;
	}

	if (f & DBT_CF_IsVirtual)
	{		
		m_BlockManager.ReadPart(hRealContact, &hRealContact, offsetof(TContact, VParent), sizeof(hRealContact), sig);		
		m_BlockManager.ReadPart(hRealContact, &f, offsetof(TContact, Flags), sizeof(f), sig);
	}

	m_BlockManager.WritePart(result, &hRealContact, offsetof(TContact, VParent), sizeof(hRealContact));

	if ((f & DBT_CF_HasVirtuals) == 0)
	{
		f |= DBT_CF_HasVirtuals;
		m_BlockManager.WritePart(hRealContact, &f, offsetof(TContact, Flags), sizeof(f));
	}

	m_Virtuals._InsertVirtual(hRealContact, result);

	SYNC_ENDWRITE(m_Sync);
	return result;
}

TDBTContactHandle CContacts::VirtualGetParent(TDBTContactHandle hVirtual)
{
	return m_Virtuals.getParent(hVirtual);
}
TDBTContactHandle CContacts::VirtualGetFirst(TDBTContactHandle hRealContact)
{
	return m_Virtuals.getFirst(hRealContact);
}
TDBTContactHandle CContacts::VirtualGetNext(TDBTContactHandle hVirtual)
{	
	return m_Virtuals.getNext(hVirtual);
}


TDBTContactHandle CContacts::compFirstContact()
{
	uint32_t sig = cContactSignature;

	SYNC_BEGINREAD(m_Sync);
	TContactKey key = {0};
	iterator i = LowerBound(key);
	TDBTContactHandle res = 0;

	while (i && (res == 0))
	{
		uint32_t f = 0;
		if (m_BlockManager.ReadPart(i->Contact, &f, offsetof(TContact, Flags), sizeof(f), sig))
		{
			if ((f & (DBT_CF_IsGroup | DBT_CF_IsVirtual)) == 0)
				res = i->Contact;
		}
		if (res == 0)
			++i;
	}
	SYNC_ENDREAD(m_Sync);

	return res;
}
TDBTContactHandle CContacts::compNextContact(TDBTContactHandle hContact)
{
	uint32_t sig = cContactSignature;

	SYNC_BEGINREAD(m_Sync);

	TContactKey key;
	key.Contact = hContact;
	TDBTContactHandle res = 0;

	if (m_BlockManager.ReadPart(hContact, &key.Level, offsetof(TContact, Level), sizeof(key.Level), sig) &&
			m_BlockManager.ReadPart(hContact, &key.Parent, offsetof(TContact, ParentContact), sizeof(key.Parent), sig))
	{
		key.Contact++;
		iterator i = LowerBound(key);
		
		while (i && (res == 0))
		{
			uint32_t f = 0;
			if (m_BlockManager.ReadPart(i->Contact, &f, offsetof(TContact, Flags), sizeof(f), sig))
			{
				if ((f & (DBT_CF_IsGroup | DBT_CF_IsVirtual)) == 0)
					res = i->Contact;
			}
			if (res == 0)
				++i;
		}
	}

	SYNC_ENDREAD(m_Sync);

	return res;
}
