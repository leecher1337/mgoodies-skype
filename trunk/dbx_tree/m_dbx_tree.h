#ifndef M_DBX_TREE_H__

#define M_DBX_TREE_H__ 1

#include "stdint.h"

#pragma pack(push)
#pragma pack(4)


/**
	\brief invalid param specified or invalid combination
**/
static const unsigned int DB_INVALIDPARAM = 0xFFFFFFFF;


///////////////////////////////////////////////////////////
// Entries
///////////////////////////////////////////////////////////

/**
	\brief A handle to an Entry
**/
typedef uint32_t TDBEntryHandle;


/**
	\brief Entry is the Root
**/
static const uint32_t DB_EF_IsRoot = 0x00000001;
/**
	\brief Entry isn't a Contact
**/
static const uint32_t DB_EF_IsGroup   = 0x00000002;
/**
	\brief Entry has Childs (Groups and Metacontacts)
**/
static const uint32_t DB_EF_HasChildren = 0x00000004;
/**
	\brief Entry is a Virtual duplicate
**/
static const uint32_t DB_EF_IsVirtual = 0x00000008;
/**
	\brief Entry has min one Virtual duplicate
**/
static const uint32_t DB_EF_HasVirtuals = 0x00000010;

///////////////////////////////////////////////////////////
// Entries
///////////////////////////////////////////////////////////

/**
	\brief 
	\param wParam = 0
	\param lParam = 0

	\return Handle to root entry
**/
#define MS_DB_ENTRY_GETROOT "DB/Entry/GetRoot"


/**
	\brief 
	\param wParam = hEntry
	\param lParam = 0

	\return ChildCount of specified Entry
**/
#define MS_DB_ENTRY_CHILDCOUNT "DB/Entry/ChildCount"


/**
	\brief 
	\param wParam = hEntry
	\param lParam = 0

	\return Parent hEntry of specified Entry
**/
#define MS_DB_ENTRY_GETPARENT "DB/Entry/GetParent"


/** 
	\brief 
  \param wParam = hEntry
  \param lParam = hParentEntry

	\return 0 on success
**/
#define MS_DB_ENTRY_SETPARENT  "DB/Entry/SetParent"


/**
	\brief 
	\param wParam = hEntry
	\param lParam = 0

	\return First Child
**/
#define MS_DB_ENTRY_GETFIRSTCHILD "DB/Entry/GetFirstChild"


/**
	\brief 
	\param wParam = hEntry
	\param lParam = 0

	\return Last Child
**/
#define MS_DB_ENTRY_GETLASTCHILD "DB/Entry/GetLastChild"


/**
	\brief 
	\param wParam = hEntry
	\param lParam = 0

	\return Next Entry with same parent
**/
#define MS_DB_ENTRY_GETNEXTSILBING "DB/Entry/GetNextSilbing"


/**
	\brief 
	\param wParam = hEntry
	\param lParam = 0

	\return Previous Entry with same parent
**/
#define MS_DB_ENTRY_GETPREVSILBING "DB/Entry/GetPrevSilbing"

/** 
	\brief Read the flags of an entry
  \param wParam = hEntry
  \param lParam = 0

	\return Flags
**/
#define MS_DB_ENTRY_GETFLAGS "DB/Entry/GetFlags"


/**
	\brief Option flag for entry iteration: changes the iteration behaviour from Breadthfirst to Depthfirst

	Standard behaviour of iteration is Breadthfirst: it goes through entries based on level.
	First the children of the specified hParentEntry are iterated, than their children (grouped by parent),
	than their children and so on.

	Depthfirst iterates the first child before it iterates the next silbing.
	It first iterates the first child of hParentEntry, than all its children and subchildren (as recursive),
	before it iterates the second child of hParentEntry and its children.
**/
static const uint32_t DB_EIFO_DEPTHFIRST = 0x00000001;

/**
	\brief Entryfilter options for entry iteration
**/
typedef
	struct TDBEntryIterFilter
	{
		uint32_t cbSize;					/// size of the structur in bytes
		uint32_t Options;					/// Options for iteration: DB_EIFO_*
		uint32_t fHasFlags;				/// flags an entry must have to be iterated
		uint32_t fDontHasFlags;		/// flags an entry have not to have to be iterated
		TDBEntryHandle hParentEntry;	/// entry which children should be iterated	
	} TDBEntryIterFilter, *PDBEntryIterFilter;

/**
	\brief Handle of an Entry-Iteration
**/
typedef uint32_t TDBEntryIterationHandle;
/**
	\brief initialize an iteration of entries
	\param wParam = PDBEntryIterFilter, NULL to iterate all entries (breadthfirst, all but root)
	\param lParam = 0

	\return EnumID
**/
#define MS_DB_ENTRY_ITER_INIT "DB/Entry/Iter/Init"


/**
	\brief get the next entry
	\param wParam = EnumID returned by MS_DB_ENTRY_ITER_INIT
	\param lParam = 0

	\return hEntry, 0 at the end
**/
#define MS_DB_ENTRY_ITER_NEXT "DB/Entry/Iter/Next"

/**
	\brief closes an iteration and frees its ressourcs
	\param wParam = IterationHandle returned by MS_DB_ENTRY_ITER_INIT
	\param lParam = 0

	\return 0 on success
**/
#define MS_DB_ENTRY_ITER_CLOSE "DB/Entry/Iter/Close"

/** 
	\brief Deletes an Entry.
	
	All children will be moved to its parent.
	If the Entry has virtual copies, history and settings will be transfered to the first duplicate.
  
	\param wParam = hEntry
  \param lParam = 0

	\return 0 on success
**/
#define MS_DB_ENTRY_DELETE  "DB/Entry/Delete"


/** 
	\brief Creates a new Entry.
  \param wParam = hParentEntry
  \param lParam = Flags, only DB_EF_IsGroup is allowed here.

	\return hEntry on success, 0 otherwise
**/
#define MS_DB_ENTRY_CREATE  "DB/Entry/Create"




///////////////////////////////////////////////////////////
// Virtual Entries
///////////////////////////////////////////////////////////

/** 
	\brief Creates a virtual duplicate of an Entry
  \param wParam = hEntry to duplicate, couldn't be a group (DB_EF_IsGroup set to 0)
  \param lParam = hParentEntry to place duplicate

	\return hEntry of created duplicate
**/
#define MS_DB_VIRTUALENTRY_CREATE  "DB/VirtualEntry/Create"

/** 
	\brief Retrieves the original entry, which this is a duplicate of
  \param wParam = hEntry of virtual entry
  \param lParam = 0

	\return hEntry of original contact
**/
#define MS_DB_VIRTUALENTRY_GETPARENT  "DB/VirtualEntry/GetParent"

/** 
	\brief Retrieves the first virtual duplicate of an entry (if any)
  \param wParam = hEntry with virtual copies
  \param lParam

	\return hEntry of first virtual duplicate
**/
#define MS_DB_VIRTUALENTRY_GETFIRST  "DB/VirtualEntry/GetFirst"

/** 
	\brief Retrieves the following duplicate
  \param wParam = hVirtualEntry of virtual entry
  \param lParam = 0

	\return hEntry of next duplicate, 0 if hVirtualEntry was the last duplicate
**/
#define MS_DB_VIRTUALENTRY_GETNEXT  "DB/VirtualEntry/GetNext"


///////////////////////////////////////////////////////////
// Settings
///////////////////////////////////////////////////////////

/**
	\brief Handle of a Setting
**/
typedef uint32_t TDBSettingHandle;


static const uint16_t DB_ST_BYTE   = 0x01;
static const uint16_t DB_ST_WORD   = 0x02;
static const uint16_t DB_ST_DWORD  = 0x03;
static const uint16_t DB_ST_QWORD  = 0x04;

static const uint16_t DB_ST_CHAR   = 0x11;
static const uint16_t DB_ST_SHORT  = 0x12;
static const uint16_t DB_ST_INT    = 0x13;
static const uint16_t DB_ST_INT64  = 0x14;

static const uint16_t DB_ST_BOOL   = 0x20;
static const uint16_t DB_ST_FLOAT  = 0x21;
static const uint16_t DB_ST_DOUBLE = 0x22;

static const uint16_t DB_ST_ASCIIZ = 0xff;
static const uint16_t DB_ST_BLOB   = 0xfe;
static const uint16_t DB_ST_UTF8   = 0xfd;
static const uint16_t DB_ST_WCHAR  = 0xfc;

#if (defined(_UNICODE) || defined(UNICODE))
	static const uint16_t DB_ST_TCHAR  = DB_ST_WCHAR;
#else
	static const uint16_t DB_ST_TCHAR  = DB_ST_ASCIIZ;
#endif

static const uint16_t DB_STF_Signed         = 0x10;
static const uint16_t DB_STF_VariableLength = 0x80;


/**
	\brief Setting Descriptor flag

	If the setting was not found, it will be searched in the children of the entry.
**/
static const uint32_t DB_SDF_SearchSubEntries       = 0x00000001;

/**
	\brief Setting Descriptor flag

	If the setting of a group was not found, it will be searched in all children of this group, 
	even in children which aren't groups.

	Please don't use this flag
**/
static const uint32_t DB_SDF_SearchOutOfGroups      = 0x00000002 | DB_SDF_SearchSubEntries;
/**
	\brief Setting Descriptor flag

	If the setting was not found, it will be searched in the parent entries, while it isn't a group.
**/
static const uint32_t DB_SDF_SearchParents          = 0x00000004;
/**
	\brief Setting Descriptor flag

	If the setting was not found, we look in the root entry for it.
**/
static const uint32_t DB_SDF_RootHasStandard        = 0x00000008;
/**
	\brief Setting Descriptor flag

	Turns off the lookup for original entry settings, if the specified entry is a virtual duplicate.
	And only for the specified entry. Children will do the lookup.
	
	Changes the save destination. for standard settings are allways stored in the original entry.
	With this flag set, the setting will be stored in the virtual duplicate and superseed the original's setting.
**/
static const uint32_t DB_SDF_NoPrimaryVirtualLookup = 0x00000010;
/**
	\brief Setting Descriptor flag

	Turns off the lookup for original entry settings, if the specified entry is a virtual duplicate.
	Turns it off for all searched entries.
**/
static const uint32_t DB_SDF_NoVirtualLookup        = 0x00000020 | DB_SDF_NoPrimaryVirtualLookup;

/**
	\brief Setting Descriptor flag

	Setting was found in previous call to MS_DB_SETTING_FIND
	FoundInEntry is valid and will be used in future calls.
	Used for speed improvements.
	\warning do not set this flag on your own!
**/
static const uint32_t DB_SDF_FoundEntryValid        = 0x01000000;


/**
	\brief Describes a setting, its name and location
**/
typedef
	struct TDBSettingDescriptor {
		uint32_t cbSize;                           /// size of the structure in bytes
		TDBEntryHandle Entry;                          /// Entryhandle where the setting can be found, or where searching starts
		char * pszSettingName;                         /// Setting name
		uint32_t Flags;                            /// flags describing where the setting can be found DB_SDF_*

		TDBEntryHandle FoundInEntry;                   /// internal use to avoid to do the searching twice
	} TDBSettingDescriptor, * PDBSettingDescriptor;

/**
	\brief Describes a settings value

	it is never used alone, without a type qualifier
**/
typedef
	union TDBSettingValue {
		bool Bool;
		int8_t  Char;  uint8_t  Byte;
		int16_t Short; uint16_t Word;
		uint32_t Int;   uint32_t DWord;
		int64_t Int64; uint64_t QWord;
		float Float;
		double Double;

		struct {
			uint32_t Length;  // length in bytes of pBlob, length in characters of char types including trailing null
			union {
				uint8_t * pBlob;				
				char * pAnsii;
				char * pUTF8;
				wchar_t * pWide;
				TCHAR * pTChar;
			};
		};
	} TDBSettingValue;

/**
	\brief Describes a setting
**/
typedef
	struct TDBSetting {
		uint32_t cbSize;		          /// size of the structure in bytes
		PDBSettingDescriptor Descriptor;  /// pointer to a Setting descriptor used to locate the setting
		uint16_t Type;			        /// type of the setting, see DB_ST_*
		TDBSettingValue Value;		        /// Value of the setting according to Type
	} TDBSetting, * PDBSetting;



/** 
	\brief retrieves the handle of the setting
  \param wParam = PDBSettingDescriptor
  \param lParam = 0

	\return hSetting when found, 0 otherwise
**/
#define MS_DB_SETTING_FIND  "DB/Setting/Find"


/** 
	\brief deletes the specified Setting
  \param wParam = PDBSettingDescriptor
  \param lParam = 0

	\return hSetting when found, 0 otherwise
**/
#define MS_DB_SETTING_DELETE  "DB/Setting/Delete"

/** 
	\brief deletes the specified Setting
  \param wParam = TDBSettingHandle
  \param lParam = 0

	\return 0 on success
**/
#define MS_DB_SETTING_DELETEHANDLE  "DB/Setting/DeleteHandle"


/** 
	\brief Write a setting (and creates it if neccessary)
  \param wParam = PDBSetting
  \param lParam = 0

	\return TDBSettingHandle on success, 0 otherwise
**/
#define MS_DB_SETTING_WRITE  "DB/Setting/Write"

/** 
	\brief retrieves the handle of the setting
  \param wParam = PDBSetting
  \param lParam = TDBSettingHandle

	\return hSetting when found (could change!), 0 otherwise
**/
#define MS_DB_SETTING_WRITEHANDLE  "DB/Setting/WriteHandle"

/** 
	\brief retrieves the value of the setting
  \param wParam = PDBSetting
  \param lParam = 0

	\return SettingHandle
**/
#define MS_DB_SETTING_READ  "DB/Setting/Read"

/** 
	\brief retrieves the value of the setting

	Also retrieves the SettingDescriptor if it is set and prepared correctly (name buffers set etc)
  \param wParam = PDBSetting
  \param lParam = TDBSettingHandle

	\return original settings type
**/
#define MS_DB_SETTING_READHANDLE  "DB/Setting/ReadHandle"



	/**
	\brief Setting Iteration Filter Option

	Settings of all Children will be iterated.
**/
static const uint32_t DB_SIFO_SearchSubEntries       = 0x00000001;

/**
	\brief Setting Iteration Filter Option

	all children of this group, even in children which aren't groups.

	Please don't use this flag
**/
static const uint32_t DB_SIFO_SearchOutOfGroups      = 0x00000002 | DB_SDF_SearchSubEntries;
/**
	\brief Setting Iteration Filter Option

	Iterates the settings of any parent entry, as long it isn't a group
**/
static const uint32_t DB_SIFO_SearchParents          = 0x00000004;
/**
	\brief Setting Iteration Filter Option

	Iterates the settings of the root entry, which could hold standard values
**/
static const uint32_t DB_SIFO_RootHasStandard        = 0x00000008;
/**
	\brief Setting Iteration Filter Option

	Turns off the lookup for original entry settings, if the specified entry is a virtual duplicate.
	And only for the specified entry. Children will do the lookup.
**/
static const uint32_t DB_SIFO_NoPrimaryVirtualLookup = 0x00000010;
/**
	\brief Setting Iteration Filter Option

	Turns off the lookup for original entry settings, if the specified entry is a virtual duplicate.
	Turns it off for all searched entries.
**/
static const uint32_t DB_SIFO_NoVirtualLookup        = 0x00000020 | DB_SDF_NoPrimaryVirtualLookup;


/**
	\brief Settings Filter Options for setting iteration
**/
typedef
	struct TDBSettingIterFilter {
		uint32_t cbSize;								/// size in bytes of this structure
		uint32_t Options;               /// DB_SIFO_* flags
		TDBEntryHandle hEntry;              /// hEntry which settings should be iterated (or where iteration begins)
		char * NameStart;                   /// if set != NULL the iteration will only return settings which name starts with this string
		uint32_t ExtraCount;            /// count of additional Entries which settings are enumerated, size of the array pointed by ExtraEntries
		TDBEntryHandle * ExtraEntries;      /// pointer to an array with additional Entry handles in prioritized order

		PDBSettingDescriptor Descriptor;    /// if set, the iteration will fill in the correct data, you may set SettingsNameLength and SettingName to a buffer to recieve the name of each setting
		PDBSetting Setting;	                /// if set, iteration loads every settings value, except variable length data (blob, strings) but returns their length

	} TDBSettingIterFilter, *PDBSettingIterFilter;


/**
	\brief Handle of a Setting-Iteration
**/
typedef uint32_t TDBSettingIterationHandle;
/**
	\brief initialize an iteration of settings
	\param wParam = PDBSettingIterFilter
	\param lParam = 0

	\return EnumID
**/
#define MS_DB_SETTING_ITER_INIT "DB/Setting/Iter/Init"


/**
	\brief get the next setting
	\param wParam = EnumID returned by MS_DB_SETTING_ITER_INIT
	\param lParam = 0

	\return hSetting, 0 at the end
**/
#define MS_DB_SETTING_ITER_NEXT "DB/Setting/Iter/Next"

/**
	\brief closes an iteration and frees its ressourcs
	\param wParam = IterationHandle returned by MS_DB_SETTING_ITER_INIT
	\param lParam = 0

	\return 0 on success
**/
#define MS_DB_SETTING_ITER_CLOSE "DB/Setting/Iter/Close"


///////////////////////////////////////////////////////////
// Events
///////////////////////////////////////////////////////////


typedef uint32_t TDBEventHandle;


/**
	\brief this is the first event in the chain, internal only: *do not* use this flag

	\warning obsolete in dbx_tree
**/
static const uint32_t DB_EVF_First = 0x00000001;

/**
	\brief this event was sent by the user. If not set this event was received.
**/
static const uint32_t DB_EVF_Sent  = 0x00000002; 

/**
	\brief event has been read by the user. It does not need to be processed any more except for history.
**/
static const uint32_t DB_EVF_Read  = 0x00000004; 

/**
	\brief event contains the right-to-left aligned text
**/
static const uint32_t DB_EVF_RTL   = 0x00000008;

/**
	\brief event contains a text in utf-8
**/
static const uint32_t DB_EVF_UTF   = 0x00000010;

typedef struct TDBEvent {
	uint32_t    cbSize;    /// size of the structure in bytes
	uint32_t    timestamp; /// seconds since 00:00, 01/01/1970. Gives us times until 2106 unless you use the standard C library which is signed and can only do until 2038. In GMT.
	uint32_t    flags;	   /// the omnipresent flags
	uint32_t    eventType; /// global-unique event type ID
	uint32_t    cbBlob;	   /// size of pBlob in bytes
	uint8_t  * pBlob;	   /// pointer to buffer containing module-defined event data

//	TDBEntryHandle  hEntry;    /// Entry to which this event belongs 
} TDBEvent, *PDBEvent;

static const uint32_t DB_EventType_Message     = 0;
static const uint32_t DB_EventType_URL         = 1;
static const uint32_t DB_EventType_Contacts    = 2;
static const uint32_t DB_EventType_Added       = 1000;
static const uint32_t DB_EventType_AuthRequest = 1001;  //specific codes, hence the module-
static const uint32_t DB_EventType_File        = 1002;  //specific limit has been raised to 2000


/** 
	\brief retrieves the blobsize of an event in bytes
  \param wParam = hEvent
  \param lParam = 0

	\return blobsize
**/
#define MS_DB_EVENT_GETBLOBSIZE "DB/Event/GetBlobSize"



/** 
	\brief retrieves all information of an event
  \param wParam = hEvent
  \param lParam = PDBEvent

	\return 0 on success
**/
#define MS_DB_EVENT_GET "DB/Event/Get"


/** 
	\brief Deletes the specfied event
  \param wParam = hEvent
  \param lParam = 0

	\return 0 on success
**/
#define MS_DB_EVENT_DELETE "DB/Event/Delete"

/** 
	\brief Creates a new Event
  \param wParam = hEntry
  \param lParam = 0

	\return hEvent on success, 0 otherwise
**/

#define MS_DB_EVENT_CREATE "DB/Event/Create"





#pragma pack(pop)

#endif