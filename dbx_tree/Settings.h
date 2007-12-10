#pragma once
#include "Interface.h"
#include "FileBTree.h"
#include "MREWSync.h"
#include "sigslot.h"
#include "Entries.h"
#include "IterationHeap.h"

#include <map>
#include <queue>

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

/**
	\brief Key Type of the SettingsBTree

	The setting names (UTF8) are hashed, so that they can easily be accessed.
**/

typedef struct TSettingKey {
	unsigned int Hash; /// 32 bit hash of the Setting name

	bool operator <  (const TSettingKey & Other) const;
	//bool operator <= (const TSettingKey & Other);
	bool operator == (const TSettingKey & Other) const;
	//bool operator >= (const TSettingKey & Other);
	bool operator >  (const TSettingKey & Other) const;
} TSettingKey;



static const unsigned int cSettingSignature = 0xF5B87A3D;

/**
	\brief The data of a setting

	A setting's data is variable length. The data is a TSetting-structure followed by varaible length data.
	- fixed data
	- SettingName (UTF8)
	- maybe blob data
**/
typedef struct TSetting {
	unsigned int   Signature;    /// Signature
	TDBEntryHandle Entry;			   /// Settings' entry
	unsigned int   Flags;        /// flags
	unsigned short Type;         /// setting type	
	unsigned short NameLength;   /// settingname length
	union {
		TDBSettingValue Value;     /// if type is fixed length, the data is stored rigth here
		
		struct {
			unsigned int BlobLength; /// if type is variable length this describes the length of the data in bytes
			unsigned int AllocSize;  /// this is the allocated space for the blob ALWAYS in byte! this prevents us to realloc it too often
		};

		// settingname with terminating NULL
		// blob
	};
} TSetting;

#pragma pack(pop)


/**
	\brief Manages the Settings in the Database
**/
class CSettingsTree : public CFileBTree<TSettingKey, unsigned int, 8, false>
{
protected:
	TDBEntryHandle m_Entry;

public: 
	CSettingsTree(CFileAccess & FileAccess, unsigned int RootNode, TDBEntryHandle Entry);
	~CSettingsTree();

	TDBEntryHandle getEntry();

	TDBSettingHandle _FindSetting(const unsigned int Hash, const char * Name, const unsigned int Length); 
	bool _DeleteSetting(const unsigned int Hash, const TDBSettingHandle hSetting);
	bool _ChangeSetting(const unsigned int Hash, const TDBSettingHandle OldSetting, const TDBSettingHandle NewSetting);
	bool _AddSetting(const unsigned int Hash, const TDBSettingHandle hSetting);
};


/**
	\brief Manages all Settings and provides access to them
**/
class CSettings : public sigslot::has_slots<>
{
public:
	typedef sigslot::signal2<CSettings*, unsigned int> TOnRootChanged;

	static const unsigned int cSettingsFileFlag = 0x00000002;

	CSettings(CFileAccess & SettingsAccess, CFileAccess & PrivateAccess, CMultiReadExclusiveWriteSynchronizer & Synchronize, unsigned int SettingsRoot, CEntries & Entries);
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
	CFileAccess & m_SettingsFile;
	CFileAccess & m_PrivateFile;
	CEntries & m_Entries;

	TSettingsTreeMap m_SettingsMap;

	typedef struct TSettingIterationResult {
		TDBSettingHandle Handle;
		TDBEntryHandle Entry;
		char * Name;
		unsigned short NameLen;
	} TSettingIterationResult;

	typedef struct TSettingIteration {
		TDBSettingIterFilter Filter;
		unsigned int FilterNameStartLength;
		TSettingsHeap * Heap;
		std::queue<TSettingIterationResult> * Frame;
	} TSettingIteration, *PSettingIteration;

	unsigned int m_IterAllocSize;
	TSettingIteration **m_Iterations;

	TOnRootChanged m_sigRootChanged;
	void onRootChanged(void* SettingsTree, unsigned int NewRoot);

	CSettingsTree * getSettingsTree(TDBEntryHandle hEntry);

	unsigned int Hash(void * Data, unsigned int Length);

	
};

