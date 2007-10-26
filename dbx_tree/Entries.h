#pragma once
#include "Interface.h"
#include "FileBTree.h"
#include "MREWSync.h"

#pragma pack(push)  // push current alignment to stack
#pragma pack(1)     // set alignment to 1 byte boundary

/**
	\brief Key Type of the EntryBTree

	The entries are sorted first based on their level. (root is first node, followed by its children)
	That is for enumeration of one Entry's children, which are all stored in one block in the BTree
**/
typedef struct TEntryKey { 
	unsigned short Level;   /// Level where Entry is located or parent-steps to root. Root.Level == 0, root children have level 1 etc.
	TEntryHandle Parent;    /// hEntry of the Parent. Root.Parent == 0
	TEntryHandle Entry;     /// hEntry of the stored entry itself

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
	unsigned int Signature;   /// Signature 
	unsigned short Level;     /// Level where Entry is located or parent-steps to root. Root.Level == 0, root children have level 1 etc. !used in the BTreeKey!
	TEntryHandle ParentEntry; /// hEntry of the Parent. Root.Parent == 0 !used in the BTreeKey!
	TEntryHandle VParent;     /// if the Entry is Virtual this is the hEntry of the related Realnode
	unsigned int Flags;       /// flags, see cEF_*
	unsigned int Settings;    /// Offset to the SettingsBTree RootNode of this contact, NULL if no settings are present !invalid for Virtual contact!
	unsigned int Events;      /// Offset to the EventsBTree RootNode of this contact, NULL if no events are present !invalid for Virtal contact!
	unsigned int ChildCount;  /// Count of the children !invalid for Virtal contact!
	unsigned int EventCount;  /// Count of the stored events !invalid for Virtal contact!
	char Reserved[8];         /// reserved storage
} TEntry;

#pragma pack(pop)		// pop the alignment from stack

/**
	\brief Manages the ContactListEntries in the Database

	A hEntry is equivalent to the fileoffset of its related TEntry structure
**/

static const unsigned int cEntrySignature = 0x9A6B3C0D;

class CEntries : public CFileBTree<TEntryKey, TEmpty, 6, true>
{
private:

protected:
	CMultiReadExclusiveWriteSynchronizer & m_Sync;

public:
	CEntries(CFileAccess & FileAccess, CMultiReadExclusiveWriteSynchronizer & Synchronize, unsigned int RootNode);
	virtual ~CEntries();

	TEntryHandle getParent(TEntryHandle hEntry);
	TEntryHandle setParent(TEntryHandle hEntry, TEntryHandle hParent);
	unsigned int getChildCount(TEntryHandle hEntry);
	TEntryHandle getFirstChild(TEntryHandle hParent);
	TEntryHandle getLastChild(TEntryHandle hParent);
	TEntryHandle getNextSilbing(TEntryHandle hEntry);
	TEntryHandle getPrevSilbing(TEntryHandle hEntry);	

	TEntryHandle CreateEntry(TEntryHandle hParent, unsigned int Flags);
	unsigned int DeleteEntry(TEntryHandle hEntry);


};
