#pragma once
#include "FileBTree.h"

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

/**
	\brief Key Type of the VirtualsBTree

	This BTree don't hold data itself, it's just for organisation
	The virtual entries are sorted first based on their real entry.
	That is for enumeration of one Entry's virtual copies, which are all stored in one block in the BTree
**/
typedef struct TVirtualKey {
	unsigned int RealEntry;   /// hEntry of the duplicated RealEntry
	unsigned int Entry;       /// hEntry of the virtual copy

	bool operator <  (const TVirtualKey & Other);
	//bool operator <= (const TVirtualKey & Other);
	bool operator == (const TVirtualKey & Other);
	//bool operator >= (const TVirtualKey & Other);
	//bool operator >  (const TVirtualKey & Other);
} TVirtualKey;

#pragma pack(pop)

/**
	\brief Manages the Virtual ContactListEntries in the Database

	A virtual entry is stored as normal entry in the database-structure, but doesn't hold own settings/events.
	Such an entry has the virtual flag set and refers its original copy.
	All copies are stored in this BTree sorted to the realentry.
	If the RealEntry should be deleted take the first virtual copy and make it real. Also change the relation of other copies.
**/
class CVirtuals :	private CFileBTree<TVirtualKey, TEmpty, 4, true>
{
private:

protected:
	virtual void RootChanged();

public:
	CVirtuals(CFileAccess & FileAccess);
	virtual ~CVirtuals();
};
