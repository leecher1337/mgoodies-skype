#pragma once

#include "stdint.h"
#include "MREWSync.h"

#include "Events.h"
#include "Settings.h"
#include "Contacts.h"

#include "FileAccess.h"
#include "MappedMemory.h"
#include "DirectAccess.h"
#include "Blockmanager.h"

#include "sigslot.h"

#include "CipherList.inc"


typedef enum TDBFileType {
	DBFileSetting = 0,
	DBFilePrivate = 1,
	DBFileMax = 2
} TDBFileType;

static const uint8_t cFileSignature[DBFileMax][20] = {"Miranda IM Settings", "Miranda IM DataTree"};
static const uint32_t cDBVersion = 0x00000001;

static const uint32_t cDBFAEncryptedMask  = 0x00000003;
static const uint32_t cDBFAEncryptFull    = 0x00000003;
static const uint32_t cDBFAEncryptBlocks  = 0x00000002;
static const uint32_t cDBFAEncryptHistory = 0x00000001;

static const uint32_t cDBFAEncryptMethodMask = 0xFF000000;

static const uint32_t cHeaderBlockSignature = 0x7265491E;

#pragma pack(push, 1)  // push current alignment to stack, set alignment to 1 byte boundary


typedef struct TSettingsHeader {
	uint8_t Signature[20];       /// signature must be cSettingsHeader
	uint32_t Version;            /// internal DB version cDataBaseVersion
	uint32_t FileAccess;         /// File Access method
	uint32_t FileSize;           /// Offset to the last used byte + 1		
	uint32_t Settings;           /// Offset to the SettingsBTree RootNode	
	uint8_t Reserved[92];        /// reserved storage
} TSettingsHeader;

typedef struct TPrivateHeader {
	uint8_t Signature[20];       /// signature must be CDataHeader
	uint32_t Version;            /// internal DB version cDataBaseVersion
	uint32_t FileAccess;         /// File Access method
	uint32_t FileSize;           /// Offset to the last used byte + 1
	uint32_t RootContact;          /// Offset to the Root CList Contact
	uint32_t Contacts;            /// Offset to the ContactBTree RootNode
	uint32_t Virtuals;           /// Offset to the VirtualsBTree RootNode	
	uint32_t EventLinks;         /// 
	uint8_t Reserved[80];        /// reserved storage
} TPrivateHeader;


typedef union TGenericFileHeader {
	struct {
		uint8_t Signature[20];       /// signature must be cSettingsHeader
		uint32_t Version;            /// internal DB version cDataBaseVersion
		uint32_t FileAccess;         /// File Access method
		uint32_t FileSize;           /// Offset to the last used byte + 1	
		uint8_t Reserved[96];        /// reserved storage
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
	CCipher *m_Cipher[DBFileMax];

	uint32_t m_HeaderBlock[DBFileMax];

	void onSettingsRootChanged(CSettings* Settings, CSettingsTree::TNodeRef NewRoot);
	void onVirtualsRootChanged(void* Virtuals, CVirtuals::TNodeRef NewRoot);
	void onContactsRootChanged(void* Contacts, CContacts::TNodeRef NewRoot);

	bool PrivateFileExists();
	bool CreateNewPrivateFile();

	int CheckFile(TDBFileType Index);
	int LoadFile(TDBFileType Index);
	CCipher* MakeCipher(uint32_t Access);
protected:
	CMultiReadExclusiveWriteSynchronizer m_Sync;

	CContacts *m_Contacts;
	CSettings *m_Settings;

	void ReWriteHeader(TDBFileType Index);


public:
	CDataBase(const char* FileName);
	virtual ~CDataBase();

	int CreateDB();
	int CheckDB();
	int OpenDB();

	CContacts & getContacts();
	CSettings & getSettings();

};


extern CDataBase *gDataBase;