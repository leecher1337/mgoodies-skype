#pragma once

#include "Interface.h"
#include "FileBTree.h"
#include "MREWSync.h"
#include <deque>
#include <hash_set>

#pragma pack(push, 1)  // push current alignment to stack, set alignment to 1 byte boundary


/**
	\brief Key Type of the VirtualsBTree

	This BTree don't hold data itself, it's just for organisation
	The virtual Entities are sorted first based on their real Entity.
	That is for enumeration of one Entity's virtual copies, which are all stored in one block in the BTree
**/
typedef struct TVirtualKey {
	TDBTEntityHandle RealEntity;     /// hEntity of the duplicated RealEntity
	TDBTEntityHandle Virtual;       /// hEntity of the virtual duplicate

	bool operator <  (const TVirtualKey & Other) const;
	//bool operator <= (const TVirtualKey & Other);
	bool operator == (const TVirtualKey & Other) const;
	//bool operator >= (const TVirtualKey & Other);
	bool operator >  (const TVirtualKey & Other) const;
} TVirtualKey;



/**
	\brief Key Type of the EntityBTree

	The Entities are sorted first based on their level. (root is first node, followed by its children)
	That is for enumeration of one Entity's children, which are all stored in one block in the BTree
**/
typedef struct TEntityKey { 
	uint16_t Level;   /// Level where Entity is located or parent-steps to root. Root.Level == 0, root children have level 1 etc.
	TDBTEntityHandle Parent;    /// hEntity of the Parent. Root.Parent == 0
	TDBTEntityHandle Entity;     /// hEntity of the stored Entity itself

	bool operator <  (const TEntityKey & Other) const;
	//bool operator <= (const TEntityKey & Other);
	bool operator == (const TEntityKey & Other) const;
	//bool operator >= (const TEntityKey & Other);
	bool operator >  (const TEntityKey & Other) const;
} TEntityKey;


/**
	\brief The data of an Entity
**/
typedef struct TEntity {
	uint16_t Level;       /// Level where Entity is located or parent-steps to root. Root.Level == 0, root children have level 1 etc. !used in the BTreeKey!
	uint16_t ChildCount;    /// Count of the children !invalid for Virtal Entity!
	TDBTEntityHandle ParentEntity; /// hEntity of the Parent. Root.Parent == 0 !used in the BTreeKey!
	union {
		TDBTEntityHandle VParent;     /// if the Entity is Virtual this is the hEntity of the related Realnode
		TDBTEntityHandle Account;     /// if the Entity's account, only for real real normal Entities
	};
	uint32_t Flags;         /// flags, see cEF_*
	/*CSettingsTree::TNodeRef*/
	uint32_t Settings;      /// Offset to the SettingsBTree RootNode of this Entity, NULL if no settings are present
	/*CEventsTree::TNodeRef*/
	uint32_t Events;        /// Offset to the EventsBTree RootNode of this Entity, NULL if no events are present !invalid for Virtal Entity!
	uint32_t EventCount;    /// Count of the stored events !invalid for Virtual Entity!
	uint8_t Reserved[8];           /// reserved storage
} TEntity;

#pragma pack(pop)		// pop the alignment from stack




/**
	\brief Manages the Virtual Entities in the Database

	A virtual Entity is stored as normal Entity in the database-structure, but doesn't hold own settings/events.
	Such an Entity has the virtual flag set and refers its original duplicate.
	All copies are stored in this BTree sorted to the RealEntity.
	If the RealEntity should be deleted take the first virtual duplicate and make it real. Also change the relation of other copies.
**/
class CVirtuals :	public CFileBTree<TVirtualKey, 4>
{
private:

protected:
	 CMultiReadExclusiveWriteSynchronizer & m_Sync;
public:
	CVirtuals(CBlockManager & BlockManager, CMultiReadExclusiveWriteSynchronizer & Synchronize, TNodeRef Root);
	virtual ~CVirtuals();

	/**
		\brief Changes reference for all copies to the first Virtual in list

		\return New Original (previously first Virtual) to associate data with
	**/
	TDBTEntityHandle _DeleteRealEntity(TDBTEntityHandle hRealEntity);
	
	bool _InsertVirtual(TDBTEntityHandle hRealEntity, TDBTEntityHandle hVirtual);
	void _DeleteVirtual(TDBTEntityHandle hRealEntity, TDBTEntityHandle hVirtual);

	// services:
	TDBTEntityHandle getParent(TDBTEntityHandle hVirtual);
	TDBTEntityHandle getFirst(TDBTEntityHandle hRealEntity);
	TDBTEntityHandle getNext(TDBTEntityHandle hVirtual);
};


static const uint32_t cEntitySignature = 0x9A6B3C0D;
static const uint16_t cEntityNodeSignature = 0x65A9;
static const uint16_t cVirtualNodeSignature = 0x874E;
/**
	\brief Manages the Entities in the Database

	A hEntity is equivalent to the fileoffset of its related TEntity structure
**/
class CEntities : public CFileBTree<TEntityKey, 6>
{

public:
	CEntities(CBlockManager & BlockManager, CMultiReadExclusiveWriteSynchronizer & Synchronize, TDBTEntityHandle RootEntity, TNodeRef EntityRoot, CVirtuals::TNodeRef VirtualRoot);
	virtual ~CEntities();

	typedef sigslot::signal2<CEntities *, TDBTEntityHandle> TOnEntityDelete;
	typedef sigslot::signal2<CEntities *, TDBTEntityHandle> TOnInternalDeleteSettings;
	typedef sigslot::signal2<CEntities *, TDBTEntityHandle> TOnInternalDeleteEvents;

	typedef sigslot::signal3<CEntities *, TDBTEntityHandle, TDBTEntityHandle> TOnInternalMergeSettings;
	typedef sigslot::signal3<CEntities *, TDBTEntityHandle, TDBTEntityHandle> TOnInternalTransferEvents;

	TOnEntityDelete &           sigEntityDelete();
	TOnInternalDeleteEvents &   _sigDeleteEvents();
	TOnInternalDeleteSettings & _sigDeleteSettings();
	TOnInternalMergeSettings &  _sigMergeSettings();
	TOnInternalTransferEvents & _sigTransferEvents();

	CVirtuals::TOnRootChanged & sigVirtualRootChanged();

	//internal helpers:
	/*CSettingsTree::TNodeRef*/
	uint32_t _getSettingsRoot(TDBTEntityHandle hEntity);
	bool _setSettingsRoot(TDBTEntityHandle hEntity, /*CSettingsTree::TNodeRef*/ uint32_t NewRoot);
	uint32_t _getEventsRoot(TDBTEntityHandle hEntity);
	bool _setEventsRoot(TDBTEntityHandle hEntity, /*CSettingsTree::TNodeRef*/ uint32_t NewRoot);
	uint32_t _getEventCount(TDBTEntityHandle hEntity);
	uint32_t _adjustEventCount(TDBTEntityHandle hEntity, int32_t Adjust);

	CVirtuals & _getVirtuals();

	//compatibility:
	TDBTEntityHandle compFirstContact();
	TDBTEntityHandle compNextContact(TDBTEntityHandle hEntity);
	//Services:
	TDBTEntityHandle getRootEntity();
	TDBTEntityHandle getParent(TDBTEntityHandle hEntity);
	TDBTEntityHandle setParent(TDBTEntityHandle hEntity, TDBTEntityHandle hParent);
	uint32_t getChildCount(TDBTEntityHandle hEntity);
	uint32_t getFlags(TDBTEntityHandle hEntity);
	uint32_t getAccount(TDBTEntityHandle hEntity);

	TDBTEntityHandle CreateEntity(const TDBTEntity & Entity);
	unsigned int DeleteEntity(TDBTEntityHandle hEntity);

	TDBTEntityIterationHandle IterationInit(const TDBTEntityIterFilter & Filter, TDBTEntityHandle hParent);
	TDBTEntityHandle IterationNext(TDBTEntityIterationHandle Iteration);
	unsigned int IterationClose(TDBTEntityIterationHandle Iteration);

	TDBTEntityHandle VirtualCreate(TDBTEntityHandle hRealEntity, TDBTEntityHandle hParent);
	TDBTEntityHandle VirtualGetParent(TDBTEntityHandle hVirtual);
	TDBTEntityHandle VirtualGetFirst(TDBTEntityHandle hRealEntity);
	TDBTEntityHandle VirtualGetNext(TDBTEntityHandle hVirtual);

private:

protected:

	typedef struct TEntityIterationItem {
		uint8_t Options;
		uint8_t LookupDepth;
		uint16_t Level;
		TDBTEntityHandle Handle;
		uint32_t Flags;
	} TEntityIterationItem;

	typedef struct TEntityIteration {
		TDBTEntityIterFilter filter;
		std::deque<TEntityIterationItem> * q;
		std::deque<TEntityIterationItem> * parents;
		std::deque<TEntityIterationItem> * accounts;
		stdext::hash_set<TDBTEntityHandle> * returned;
	} TEntityIteration, *PEntityIteration;

	TDBTEntityHandle m_RootEntity;
	CMultiReadExclusiveWriteSynchronizer & m_Sync;
	CVirtuals m_Virtuals;

	TDBTEntityHandle CreateRootEntity();

	TOnEntityDelete            m_sigEntityDelete;
	TOnInternalDeleteEvents    m_sigInternalDeleteEvents;
	TOnInternalDeleteSettings  m_sigInternalDeleteSettings;
	TOnInternalMergeSettings   m_sigInternalMergeSettings;
	TOnInternalTransferEvents  m_sigInternalTransferEvents;

};
