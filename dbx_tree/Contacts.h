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
	TDBContactHandle RealContact;     /// hContact of the duplicated RealContact
	TDBContactHandle Virtual;       /// hContact of the virtual duplicate

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
	TDBContactHandle Parent;    /// hContact of the Parent. Root.Parent == 0
	TDBContactHandle Contact;     /// hContact of the stored Contact itself

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
	TDBContactHandle ParentContact; /// hContact of the Parent. Root.Parent == 0 !used in the BTreeKey!
	TDBContactHandle VParent;     /// if the Contact is Virtual this is the hContact of the related Realnode
	uint32_t Flags;         /// flags, see cEF_*
	/*CSettingsTree::TNodeRef*/
	uint32_t Settings;      /// Offset to the SettingsBTree RootNode of this contact, NULL if no settings are present !invalid for Virtual contact!
	/// TODO update type to events TNodeRef
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
class CVirtuals :	public CFileBTree<TVirtualKey, TEmpty, 4, true>
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
	TDBContactHandle _DeleteRealContact(TDBContactHandle hRealContact);
	
	bool _InsertVirtual(TDBContactHandle hRealContact, TDBContactHandle hVirtual);
	void _DeleteVirtual(TDBContactHandle hRealContact, TDBContactHandle hVirtual);

	// services:
	TDBContactHandle getParent(TDBContactHandle hVirtual);
	TDBContactHandle getFirst(TDBContactHandle hRealContact);
	TDBContactHandle getNext(TDBContactHandle hVirtual);
};


static const uint32_t cContactSignature = 0x9A6B3C0D;
static const uint16_t cContactNodeSignature = 0x65A9;
static const uint16_t cVirtualNodeSignature = 0x874E;
/**
	\brief Manages the ContactListContacts in the Database

	A hContact is equivalent to the fileoffset of its related TContact structure
**/
class CContacts : public CFileBTree<TContactKey, TEmpty, 6, true>
{
private:

protected:

	typedef struct TContactIterationItem {
		uint8_t Options;
		uint8_t LookupDepth;
		TDBContactHandle Handle;
		uint16_t Level;
		uint32_t Flags;
	} TContactIterationItem;

	typedef struct TContactIteration {
		TDBContactIterFilter filter;
		std::deque<TContactIterationItem> * q;
		std::deque<TContactIterationItem> * parents;
		stdext::hash_set<TDBContactHandle> * returned;
	} TContactIteration, *PContactIteration;

	TDBContactHandle m_RootContact;
	CMultiReadExclusiveWriteSynchronizer & m_Sync;
	CVirtuals m_Virtuals;

	unsigned int m_IterAllocSize;
	TContactIteration **m_Iterations;

	TDBContactHandle CreateRootContact();

public:
	CContacts(CBlockManager & BlockManager, CMultiReadExclusiveWriteSynchronizer & Synchronize, TDBContactHandle RootContact, TNodeRef ContactRoot, CVirtuals::TNodeRef VirtualRoot);
	virtual ~CContacts();


	CVirtuals::TOnRootChanged & sigVirtualRootChanged();

	//internal helpers:
	/*CSettingsTree::TNodeRef*/
	uint32_t _getSettingsRoot(TDBContactHandle hContact);
	bool _setSettingsRoot(TDBContactHandle hContact, /*CSettingsTree::TNodeRef*/ uint32_t NewRoot);
	uint32_t _getEventsRoot(TDBContactHandle hContact);
	bool _setEventsRoot(TDBContactHandle hContact, /*CSettingsTree::TNodeRef*/ uint32_t NewRoot);


	//Services:
	TDBContactHandle getRootContact();
	TDBContactHandle getParent(TDBContactHandle hContact);
	TDBContactHandle setParent(TDBContactHandle hContact, TDBContactHandle hParent);
	uint32_t getChildCount(TDBContactHandle hContact);
	TDBContactHandle getFirstChild(TDBContactHandle hParent);
	TDBContactHandle getLastChild(TDBContactHandle hParent);
	TDBContactHandle getNextSilbing(TDBContactHandle hContact);
	TDBContactHandle getPrevSilbing(TDBContactHandle hContact);	
	uint32_t getFlags(TDBContactHandle hContact);

	TDBContactHandle CreateContact(TDBContactHandle hParent, uint32_t Flags);
	unsigned int DeleteContact(TDBContactHandle hContact);

	TDBContactIterationHandle IterationInit(const TDBContactIterFilter & Filter, TDBContactHandle hParent);
	TDBContactHandle IterationNext(TDBContactIterationHandle Iteration);
	unsigned int IterationClose(TDBContactIterationHandle Iteration);

	TDBContactHandle VirtualCreate(TDBContactHandle hRealContact, TDBContactHandle hParent);
	TDBContactHandle VirtualGetParent(TDBContactHandle hVirtual);
	TDBContactHandle VirtualGetFirst(TDBContactHandle hRealContact);
	TDBContactHandle VirtualGetNext(TDBContactHandle hVirtual);

};
