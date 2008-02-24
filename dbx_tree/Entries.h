#pragma once

#include "Interface.h"
#include "FileBTree.h"
#include "MREWSync.h"
#include <deque>

#pragma pack(push, 1)  // push current alignment to stack, set alignment to 1 byte boundary


/**
	\brief Key Type of the VirtualsBTree

	This BTree don't hold data itself, it's just for organisation
	The virtual entries are sorted first based on their real entry.
	That is for enumeration of one Entry's virtual copies, which are all stored in one block in the BTree
**/
typedef struct TVirtualKey {
	TDBEntryHandle RealEntry;     /// hEntry of the duplicated RealEntry
	TDBEntryHandle Virtual;       /// hEntry of the virtual duplicate

	bool operator <  (const TVirtualKey & Other) const;
	//bool operator <= (const TVirtualKey & Other);
	bool operator == (const TVirtualKey & Other) const;
	//bool operator >= (const TVirtualKey & Other);
	bool operator >  (const TVirtualKey & Other) const;
} TVirtualKey;



/**
	\brief Key Type of the EntryBTree

	The entries are sorted first based on their level. (root is first node, followed by its children)
	That is for enumeration of one Entry's children, which are all stored in one block in the BTree
**/
typedef struct TEntryKey { 
	uint16_t Level;   /// Level where Entry is located or parent-steps to root. Root.Level == 0, root children have level 1 etc.
	TDBEntryHandle Parent;    /// hEntry of the Parent. Root.Parent == 0
	TDBEntryHandle Entry;     /// hEntry of the stored entry itself

	bool operator <  (const TEntryKey & Other) const;
	//bool operator <= (const TEntryKey & Other);
	bool operator == (const TEntryKey & Other) const;
	//bool operator >= (const TEntryKey & Other);
	bool operator >  (const TEntryKey & Other) const;
} TEntryKey;


/**
	\brief The data of an Entry
**/
typedef struct TEntry {
	uint16_t Level;       /// Level where Entry is located or parent-steps to root. Root.Level == 0, root children have level 1 etc. !used in the BTreeKey!
	uint16_t ChildCount;    /// Count of the children !invalid for Virtal contact!
	TDBEntryHandle ParentEntry; /// hEntry of the Parent. Root.Parent == 0 !used in the BTreeKey!
	TDBEntryHandle VParent;     /// if the Entry is Virtual this is the hEntry of the related Realnode
	uint32_t Flags;         /// flags, see cEF_*
	/*CSettingsTree::TNodeRef*/
	uint32_t Settings;      /// Offset to the SettingsBTree RootNode of this contact, NULL if no settings are present !invalid for Virtual contact!
	/// TODO update type to events TNodeRef
	uint32_t Events;        /// Offset to the EventsBTree RootNode of this contact, NULL if no events are present !invalid for Virtal contact!
	uint32_t EventCount;    /// Count of the stored events !invalid for Virtal contact!
	uint8_t Reserved[8];           /// reserved storage
} TEntry;

#pragma pack(pop)		// pop the alignment from stack




/**
	\brief Manages the Virtual ContactListEntries in the Database

	A virtual entry is stored as normal entry in the database-structure, but doesn't hold own settings/events.
	Such an entry has the virtual flag set and refers its original duplicate.
	All copies are stored in this BTree sorted to the realentry.
	If the RealEntry should be deleted take the first virtual duplicate and make it real. Also change the relation of other copies.
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
	TDBEntryHandle _DeleteRealEntry(TDBEntryHandle hRealEntry);
	
	bool _InsertVirtual(TDBEntryHandle hRealEntry, TDBEntryHandle hVirtual);
	void _DeleteVirtual(TDBEntryHandle hRealEntry, TDBEntryHandle hVirtual);

	// services:
	TDBEntryHandle getParent(TDBEntryHandle hVirtual);
	TDBEntryHandle getFirst(TDBEntryHandle hRealEntry);
	TDBEntryHandle getNext(TDBEntryHandle hVirtual);
};


static const uint32_t cEntrySignature = 0x9A6B3C0D;
static const uint16_t cEntryNodeSignature = 0x65A9;
static const uint16_t cVirtualNodeSignature = 0x874E;
/**
	\brief Manages the ContactListEntries in the Database

	A hEntry is equivalent to the fileoffset of its related TEntry structure
**/
class CEntries : public CFileBTree<TEntryKey, TEmpty, 6, true>
{
private:

protected:
	typedef struct TEntryIteration {
		TDBEntryIterFilter filter;
		std::deque<iterator *> * q;
	} TEntryIteration, *PEntryIteration;

	TDBEntryHandle m_RootEntry;
	CMultiReadExclusiveWriteSynchronizer & m_Sync;
	CVirtuals m_Virtuals;

	unsigned int m_IterAllocSize;
	TEntryIteration **m_Iterations;

	TDBEntryHandle CreateRootEntry();

public:
	CEntries(CBlockManager & BlockManager, CMultiReadExclusiveWriteSynchronizer & Synchronize, TDBEntryHandle RootEntry, TNodeRef EntryRoot, CVirtuals::TNodeRef VirtualRoot);
	virtual ~CEntries();


	CVirtuals::TOnRootChanged & sigVirtualRootChanged();

	//internal helpers:
	/*CSettingsTree::TNodeRef*/
	uint32_t _getSettingsRoot(TDBEntryHandle hEntry);
	bool _setSettingsRoot(TDBEntryHandle hEntry, /*CSettingsTree::TNodeRef*/ uint32_t NewRoot);

	//Services:
	TDBEntryHandle getRootEntry();
	TDBEntryHandle getParent(TDBEntryHandle hEntry);
	TDBEntryHandle setParent(TDBEntryHandle hEntry, TDBEntryHandle hParent);
	uint32_t getChildCount(TDBEntryHandle hEntry);
	TDBEntryHandle getFirstChild(TDBEntryHandle hParent);
	TDBEntryHandle getLastChild(TDBEntryHandle hParent);
	TDBEntryHandle getNextSilbing(TDBEntryHandle hEntry);
	TDBEntryHandle getPrevSilbing(TDBEntryHandle hEntry);	
	uint32_t getFlags(TDBEntryHandle hEntry);

	TDBEntryHandle CreateEntry(TDBEntryHandle hParent, uint32_t Flags);
	unsigned int DeleteEntry(TDBEntryHandle hEntry);

	TDBEntryIterationHandle IterationInit(const TDBEntryIterFilter & Filter);
	TDBEntryHandle IterationNext(TDBEntryIterationHandle Iteration);
	unsigned int IterationClose(TDBEntryIterationHandle Iteration);

	TDBEntryHandle VirtualCreate(TDBEntryHandle hRealEntry, TDBEntryHandle hParent);
	TDBEntryHandle VirtualGetParent(TDBEntryHandle hVirtual);
	TDBEntryHandle VirtualGetFirst(TDBEntryHandle hRealEntry);
	TDBEntryHandle VirtualGetNext(TDBEntryHandle hVirtual);

};
