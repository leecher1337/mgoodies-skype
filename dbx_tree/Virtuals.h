#pragma once
#include "FileBTree.h"

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */


typedef struct TVirtualKey {
	unsigned int RealEntry;
	unsigned int Entry;

	bool operator <  (const TVirtualKey & Other);
	//bool operator <= (const TVirtualKey & Other);
	bool operator == (const TVirtualKey & Other);
	//bool operator >= (const TVirtualKey & Other);
	//bool operator >  (const TVirtualKey & Other);
} TVirtualKey;

#pragma pack(pop)

class CVirtuals :	private CFileBTree<TVirtualKey, TEmpty, 4, true>
{
private:

protected:
	virtual void RootChanged();

public:
	CVirtuals(CFileAccess & FileAccess);
	~CVirtuals();
};
