#pragma once
#include "Interface.h"
#include "FileBTree.h"
#include "MREWSync.h"
#include <deque>

#pragma pack(push)  // push current alignment to stack
#pragma pack(1)     // set alignment to 1 byte boundary


/**
	\brief Key Type of the VirtualsBTree

	This BTree don't hold data itself, it's just for organisation
	The virtual entries are sorted first based on their real entry.
	That is for enumeration of one Entry's virtual copies, which are all stored in one block in the BTree
**/
typedef struct TVirtualKey {
	unsigned int RealEntry;     /// hEntry of the duplicated RealEntry
	unsigned int Virtual;       /// hEntry of the virtual duplicate

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
	unsigned short Level;   /// Level where Entry is located or parent-steps to root. Root.Level == 0, root children have level 1 etc.
	TDBEntryHandle Parent;    /// hEntry of the Parent. Root.Parent == 0
	TDBEntryHandle Entry;     /// hEntry of the stored entry itself

	bool operator <  (const TEntryKey & Other) const;
	//bool operator <= (const TEntryKey & Other);
	bool operator == (const TEntryKey & Other) const;
	//bool operator >= (const TEntryKey & Other);
	bool operator >  (const TEntryKey & Other) const;
} TEntryKey;


static const unsigned int cEntrySignature = 0x9A6B3C0D;

/**
	\brief The data of an Entry
**/
typedef struct TEntry {
	unsigned int Signature;     /// Signature 
	unsigned short Level;       /// Level where Entry is located or parent-steps to root. Root.Level == 0, root children have level 1 etc. !used in the BTreeKey!
	TDBEntryHandle ParentEntry; /// hEntry of the Parent. Root.Parent == 0 !used in the BTreeKey!
	TDBEntryHandle VParent;     /// if the Entry is Virtual this is the hEntry of the related Realnode
	unsigned int Flags;         /// flags, see cEF_*
	unsigned int Settings;      /// Offset to the SettingsBTree RootNode of this contact, NULL if no settings are present !invalid for Virtual contact!
	unsigned int Events;        /// Offset to the EventsBTree RootNode of this contact, NULL if no events are present !invalid for Virtal contact!
	unsigned int ChildCount;    /// Count of the children !invalid for Virtal contact!
	unsigned int EventCount;    /// Count of the stored events !invalid for Virtal contact!
	char Reserved[8];           /// reserved storage
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
	CVirtuals(CFileAccess & FileAccess, CMultiReadExclusiveWriteSynchronizer & Synchronize, unsigned int Root);
	virtual ~CVirtuals();

	/**
		\brief Changes reference for all copies to the first Virtual in list

		\return New Original (previously first Virtual) to associate data with
	**/
	TDBEntryHandle _DeleteRealEntry(TDBEntryHandle hRealEntry);
	
	unsigned int _InsertVirtual(TDBEntryHandle hRealEntry, TDBEntryHandle hVirtual);
	void _DeleteVirtual(TDBEntryHandle hRealEntry, TDBEntryHandle hVirtual);

	// services:
	TDBEntryHandle getParent(TDBEntryHandle hVirtual);
	TDBEntryHandle getFirst(TDBEntryHandle hRealEntry);
	TDBEntryHandle getNext(TDBEntryHandle hVirtual);
};

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

public:
	CEntries(CFileAccess & FileAccess, CMultiReadExclusiveWriteSynchronizer & Synchronize, TDBEntryHandle RootEntry, unsigned int EntryRoot, unsigned int VirtualRoot);
	virtual ~CEntries();


	CVirtuals::TOnRootChanged & sigVirtualRootChanged();

	//internal helpers:
	unsigned int _getSettingsRoot(TDBEntryHandle hEntry);
	unsigned int _setSettingsRoot(TDBEntryHandle hEntry, unsigned int NewRoot);

	//Services:
	TDBEntryHandle getRootEntry();
	TDBEntryHandle getParent(TDBEntryHandle hEntry);
	TDBEntryHandle setParent(TDBEntryHandle hEntry, TDBEntryHandle hParent);
	unsigned int getChildCount(TDBEntryHandle hEntry);
	TDBEntryHandle getFirstChild(TDBEntryHandle hParent);
	TDBEntryHandle getLastChild(TDBEntryHandle hParent);
	TDBEntryHandle getNextSilbing(TDBEntryHandle hEntry);
	TDBEntryHandle getPrevSilbing(TDBEntryHandle hEntry);	
	unsigned int getFlags(TDBEntryHandle hEntry);

	TDBEntryHandle CreateEntry(TDBEntryHandle hParent, unsigned int Flags);
	unsigned int DeleteEntry(TDBEntryHandle hEntry);

	TDBEntryIterationHandle IterationInit(const TDBEntryIterFilter & Filter);
	TDBEntryHandle IterationNext(TDBEntryIterationHandle Iteration);
	unsigned int IterationClose(TDBEntryIterationHandle Iteration);

	TDBEntryHandle VirtualCreate(TDBEntryHandle hRealEntry, TDBEntryHandle hParent);
	TDBEntryHandle VirtualGetParent(TDBEntryHandle hVirtual);
	TDBEntryHandle VirtualGetFirst(TDBEntryHandle hRealEntry);
	TDBEntryHandle VirtualGetNext(TDBEntryHandle hVirtual);

};
