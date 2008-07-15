#pragma once

#include "Interface.h"
#include "FileBTree.h"
#include "MREWSync.h"
#include <deque>
#include <hash_set>

#pragma pack(push, 1)  // push current alignment to stack, set alignment to 1 byte boundary


/**
	\brief Key Type of the VirtualsBTree

	This BTree don't hold data itself, it's just for organisation
	The virtual Contacts are sorted first based on their real Contact.
	That is for enumeration of one Contact's virtual copies, which are all stored in one block in the BTree
**/
typedef struct TVirtualKey {
	TDBTContactHandle RealContact;     /// hContact of the duplicated RealContact
	TDBTContactHandle Virtual;       /// hContact of the virtual duplicate

	bool operator <  (const TVirtualKey & Other) const;
	//bool operator <= (const TVirtualKey & Other);
	bool operator == (const TVirtualKey & Other) const;
	//bool operator >= (const TVirtualKey & Other);
	bool operator >  (const TVirtualKey & Other) const;
} TVirtualKey;



/**
	\brief Key Type of the ContactBTree

	The Contacts are sorted first based on their level. (root is first node, followed by its children)
	That is for enumeration of one Contact's children, which are all stored in one block in the BTree
**/
typedef struct TContactKey { 
	uint16_t Level;   /// Level where Contact is located or parent-steps to root. Root.Level == 0, root children have level 1 etc.
	TDBTContactHandle Parent;    /// hContact of the Parent. Root.Parent == 0
	TDBTContactHandle Contact;     /// hContact of the stored Contact itself

	bool operator <  (const TContactKey & Other) const;
	//bool operator <= (const TContactKey & Other);
	bool operator == (const TContactKey & Other) const;
	//bool operator >= (const TContactKey & Other);
	bool operator >  (const TContactKey & Other) const;
} TContactKey;


/**
	\brief The data of an Contact
**/
typedef struct TContact {
	uint16_t Level;       /// Level where Contact is located or parent-steps to root. Root.Level == 0, root children have level 1 etc. !used in the BTreeKey!
	uint16_t ChildCount;    /// Count of the children !invalid for Virtal contact!
	TDBTContactHandle ParentContact; /// hContact of the Parent. Root.Parent == 0 !used in the BTreeKey!
	TDBTContactHandle VParent;     /// if the Contact is Virtual this is the hContact of the related Realnode
	uint32_t Flags;         /// flags, see cEF_*
	/*CSettingsTree::TNodeRef*/
	uint32_t Settings;      /// Offset to the SettingsBTree RootNode of this contact, NULL if no settings are present
	/*CEventsTree::TNodeRef*/
	uint32_t Events;        /// Offset to the EventsBTree RootNode of this contact, NULL if no events are present !invalid for Virtal contact!
	uint32_t EventCount;    /// Count of the stored events !invalid for Virtual contact!
	uint8_t Reserved[8];           /// reserved storage
} TContact;

#pragma pack(pop)		// pop the alignment from stack




/**
	\brief Manages the Virtual ContactListContacts in the Database

	A virtual Contact is stored as normal Contact in the database-structure, but doesn't hold own settings/events.
	Such an Contact has the virtual flag set and refers its original duplicate.
	All copies are stored in this BTree sorted to the realContact.
	If the RealContact should be deleted take the first virtual duplicate and make it real. Also change the relation of other copies.
**/
class CVirtuals :	public CFileBTree<TVirtualKey, 4>
{
private:

protected:
	 CMultiReadExclusiveWriteSynchronizer & m_Sync;
public:
	CVirtuals(CBlockManager & BlockManager, CMultiReadExclusiveWriteSynchronizer & Synchronize, TNodeRef Root);
	virtual ~CVirtuals();

	/**
		\brief Changes reference for all copies to the first Virtual in list

		\return New Original (previously first Virtual) to associate data with
	**/
	TDBTContactHandle _DeleteRealContact(TDBTContactHandle hRealContact);
	
	bool _InsertVirtual(TDBTContactHandle hRealContact, TDBTContactHandle hVirtual);
	void _DeleteVirtual(TDBTContactHandle hRealContact, TDBTContactHandle hVirtual);

	// services:
	TDBTContactHandle getParent(TDBTContactHandle hVirtual);
	TDBTContactHandle getFirst(TDBTContactHandle hRealContact);
	TDBTContactHandle getNext(TDBTContactHandle hVirtual);
};


static const uint32_t cContactSignature = 0x9A6B3C0D;
static const uint16_t cContactNodeSignature = 0x65A9;
static const uint16_t cVirtualNodeSignature = 0x874E;
/**
	\brief Manages the ContactListContacts in the Database

	A hContact is equivalent to the fileoffset of its related TContact structure
**/
class CContacts : public CFileBTree<TContactKey, 6>
{

public:
	CContacts(CBlockManager & BlockManager, CMultiReadExclusiveWriteSynchronizer & Synchronize, TDBTContactHandle RootContact, TNodeRef ContactRoot, CVirtuals::TNodeRef VirtualRoot);
	virtual ~CContacts();

	typedef sigslot::signal2<CContacts *, TDBTContactHandle> TOnContactDelete;
	typedef sigslot::signal2<CContacts *, TDBTContactHandle> TOnInternalDeleteSettings;
	typedef sigslot::signal2<CContacts *, TDBTContactHandle> TOnInternalDeleteEvents;

	typedef sigslot::signal3<CContacts *, TDBTContactHandle, TDBTContactHandle> TOnInternalMergeSettings;
	typedef sigslot::signal3<CContacts *, TDBTContactHandle, TDBTContactHandle> TOnInternalTransferEvents;

	TOnContactDelete &           sigContactDelete();
	TOnInternalDeleteEvents &   _sigDeleteEvents();
	TOnInternalDeleteSettings & _sigDeleteSettings();
	TOnInternalMergeSettings &  _sigMergeSettings();
	TOnInternalTransferEvents & _sigTransferEvents();

	CVirtuals::TOnRootChanged & sigVirtualRootChanged();

	//internal helpers:
	/*CSettingsTree::TNodeRef*/
	uint32_t _getSettingsRoot(TDBTContactHandle hContact);
	bool _setSettingsRoot(TDBTContactHandle hContact, /*CSettingsTree::TNodeRef*/ uint32_t NewRoot);
	uint32_t _getEventsRoot(TDBTContactHandle hContact);
	bool _setEventsRoot(TDBTContactHandle hContact, /*CSettingsTree::TNodeRef*/ uint32_t NewRoot);
	uint32_t _getEventCount(TDBTContactHandle hContact);
	uint32_t _adjustEventCount(TDBTContactHandle hContact, int32_t Adjust);

	CVirtuals & _getVirtuals();

	//compatibility:
	TDBTContactHandle compFirstContact();
	TDBTContactHandle compNextContact(TDBTContactHandle hContact);
	//Services:
	TDBTContactHandle getRootContact();
	TDBTContactHandle getParent(TDBTContactHandle hContact);
	TDBTContactHandle setParent(TDBTContactHandle hContact, TDBTContactHandle hParent);
	uint32_t getChildCount(TDBTContactHandle hContact);
	TDBTContactHandle getFirstChild(TDBTContactHandle hParent);
	TDBTContactHandle getLastChild(TDBTContactHandle hParent);
	TDBTContactHandle getNextSilbing(TDBTContactHandle hContact);
	TDBTContactHandle getPrevSilbing(TDBTContactHandle hContact);	
	uint32_t getFlags(TDBTContactHandle hContact);

	TDBTContactHandle CreateContact(TDBTContactHandle hParent, uint32_t Flags);
	unsigned int DeleteContact(TDBTContactHandle hContact);

	TDBTContactIterationHandle IterationInit(const TDBTContactIterFilter & Filter, TDBTContactHandle hParent);
	TDBTContactHandle IterationNext(TDBTContactIterationHandle Iteration);
	unsigned int IterationClose(TDBTContactIterationHandle Iteration);

	TDBTContactHandle VirtualCreate(TDBTContactHandle hRealContact, TDBTContactHandle hParent);
	TDBTContactHandle VirtualGetParent(TDBTContactHandle hVirtual);
	TDBTContactHandle VirtualGetFirst(TDBTContactHandle hRealContact);
	TDBTContactHandle VirtualGetNext(TDBTContactHandle hVirtual);

private:

protected:

	typedef struct TContactIterationItem {
		uint8_t Options;
		uint8_t LookupDepth;
		uint16_t Level;
		TDBTContactHandle Handle;
		uint32_t Flags;
	} TContactIterationItem;

	typedef struct TContactIteration {
		TDBTContactIterFilter filter;
		std::deque<TContactIterationItem> * q;
		std::deque<TContactIterationItem> * parents;
		stdext::hash_set<TDBTContactHandle> * returned;
	} TContactIteration, *PContactIteration;

	TDBTContactHandle m_RootContact;
	CMultiReadExclusiveWriteSynchronizer & m_Sync;
	CVirtuals m_Virtuals;

	TDBTContactHandle CreateRootContact();

	TOnContactDelete           m_sigContactDelete;
	TOnInternalDeleteEvents    m_sigInternalDeleteEvents;
	TOnInternalDeleteSettings  m_sigInternalDeleteSettings;
	TOnInternalMergeSettings   m_sigInternalMergeSettings;
	TOnInternalTransferEvents  m_sigInternalTransferEvents;

};
