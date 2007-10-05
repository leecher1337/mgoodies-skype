#pragma once
#include "FileBTree.h"

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct TEntryKey {
	unsigned short Level;
	unsigned int Parent;
	unsigned int Entry;

	bool operator <  (const TEntryKey & Other);
	//bool operator <= (const TEntryKey & Other);
	bool operator == (const TEntryKey & Other);
	//bool operator >= (const TEntryKey & Other);
	//bool operator >  (const TEntryKey & Other);
} TEntryKey;

const unsigned int cEF_IsGroup   = 0x00000001;
const unsigned int cEF_HasChilds = 0x00000002;
const unsigned int cEF_IsVirtual = 0x00000004;

typedef struct TEntry {
	unsigned int Signature;
	unsigned short Level;
	unsigned int ParentEntry;
	unsigned int VParent;
	unsigned int Flags;
	unsigned int Settings;
	unsigned int Events;
	unsigned int EventCount;
	char Reserved[8];
} TEntry;

#pragma pack(pop)


class CEntries :	private CFileBTree<TEntryKey, TEmpty, 4, true>
{
private:

protected:
	virtual void RootChanged();

public:
	CEntries(CFileAccess & FileAccess);
	~CEntries();
};
