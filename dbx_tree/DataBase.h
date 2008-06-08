#pragma once

#include "stdint.h"
#include "MREWSync.h"
#include "Exception.h"

#include "Events.h"
#include "Settings.h"
#include "Contacts.h"

#include "FileAccess.h"
#include "MappedMemory.h"
#include "DirectAccess.h"
#include "Blockmanager.h"

#include "sigslot.h"

#include "EncryptionManager.h"

typedef enum TDBFileType {
	DBFileSetting = 0,
	DBFilePrivate = 1,
	DBFileMax = 2
} TDBFileType;

static const uint8_t cFileSignature[DBFileMax][20] = {"Miranda IM Settings", "Miranda IM DataTree"};
static const uint32_t cDBVersion = 0x00000001;

static const uint32_t cHeaderBlockSignature = 0x7265491E;

#pragma pack(push, 1)  // push current alignment to stack, set alignment to 1 byte boundary

typedef struct TSettingsHeader {
	uint8_t Signature[20];          /// signature must be cSettingsHeader
	uint32_t Version;               /// internal DB version cDataBaseVersion
	uint32_t Obscure;
	TFileEncryption FileEncryption; /// Encryption Method
	uint32_t FileSize;              /// Offset to the last used byte + 1		
	uint32_t Settings;              /// Offset to the SettingsBTree RootNode	
	uint8_t Reserved[256 - sizeof(TFileEncryption) - 20 - 4*sizeof(uint32_t)]; /// reserved storage
} TSettingsHeader;

typedef struct TPrivateHeader {
	uint8_t Signature[20];          /// signature must be CDataHeader
	uint32_t Version;               /// internal DB version cDataBaseVersion
	uint32_t Obscure;
	TFileEncryption FileEncryption; /// Encryption Method
	uint32_t FileSize;              /// Offset to the last used byte + 1
	uint32_t RootContact;           /// Offset to the Root CList Contact
	uint32_t Contacts;              /// Offset to the ContactBTree RootNode
	uint32_t Virtuals;              /// Offset to the VirtualsBTree RootNode	
	uint32_t EventLinks;            /// 
	uint8_t Reserved[256 - sizeof(TFileEncryption) - 20 - 7*sizeof(uint32_t)]; /// reserved storage
} TPrivateHeader;


typedef union TGenericFileHeader {
	struct {
		uint8_t Signature[20];          /// signature must be cSettingsHeader
		uint32_t Version;               /// internal DB version cDataBaseVersion
		uint32_t Obscure;
		TFileEncryption FileEncryption; /// Encryption Method
		uint32_t FileSize;              /// Offset to the last used byte + 1	
		uint8_t Reserved[256 - sizeof(TFileEncryption) - 20 - 3*sizeof(uint32_t)]; /// reserved storage
	} Gen;
	TSettingsHeader Set;
	TPrivateHeader Pri;
} TGenericFileHeader;

#pragma pack(pop)


class CDataBase : public sigslot::has_slots<>
{
private:
	char* m_FileName[DBFileMax];
	bool m_Opened;

	CBlockManager *m_BlockManager[DBFileMax];
	CFileAccess *m_FileAccess[DBFileMax];
	TGenericFileHeader m_Header[DBFileMax];
	CEncryptionManager *m_EncryptionManager[DBFileMax];

	uint32_t m_HeaderBlock[DBFileMax];

	void onSettingsRootChanged(CSettings* Settings, CSettingsTree::TNodeRef NewRoot);
	void onVirtualsRootChanged(void* Virtuals, CVirtuals::TNodeRef NewRoot);
	void onContactsRootChanged(void* Contacts, CContacts::TNodeRef NewRoot);
	void onEventLinksRootChanged(void* Events, CEventLinks::TNodeRef NewRoot);
	void onFileSizeChange(CFileAccess * File, uint32_t Size);

	bool PrivateFileExists();
	bool CreateNewFile(TDBFileType File);

	int CheckFile(TDBFileType Index);
	int LoadFile(TDBFileType Index);
protected:
	CMultiReadExclusiveWriteSynchronizer m_Sync;

	CContacts *m_Contacts;
	CSettings *m_Settings;
	CEvents   *m_Events;

	void ReWriteHeader(TDBFileType Index);

public:
	CDataBase(const char* FileName);
	virtual ~CDataBase();

	int CreateDB();
	int CheckDB();
	int OpenDB();

	CContacts & getContacts();
	CSettings & getSettings();
	CEvents   & getEvents();

};


extern CDataBase *gDataBase;