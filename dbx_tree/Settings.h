#pragma once

#include "Interface.h"
#include "FileBTree.h"
#include "MREWSync.h"
#include "sigslot.h"
#include "IterationHeap.h"

#include <map>
#include <queue>

class CSettings;
class CSettingsTree;

#include "Entries.h"

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

/**
	\brief Key Type of the SettingsBTree

	The setting names (UTF8) are hashed, so that they can easily be accessed.
**/

typedef struct TSettingKey {
	uint32_t Hash; /// 32 bit hash of the Setting name

	bool operator <  (const TSettingKey & Other) const;
	//bool operator <= (const TSettingKey & Other);
	bool operator == (const TSettingKey & Other) const;
	//bool operator >= (const TSettingKey & Other);
	bool operator >  (const TSettingKey & Other) const;
} TSettingKey;



static const uint32_t cSettingSignature = 0xF5B87A3D;
static const uint16_t cSettingNodeSignature = 0xBA12;

/**
	\brief The data of a setting

	A setting's data is variable length. The data is a TSetting-structure followed by varaible length data.
	- fixed data
	- SettingName (UTF8)
	- maybe blob data
**/
typedef struct TSetting {
	TDBEntryHandle Entry;			   /// Settings' entry
	uint32_t   Flags;        /// flags
	uint16_t   Type;         /// setting type	
	uint16_t   NameLength;   /// settingname length
	union {
		TDBSettingValue Value;     /// if type is fixed length, the data is stored rigth here
		
		struct {
			uint32_t BlobLength; /// if type is variable length this describes the length of the data in bytes
			uint32_t AllocSize;  /// this is the allocated space for the blob ALWAYS in byte! this prevents us to realloc it too often
		};

		// settingname with terminating NULL
		// blob
	};
} TSetting;

#pragma pack(pop)


/**
	\brief Manages the Settings in the Database
**/
class CSettingsTree : public CFileBTree<TSettingKey, TDBSettingHandle, 8, false>
{
protected:
	TDBEntryHandle m_Entry;

public: 
	CSettingsTree(CBlockManager & BlockManager, TNodeRef RootNode, TDBEntryHandle Entry);
	~CSettingsTree();

	TDBEntryHandle getEntry();

	TDBSettingHandle _FindSetting(const uint32_t Hash, const char * Name, const uint32_t Length); 
	bool _DeleteSetting(const uint32_t Hash, const TDBSettingHandle hSetting);
	//bool _ChangeSetting(const uint32_t Hash, const TDBSettingHandle OldSetting, const TDBSettingHandle NewSetting);
	bool _AddSetting(const uint32_t Hash, const TDBSettingHandle hSetting);
};


/**
	\brief Manages all Settings and provides access to them
**/
class CSettings : public sigslot::has_slots<>
{
public:
	typedef sigslot::signal2<CSettings*, CSettingsTree::TNodeRef> TOnRootChanged;

	static const uint32_t cSettingsFileFlag = 0x00000001;

	CSettings(CBlockManager & BlockManagerSet, CBlockManager & BlockManagerPri, CMultiReadExclusiveWriteSynchronizer & Synchronize, CSettingsTree::TNodeRef SettingsRoot, CEntries & Entries);
	virtual ~CSettings();

	TOnRootChanged & sigRootChanged();


	// services:
	TDBSettingHandle FindSetting(TDBSettingDescriptor & Descriptor);
	unsigned int DeleteSetting(TDBSettingDescriptor & Descriptor);
	unsigned int DeleteSetting(TDBSettingHandle hSetting);
	TDBSettingHandle WriteSetting(TDBSetting & Setting);
	TDBSettingHandle WriteSetting(TDBSetting & Setting, TDBSettingHandle hSetting);
	unsigned int ReadSetting(TDBSetting & Setting);
	unsigned int ReadSetting(TDBSetting & Setting, TDBSettingHandle hSetting);


	TDBSettingIterationHandle IterationInit(TDBSettingIterFilter & Filter);
	TDBSettingHandle IterationNext(TDBSettingIterationHandle Iteration);
	unsigned int IterationClose(TDBSettingIterationHandle Iteration);

private:
	typedef std::map<TDBEntryHandle, CSettingsTree*> TSettingsTreeMap;
	typedef CIterationHeap<CSettingsTree::iterator> TSettingsHeap;

	CMultiReadExclusiveWriteSynchronizer & m_Sync;
	CBlockManager & m_BlockManagerSet;
	CBlockManager & m_BlockManagerPri;

	CEntries & m_Entries;

	TSettingsTreeMap m_SettingsMap;

	typedef struct TSettingIterationResult {
		TDBSettingHandle Handle;
		TDBEntryHandle Entry;
		char * Name;
		uint16_t NameLen;
	} TSettingIterationResult;

	typedef struct TSettingIteration {
		TDBSettingIterFilter Filter;
		uint16_t FilterNameStartLength;
		TSettingsHeap * Heap;
		std::queue<TSettingIterationResult> * Frame;
	} TSettingIteration, *PSettingIteration;

	unsigned int m_IterAllocSize;
	TSettingIteration **m_Iterations;

	TOnRootChanged m_sigRootChanged;
	void onRootChanged(void* SettingsTree, CSettingsTree::TNodeRef NewRoot);

	CSettingsTree * getSettingsTree(TDBEntryHandle hEntry);

	uint32_t Hash(void * Data, uint32_t Length);

	
};
