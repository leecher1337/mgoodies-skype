#include "Virtuals.h"


__forceinline bool TVirtualKey::operator <  (const TVirtualKey & Other) const
{
	if (RealEntry != Other.RealEntry) return RealEntry < Other.RealEntry;
	if (Entry != Other.Entry) return Entry < Other.Entry;
	return false;
}

__forceinline bool TVirtualKey::operator == (const TVirtualKey & Other) const
{
	return (RealEntry == Other.RealEntry) && (Entry == Other.Entry);
}

__forceinline bool TVirtualKey::operator >  (const TVirtualKey & Other) const
{	
	if (RealEntry != Other.RealEntry) return RealEntry > Other.RealEntry;
	if (Entry != Other.Entry) return Entry > Other.Entry;
	return false;
}

CVirtuals::CVirtuals(CFileAccess & FileAccess, CMultiReadExclusiveWriteSynchronizer & Synchronize, unsigned int RootNode)
: CFileBTree(FileAccess, RootNode),
	m_Sync(Synchronize)
{

}

CVirtuals::~CVirtuals()
{

}

