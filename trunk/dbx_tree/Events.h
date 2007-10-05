#pragma once
#include "FileBTree.h"
#include <time.h>


#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct TEventKey {
	__time64_t TimeStamp;
	unsigned int Index;

	bool operator <  (const TEventKey & Other);
	//bool operator <= (const TEventKey & Other);
	bool operator == (const TEventKey & Other);
	//bool operator >= (const TEventKey & Other);
	//bool operator >  (const TEventKey & Other);
} TEventKey;



typedef struct TEvent {
	unsigned int Signature;
	unsigned short Flags;
	unsigned short Type;
	unsigned short ModuleLen;
	unsigned short SettingLen;
	union {
		unsigned int Data;
		unsigned int BlobLen;
	};
} TEvent;

#pragma pack(pop)

class CEvents :	private CFileBTree<TEventKey, unsigned int, 4, true>
{
private:

protected:
	virtual void RootChanged();

public:
	CEvents(CFileAccess & FileAccess);
	~CEvents();
};
