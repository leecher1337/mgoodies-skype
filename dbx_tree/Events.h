#pragma once
#include "FileBTree.h"
#include "MREWSync.h"
#include <time.h>


#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

/**
	\brief Key Type of the EventsBTree
	
	The Key consists of a 64bit timestamp, seconds elapsed since 1.1.1970
	and an Index releated to the global counter in the header, which makes it possible to store multiple events with the same timestamp
**/
typedef struct TEventKey {
	__time64_t TimeStamp;   /// timestamp at which the event occoured
	unsigned int Index;     /// index counted globally in the databaseheader

	bool operator <  (const TEventKey & Other) const;
	//bool operator <= (const TEventKey & Other);
	bool operator == (const TEventKey & Other) const;
	//bool operator >= (const TEventKey & Other);
	bool operator >  (const TEventKey & Other) const;
} TEventKey;


/**
	\brief The data of an Event

	A event's data is variable length. The data is a TEvent-structure followed by varaible length data.
	- fixed data
	- module name
	- blob data (mostly UTF8 message body)
**/
typedef struct TEvent {
	unsigned int Signature;    /// Signature
	unsigned short Flags;      /// Flags
	unsigned short Type;       /// Eventtype
	__time64_t TimeStamp;      /// Timestamp of the event (seconds elapsed since 1.1.1970) used as key element
	unsigned int Index;        /// index counted globally in the databaseheader
	unsigned int Entry;        /// hEntry which owns this event
	unsigned int ModuleLen;    /// Length of the Module name
	unsigned int DataLen;      /// Length of the stored data in bytes
} TEvent;

#pragma pack(pop)

/**
	\brief Manages the Events in the Database
**/
class CEvents :	public CFileBTree<TEventKey, unsigned int, 16, true>
{
private:

protected:
	 CMultiReadExclusiveWriteSynchronizer & m_Sync;
public:
	CEvents(CFileAccess & FileAccess, CMultiReadExclusiveWriteSynchronizer & Synchronize, unsigned int RootNode);
	virtual ~CEvents();
};
