#pragma once

#include "Interface.h"
#include "BTree.h"
#include "FileBTree.h"
#include "BlockManager.h"
#include "IterationHeap.h"
#include "Contacts.h"
#include "Settings.h"
#include "Hash.h"
#include "EncryptionManager.h"
#include "sigslot.h"

#include <hash_map>
#include <hash_set>
#include <queue>
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
	TDBTEventHandle   Event;    /// handle to the event
	TDBTContactHandle Contact;  /// handle to the contact which includes this event

	bool operator <  (const TEventLinkKey & Other) const;
	//bool operator <= (const TEventKey & Other);
	bool operator == (const TEventLinkKey & Other) const;
	//bool operator >= (const TEventKey & Other);
	bool operator >  (const TEventLinkKey & Other) const;
} TEventLinkKey;

/**
	\brief The data of an Event

	A event's data is variable length. The data is a TDBTEvent-structure followed by varaible length data.
	- fixed data
	- blob data (mostly UTF8 message body)
**/
typedef struct TEvent {
	uint32_t Flags;				       /// Flags
	union {
		struct {
			uint32_t TimeStamp;          /// Timestamp of the event (seconds elapsed since 1.1.1970) used as key element
			uint32_t Index;              /// index counter to seperate events with the same timestamp
		};
		TEventKey Key;
	};
	uint32_t Type;               /// Eventtype
	union {
		TDBTContactHandle Contact;  /// hContact which owns this event
		uint32_t ReferenceCount;   /// Reference Count, if event was hardlinked
	};
	uint32_t DataLength;         /// Length of the stored data in bytes

	uint8_t Reserved[8];            /// reserved storage
} TEvent;

#pragma pack(pop)



static const uint32_t cEventSignature = 0x365A7E92;
static const uint16_t cEventNodeSignature = 0x195C;
static const uint16_t cEventLinkNodeSignature = 0xC16A;



/**
	\brief Manages the Events Index in the Database
**/
class CEventsTree : public CFileBTree<TEventKey, TDBTEventHandle, 16, true>
{
private:
	TDBTContactHandle m_Contact;

public:
	CEventsTree(CBlockManager & BlockManager, TNodeRef RootNode, TDBTContactHandle Contact);
	~CEventsTree();

	TDBTContactHandle getContact();
	void setContact(TDBTContactHandle NewContact);
};

/**
	\brief Manages the Virtual Events Index
	Sorry for duplicating code...
**/
class CVirtualEventsTree : public CBTree<TEventKey, TDBTEventHandle, 16, true>
{
private:
	TDBTContactHandle m_Contact;

public:
	CVirtualEventsTree(TDBTContactHandle Contact);
	~CVirtualEventsTree();

	TDBTContactHandle getContact();
	void setContact(TDBTContactHandle NewContact);
};


class CEventsTypeManager 
{
public:
	CEventsTypeManager(CContacts & Contacts, CSettings & Settings);
	~CEventsTypeManager();

	uint32_t MakeGlobalID(char* Module, uint32_t EventType);
	bool GetType(uint32_t GlobalID, char * & Module, uint32_t & EventType);
	uint32_t EnsureIDExists(char* Module, uint32_t EventType);

private:
	typedef struct TEventType {
		char *   ModuleName;
		uint32_t EventType;
	} TEventType, *PEventType;
	typedef stdext::hash_map<uint32_t, PEventType> TTypeMap;

	CContacts & m_Contacts;
	CSettings & m_Settings;
	TTypeMap m_Map;

};


class CEventLinks : public CFileBTree<TEventLinkKey, TEmpty, 8, true>
{
public:
	CEventLinks(CBlockManager & BlockManager, TNodeRef RootNode);
	~CEventLinks();

private:


};

class CEvents : public sigslot::has_slots<>
{
public:

	CEvents(
		CBlockManager & BlockManager, 
		CEncryptionManager & EncryptionManager,
		CEventLinks::TNodeRef LinkRootNode, 
		CMultiReadExclusiveWriteSynchronizer & Synchronize,
		CContacts & Contacts, 
		CSettings & Settings,
		uint32_t IndexCounter
		);
	~CEvents();

	CEventLinks::TOnRootChanged & sigLinkRootChanged();
	
	typedef sigslot::signal2<CEvents *, uint32_t> TOnIndexCounterChanged;
	TOnIndexCounterChanged & _sigIndexCounterChanged();


	//compatibility
	TDBTEventHandle compFirstEvent(TDBTContactHandle hContact);
	TDBTEventHandle compFirstUnreadEvent(TDBTContactHandle hContact);
	TDBTEventHandle compLastEvent(TDBTContactHandle hContact);
	TDBTEventHandle compNextEvent(TDBTEventHandle hEvent);
	TDBTEventHandle compPrevEvent(TDBTEventHandle hEvent);

	//services
	unsigned int GetBlobSize(TDBTEventHandle hEvent);
	unsigned int Get(TDBTEventHandle hEvent, TDBTEvent & Event);
	unsigned int GetCount(TDBTContactHandle hContact);
	unsigned int Delete(TDBTContactHandle hContact, TDBTEventHandle hEvent);
	TDBTEventHandle Add(TDBTContactHandle hContact, TDBTEvent & Event);
	unsigned int MarkRead(TDBTContactHandle hContact, TDBTEventHandle hEvent);
	unsigned int WriteToDisk(TDBTContactHandle hContact, TDBTEventHandle hEvent);
	unsigned int HardLink(TDBTEventHardLink & HardLink);

	TDBTContactHandle GetContact(TDBTEventHandle hEvent);

	TDBTEventIterationHandle IterationInit(TDBTEventIterFilter & Filter);
	TDBTEventHandle IterationNext(TDBTEventIterationHandle Iteration);
	unsigned int IterationClose(TDBTEventIterationHandle Iteration);


private:
	typedef CBTree<TEventKey, TDBTEventHandle, 16, true> TEventBase;
	typedef stdext::hash_map<TDBTContactHandle, CEventsTree*> TEventsTreeMap;
	typedef stdext::hash_map<TDBTContactHandle, CVirtualEventsTree*> TVirtualEventsTreeMap;
	typedef stdext::hash_map<TDBTContactHandle, uint32_t> TVirtualEventsCountMap;
	typedef CIterationHeap<TEventBase::iterator> TEventsHeap;
	
	typedef stdext::hash_set<TDBTContactHandle> TVirtualOwnerSet;
	typedef stdext::hash_map<TDBTEventHandle, TVirtualOwnerSet*> TVirtualOwnerMap;

	CMultiReadExclusiveWriteSynchronizer & m_Sync;
	CBlockManager & m_BlockManager;
	CEncryptionManager & m_EncryptionManager;

	CContacts & m_Contacts;
	CEventsTypeManager m_Types;
	CEventLinks m_Links;

	TEventsTreeMap m_EventsMap;
	TVirtualEventsTreeMap m_VirtualEventsMap;
	TVirtualOwnerMap m_VirtualOwnerMap;
	TVirtualEventsCountMap m_VirtualCountMap;

	uint32_t m_Counter;
	TOnIndexCounterChanged m_sigIndexCounterChanged;

	typedef struct TEventIteration {
		TDBTEventIterFilter Filter;
		TEventsHeap * Heap;
		TDBTEventHandle LastEvent;
	} TEventIteration, *PEventIteration;

	void onRootChanged(void* EventsTree, CEventsTree::TNodeRef NewRoot);

	void onDeleteEventCallback(void * Tree, TEventKey Key, TDBTEventHandle Data, uint32_t Param);
	void onDeleteVirtualEventCallback(void * Tree, TEventKey Key, TDBTEventHandle Data, uint32_t Param);
	void onDeleteEvents(CContacts * Contacts, TDBTContactHandle hContact);
	void onTransferEvents(CContacts * Contacts, TDBTContactHandle Source, TDBTContactHandle Dest);

	CEventsTree * getEventsTree(TDBTContactHandle hContact);
	CVirtualEventsTree * getVirtualEventsTree(TDBTContactHandle hContact);
	uint32_t adjustVirtualEventCount(TDBTContactHandle hContact, int32_t Adjust);
};
