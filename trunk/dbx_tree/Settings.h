#pragma once

#include "Interface.h"
#include "FileBTree.h"
#include "MREWSync.h"
#include "sigslot.h"
#include "IterationHeap.h"
#include "EncryptionManager.h"

#include <hash_map>
#include <queue>
#include <vector>

class CSettings;
class CSettingsTree;

#include "Contacts.h"

#pragma pack(push, 1)  // push current alignment to stack, set alignment to 1 byte boundary

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
	TDBTContactHandle Contact; /// Settings' Contact
	uint32_t   Flags;         /// flags
	uint16_t   Type;          /// setting type	
	uint16_t   NameLength;    /// settingname length
	uint8_t    Reserved[8];
	union {
		TDBTSettingValue Value;  /// if type is fixed length, the data is stored rigth here
		
		struct {
			uint32_t BlobLength;  /// if type is variable length this describes the length of the data in bytes
			uint32_t AllocSize;   /// this is the allocated space for the blob ALWAYS in byte! this prevents us to realloc it too often
		};
	};
	
	// settingname with terminating NULL
  // blob
} TSetting;

#pragma pack(pop)

/**
	\brief Manages the Settings in the Database
**/
class CSettingsTree : public CFileBTree<TSettingKey, TDBTSettingHandle, 8, false>
{
protected:
	TDBTContactHandle m_Contact;
	CSettings & m_Owner;
	CEncryptionManager & m_EncryptionManager;
public: 
	CSettingsTree(
		CSettings & Owner,
		CBlockManager & BlockManager,
		CEncryptionManager & EncryptionManager,
		TNodeRef RootNode,
		TDBTContactHandle Contact
		);
	~CSettingsTree();

	TDBTContactHandle getContact();
	void setContact(TDBTContactHandle NewContact);

	TDBTSettingHandle _FindSetting(const uint32_t Hash, const char * Name, const uint32_t Length); 
	bool _DeleteSetting(const uint32_t Hash, const TDBTSettingHandle hSetting);
	bool _AddSetting(const uint32_t Hash, const TDBTSettingHandle hSetting);
};


/**
	\brief Manages all Settings and provides access to them
**/
class CSettings : public sigslot::has_slots<>
{
public:
	typedef sigslot::signal2<CSettings*, CSettingsTree::TNodeRef> TOnRootChanged;

	static const uint32_t cSettingsFileFlag = 0x00000001;

	CSettings(
		CBlockManager & BlockManagerSet,
		CBlockManager & BlockManagerPri,
		CEncryptionManager & EncryptionManagerSet,
		CEncryptionManager & EncryptionManagerPri,
		CMultiReadExclusiveWriteSynchronizer & Synchronize, 
		CSettingsTree::TNodeRef SettingsRoot,
		CContacts & Contacts
		);
	virtual ~CSettings();

	TOnRootChanged & sigRootChanged();

	bool _ReadSettingName(CBlockManager & BlockManager, CEncryptionManager & EncryptionManager, TDBTSettingHandle Setting, uint16_t & NameLength, char *& NameBuf);

	// services:
	TDBTSettingHandle FindSetting(TDBTSettingDescriptor & Descriptor);
	unsigned int DeleteSetting(TDBTSettingDescriptor & Descriptor);
	unsigned int DeleteSetting(TDBTSettingHandle hSetting);
	TDBTSettingHandle WriteSetting(TDBTSetting & Setting);
	TDBTSettingHandle WriteSetting(TDBTSetting & Setting, TDBTSettingHandle hSetting);
	unsigned int ReadSetting(TDBTSetting & Setting);
	unsigned int ReadSetting(TDBTSetting & Setting, TDBTSettingHandle hSetting);


	TDBTSettingIterationHandle IterationInit(TDBTSettingIterFilter & Filter);
	TDBTSettingHandle IterationNext(TDBTSettingIterationHandle Iteration);
	unsigned int IterationClose(TDBTSettingIterationHandle Iteration);


private:
	typedef stdext::hash_map<TDBTContactHandle, CSettingsTree*> TSettingsTreeMap;
	typedef CIterationHeap<CSettingsTree::iterator> TSettingsHeap;

	CMultiReadExclusiveWriteSynchronizer & m_Sync;
	CBlockManager & m_BlockManagerSet;
	CBlockManager & m_BlockManagerPri;
	CEncryptionManager & m_EncryptionManagerSet;
	CEncryptionManager & m_EncryptionManagerPri;

	CContacts & m_Contacts;

	TSettingsTreeMap m_SettingsMap;

	typedef struct TSettingIterationResult {
		TDBTSettingHandle Handle;
		TDBTContactHandle Contact;
		char * Name;
		uint16_t NameLen;
	} TSettingIterationResult;

	typedef struct TSettingIteration {
		TDBTSettingIterFilter Filter;
		uint16_t FilterNameStartLength;
		TSettingsHeap * Heap;
		std::queue<TSettingIterationResult> * Frame;
	} TSettingIteration, *PSettingIteration;


	typedef std::vector<PSettingIteration> TSettingIterationVector;

	TSettingIterationVector m_Iterations;

	TOnRootChanged m_sigRootChanged;
	void onRootChanged(void* SettingsTree, CSettingsTree::TNodeRef NewRoot);

	void onDeleteSettingCallback(void * Tree, TSettingKey Key, TDBTSettingHandle Data, uint32_t Param);
	void onDeleteSettings(CContacts * Contacts, TDBTContactHandle hContact);
	void onMergeSettingCallback(void * Tree, TSettingKey Key, TDBTSettingHandle Data, uint32_t Param);
	void onMergeSettings(CContacts * Contacts, TDBTContactHandle Source, TDBTContactHandle Dest);

	CSettingsTree * getSettingsTree(TDBTContactHandle hContact);
	
};
