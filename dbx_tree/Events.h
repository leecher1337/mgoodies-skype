#pragma once

#include "Interface.h"
#include "BTree.h"
#include "FileBTree.h"
#include "BlockManager.h"
#include "IterationHeap.h"
#include "Contacts.h"
#include "Settings.h"
#include "Hash.h"

#include <hash_map>
#include <hash_set>
#include <time.h>
#include <windows.h>


#pragma pack(push, 1)  // push current alignment to stack, set alignment to 1 byte boundary

/**
	\brief Key Type of the EventsBTree
	
	The Key consists of a timestamp, seconds elapsed since 1.1.1970
	and an Index, which makes it possible to store multiple events with the same timestamp
**/
typedef struct TEventKey {
	uint32_t TimeStamp; /// timestamp at which the event occoured
	uint32_t Index;     /// index counted globally

	bool operator <  (const TEventKey & Other) const;
	//bool operator <= (const TEventKey & Other);
	bool operator == (const TEventKey & Other) const;
	//bool operator >= (const TEventKey & Other);
	bool operator >  (const TEventKey & Other) const;
} TEventKey;

/**
	\brief Key Type of the EventLinkBTree
**/
typedef struct TEventLinkKey {
	TDBEventHandle   Event;    /// handle to the event
	TDBContactHandle Contact;  /// handle to the contact which includes this event

	bool operator <  (const TEventLinkKey & Other) const;
	//bool operator <= (const TEventKey & Other);
	bool operator == (const TEventLinkKey & Other) const;
	//bool operator >= (const TEventKey & Other);
	bool operator >  (const TEventLinkKey & Other) const;
} TEventLinkKey;

/**
	\brief The data of an Event

	A event's data is variable length. The data is a TDBEvent-structure followed by varaible length data.
	- fixed data
	- blob data (mostly UTF8 message body)
**/
typedef struct TEvent {
	uint32_t Flags;				       /// Flags
	uint32_t TimeStamp;          /// Timestamp of the event (seconds elapsed since 1.1.1970) used as key element
	uint32_t Index;              /// index counter to seperate events with the same timestamp
	uint32_t Type;               /// Eventtype
	union {
		TDBContactHandle Contact;  /// hContact which owns this event
		uint32_t ReferenceCount;   /// Reference Count, if event was hardlinked
	};
	uint32_t DataLength;         /// Length of the stored data in bytes

	char Reserved[8];            /// reserved storage
} TEvent;

#pragma pack(pop)



static const uint32_t cEventSignature = 0x365A7E92;
static const uint16_t cEventNodeSignature = 0x195C;



/**
	\brief Manages the Events Index in the Database
**/
class CEventsTree : public CFileBTree<TEventKey, TDBEventHandle, 16, true>
{
private:
	TDBContactHandle m_Contact;

public:
	CEventsTree(CBlockManager & BlockManager, TNodeRef RootNode, TDBContactHandle Contact);
	~CEventsTree();

	TDBContactHandle getContact();
};

/**
	\brief Manages the Virtual Events Index
	Sorry for duplicating code...
**/
class CVirtualEventsTree : public CBTree<TEventKey, TDBEventHandle, 16, true>
{
private:
	TDBContactHandle m_Contact;

public:
	CVirtualEventsTree(TDBContactHandle Contact);
	~CVirtualEventsTree();

	TDBContactHandle getContact();
};


class CEventsTypeManager 
{
public:
	CEventsTypeManager(CContacts & Contacts, CSettings & Settings);
	~CEventsTypeManager();


	uint32_t MakeGlobalID(char* Module, uint32_t EventType);
	PDBEventTypeDescriptor GetDescriptor(uint32_t GlobalID);
	uint32_t EnsureIDExists(char* Module, uint32_t EventType, char* Description);

private:
	typedef stdext::hash_map<uint32_t, PDBEventTypeDescriptor> TTypeMap;

	CContacts & m_Contacts;
	CSettings & m_Settings;
	TTypeMap m_Map;

};


class CEventLinks : public CFileBTree<TEventLinkKey, TEmpty, 8, true>
{
public:
	CEventLinks(TNodeRef RootNode);
	~CEventLinks();

private:


};

class CEvents : public sigslot::has_slots<>
{
public:

	CEvents(CBlockManager & BlockManager, CEventLinks::TNodeRef LinkRootNode, CMultiReadExclusiveWriteSynchronizer & Synchronize, CContacts & Contacts, CSettings & Settings);
	~CEvents();

	CEventLinks::TOnRootChanged & sigLinkRootChanged();

	unsigned int TypeRegister(TDBEventTypeDescriptor & Type);
	PDBEventTypeDescriptor TypeGet(char * ModuleName, uint32_t EventType);

	unsigned int GetBlobSize(TDBEventHandle hEvent);
	unsigned int Get(TDBEventHandle hEvent, TDBEvent & Event);
	unsigned int Delete(TDBContactHandle hContact, TDBEventHandle hEvent);
	TDBEventHandle Add(TDBContactHandle hContact, TDBEvent & Event);
	unsigned int MarkRead(TDBContactHandle hContact, TDBEventHandle hEvent);
	unsigned int WriteToDisk(TDBContactHandle hContact, TDBEventHandle hEvent);
	unsigned int CreateHardLink(TDBEventHardLink & HardLink);

	TDBContactHandle GetContact(TDBEventHandle hEvent);

	TDBEventIterationHandle IterationInit(TDBEventIterFilter & Filter);
	TDBEventHandle IterationNext(TDBEventIterationHandle Iteration);
	unsigned int IterationClose(TDBEventIterationHandle Iteration);

private:
	typedef CBTree<TEventKey, TDBEventHandle, 16, true>::iterator TEventBaseIterator;
	typedef stdext::hash_map<TDBContactHandle, CEventsTree*> TEventsTreeMap;
	typedef stdext::hash_map<TDBContactHandle, CVirtualEventsTree*> TVirtualEventsTreeMap;
	typedef CIterationHeap<TEventBaseIterator> TEventsHeap;
	
	typedef stdext::hash_set<TDBContactHandle> TVirtualOwnerSet;
	typedef stdext::hash_map<TDBEventHandle, TVirtualOwnerSet*> TVirtualOwnerMap;

	CMultiReadExclusiveWriteSynchronizer & m_Sync;
	CBlockManager & m_BlockManager;

	CContacts & m_Contacts;
	CEventsTypeManager m_Types;
	CEventLinks m_Links;

	TEventsTreeMap m_EventsMap;
	TVirtualEventsTreeMap m_VirtualEventsMap;
	TVirtualOwnerMap m_VirtualOwnerMap;

	uint32_t m_Counter;

	typedef struct TEventIteration {
		TDBEventIterFilter Filter;
		TEventsHeap * Heap;
		TEventKey LastKey;
	} TEventIteration, *PEventIteration;

	unsigned int m_IterAllocSize;
	TEventIteration **m_Iterations;

	void onRootChanged(void* EventsTree, CEventsTree::TNodeRef NewRoot);

	CEventsTree * getEventsTree(TDBContactHandle hContact);
	CVirtualEventsTree * getVirtualEventsTree(TDBContactHandle hContact);
};
