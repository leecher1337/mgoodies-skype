#ifndef M_DBX_TREE_H__

#define M_DBX_TREE_H__ 1

#include "stdint.h"
#pragma pack(push, 8)


/**
	\brief general return value if invalid param or invalid combination of params specified
**/
static const unsigned int DBT_INVALIDPARAM = 0xFFFFFFFF;


///////////////////////////////////////////////////////////
// Contacts
///////////////////////////////////////////////////////////

/**
	\brief A handle to a Contact
**/
typedef uint32_t TDBTContactHandle;


static const uint32_t DBT_CF_IsRoot      = 0x00000001;  /// Contact is the Root
static const uint32_t DBT_CF_IsGroup     = 0x00000002;  /// Contact is group
static const uint32_t DBT_CF_HasChildren = 0x00000004;  /// Contact has Children (for Groups and Metacontacts)
static const uint32_t DBT_CF_IsVirtual   = 0x00000008;  /// Contact is a Virtual duplicate
static const uint32_t DBT_CF_HasVirtuals = 0x00000010;  /// Contact has min. one Virtual duplicate

///////////////////////////////////////////////////////////
// Contacts
///////////////////////////////////////////////////////////

/**
	\brief
	\param wParam = 0
	\param lParam = 0

	\return Handle to root Contact
**/
#define MS_DBT_CONTACT_GETROOT "DBT/Contact/GetRoot"


/**
	\brief
	\param wParam = hContact
	\param lParam = 0

	\return ChildCount of specified Contact
**/
#define MS_DBT_CONTACT_CHILDCOUNT "DBT/Contact/ChildCount"


/**
	\brief
	\param wParam = hContact
	\param lParam = 0

	\return Parent hContact of specified Contact
**/
#define MS_DBT_CONTACT_GETPARENT "DBT/Contact/GetParent"


/**
	\brief
  \param wParam = hContact
  \param lParam = hParentContact

	\return 0 on success
**/
#define MS_DBT_CONTACT_SETPARENT  "DBT/Contact/SetParent"


/**
	\brief
	\param wParam = hContact
	\param lParam = 0

	\return First Child
**/
#define MS_DBT_CONTACT_GETFIRSTCHILD "DBT/Contact/GetFirstChild"


/**
	\brief
	\param wParam = hContact
	\param lParam = 0

	\return Last Child
**/
#define MS_DBT_CONTACT_GETLASTCHILD "DBT/Contact/GetLastChild"


/**
	\brief
	\param wParam = hContact
	\param lParam = 0

	\return Next Contact with same parent
**/
#define MS_DBT_CONTACT_GETNEXTSILBING "DBT/Contact/GetNextSilbing"


/**
	\brief
	\param wParam = hContact
	\param lParam = 0

	\return Previous Contact with same parent
**/
#define MS_DBT_CONTACT_GETPREVSILBING "DBT/Contact/GetPrevSilbing"

/**
	\brief Read the flags of an Contact
  \param wParam = hContact
  \param lParam = 0

	\return Flags
**/
#define MS_DBT_CONTACT_GETFLAGS "DBT/Contact/GetFlags"



static const uint32_t DBT_CIFO_OSC_AC   = 0x00000001;                  /// onStartContact - AddChildren
static const uint32_t DBT_CIFO_OSC_AP   = 0x00000002;                  /// onStartContact - AddParent
static const uint32_t DBT_CIFO_OSC_AO   = 0x00000004;                  /// onStartContact - AddOriginal (only if contact is virtual)
static const uint32_t DBT_CIFO_OSC_AOC  = 0x00000008 | DBT_CIFO_OSC_AO; /// onStartContact - AddOriginalChildren (only if contact is virtual)
static const uint32_t DBT_CIFO_OSC_AOP  = 0x00000010 | DBT_CIFO_OSC_AO; /// onStartContact - AddOriginalParent (only if contact is virtual)

static const uint32_t DBT_CIFO_OC_AC    = 0x00000001 <<8;                 /// onChildContact - AddChildren
//static const uint32_t DBT_LC_OC_AP      = 0x00000002 <<8;                // invalid for children
static const uint32_t DBT_CIFO_OC_AO    = 0x00000004 <<8;                 /// onChildContact - AddOriginal (only if contact is virtual)
static const uint32_t DBT_CIFO_OC_AOC   = 0x00000008 <<8 | DBT_CIFO_OC_AO; /// onChildContact - AddOriginalChildren (only if contact is virtual)
static const uint32_t DBT_CIFO_OC_AOP   = 0x00000010 <<8 | DBT_CIFO_OC_AO; /// onChildContact - AddOriginalParent (only if contact is virtual)

static const uint32_t DBT_CIFO_OP_AC    = 0x00000001 <<16;                 /// onParentContact - AddChildren
static const uint32_t DBT_CIFO_OP_AP    = 0x00000002 <<16;                 /// onParentContact - AddParent
static const uint32_t DBT_CIFO_OP_AO    = 0x00000004 <<16;                 /// onParentContact - AddOriginal (only if contact is virtual)
static const uint32_t DBT_CIFO_OP_AOC   = 0x00000008 <<16 | DBT_CIFO_OP_AO; /// onParentContact - AddOriginalChildren (only if contact is virtual)
static const uint32_t DBT_CIFO_OP_AOP   = 0x00000010 <<16 | DBT_CIFO_OP_AO; /// onParentContact - AddOriginalParent (only if contact is virtual)

static const uint32_t DBT_CIFO_GF_DEPTHFIRST = 0x01000000;  /// general flags - depth first iteration instead of breath first
static const uint32_t DBT_CIFO_GF_USEROOT    = 0x02000000;  /// general flags - use root as fallback, only for settings
static const uint32_t DBT_CIFO_GF_VL1        = 0x10000000;  /// general flags - limit virtual lookup depth to 1
static const uint32_t DBT_CIFO_GF_VL2        = 0x20000000;  /// general flags - limit virtual lookup depth to 2
static const uint32_t DBT_CIFO_GF_VL3        = 0x30000000;  /// general flags - limit virtual lookup depth to 3
static const uint32_t DBT_CIFO_GF_VL4        = 0x40000000;  /// general flags - limit virtual lookup depth to 4

/**
	\brief Contactfilter options for Contact iteration
**/
typedef
	struct TDBTContactIterFilter
	{
		uint32_t cbSize;					/// size of the structur in bytes
		uint32_t Options;					/// Options for iteration: DB_EIFO_*
		uint32_t fHasFlags;				/// flags an Contact must have to be iterated
		uint32_t fDontHasFlags;		/// flags an Contact have not to have to be iterated
	} TDBTContactIterFilter, *PDBTContactIterFilter;

/**
	\brief Handle of an Contact-Iteration
**/
typedef uint32_t TDBTContactIterationHandle;
/**
	\brief initialize an iteration of Contacts
	\param wParam = PDBTContactIterFilter, NULL to iterate all Contacts (breadthfirst, all but root)
	\param lParam = TDBTContactHandle Contact, where iteration starts

	\return EnumID
**/
#define MS_DBT_CONTACT_ITER_INIT "DBT/Contact/Iter/Init"


/**
	\brief get the next Contact
	\param wParam = EnumID returned by MS_DBT_CONTACT_ITER_INIT
	\param lParam = 0

	\return hContact, 0 at the end
**/
#define MS_DBT_CONTACT_ITER_NEXT "DBT/Contact/Iter/Next"

/**
	\brief closes an iteration and frees its ressourcs
	\param wParam = IterationHandle returned by MS_DBT_CONTACT_ITER_INIT
	\param lParam = 0

	\return 0 on success
**/
#define MS_DBT_CONTACT_ITER_CLOSE "DBT/Contact/Iter/Close"

/**
	\brief Deletes an Contact.

	All children will be moved to its parent.
	If the Contact has virtual copies, history and settings will be transfered to the first duplicate.

	\param wParam = hContact
  \param lParam = 0

	\return 0 on success
**/
#define MS_DBT_CONTACT_DELETE  "DBT/Contact/Delete"


/**
	\brief Creates a new Contact.
  \param wParam = hParentContact
  \param lParam = Flags, only DBT_CF_IsGroup is allowed here.

	\return hContact on success, 0 otherwise
**/
#define MS_DBT_CONTACT_CREATE  "DBT/Contact/Create"


///////////////////////////////////////////////////////////
// Virtual Contacts
///////////////////////////////////////////////////////////

/**
	\brief Creates a virtual duplicate of an Contact
  \param wParam = hContact to duplicate, couldn't be a group (DBT_CF_IsGroup set to 0)
  \param lParam = hParentContact to place duplicate

	\return hContact of created duplicate
**/
#define MS_DBT_VIRTUALCONTACT_CREATE  "DBT/VirtualContact/Create"

/**
	\brief Retrieves the original Contact, which this is a duplicate of
  \param wParam = hContact of virtual Contact
  \param lParam = 0

	\return hContact of original contact
**/
#define MS_DBT_VIRTUALCONTACT_GETPARENT  "DBT/VirtualContact/GetParent"

/**
	\brief Retrieves the first virtual duplicate of an Contact (if any)
  \param wParam = hContact with virtual copies
  \param lParam

	\return hContact of first virtual duplicate
**/
#define MS_DBT_VIRTUALCONTACT_GETFIRST  "DBT/VirtualContact/GetFirst"

/**
	\brief Retrieves the following duplicate
  \param wParam = hVirtualContact of virtual Contact
  \param lParam = 0

	\return hContact of next duplicate, 0 if hVirtualContact was the last duplicate
**/
#define MS_DBT_VIRTUALCONTACT_GETNEXT  "DBT/VirtualContact/GetNext"


///////////////////////////////////////////////////////////
// Settings
///////////////////////////////////////////////////////////

/**
	\brief Handle of a Setting
**/
typedef uint32_t TDBTSettingHandle;


static const uint16_t DBT_ST_BYTE   = 0x01;
static const uint16_t DBT_ST_WORD   = 0x02;
static const uint16_t DBT_ST_DWORD  = 0x03;
static const uint16_t DBT_ST_QWORD  = 0x04;

static const uint16_t DBT_ST_CHAR   = 0x11;
static const uint16_t DBT_ST_SHORT  = 0x12;
static const uint16_t DBT_ST_INT    = 0x13;
static const uint16_t DBT_ST_INT64  = 0x14;

static const uint16_t DBT_ST_BOOL   = 0x20;
static const uint16_t DBT_ST_FLOAT  = 0x21;
static const uint16_t DBT_ST_DOUBLE = 0x22;

static const uint16_t DBT_ST_ASCIIZ = 0xff;
static const uint16_t DBT_ST_BLOB   = 0xfe;
static const uint16_t DBT_ST_UTF8   = 0xfd;
static const uint16_t DBT_ST_WCHAR  = 0xfc;

#if (defined(_UNICODE) || defined(UNICODE))
	static const uint16_t DBT_ST_TCHAR  = DBT_ST_WCHAR;
#else
	static const uint16_t DBT_ST_TCHAR  = DBT_ST_ASCIIZ;
#endif

static const uint16_t DBT_STF_Signed         = 0x10;
static const uint16_t DBT_STF_VariableLength = 0x80;



static const uint32_t DBT_SDF_FoundValid  = 0x00000001;
static const uint32_t DBT_SDF_HashValid   = 0x00000002;

/**
	\brief Describes a setting, its name and location
**/
typedef
	struct TDBTSettingDescriptor {
		uint32_t cbSize;                               /// size of the structure in bytes
		TDBTContactHandle Contact;                      /// Contacthandle where the setting can be found, or where searching starts
		char * pszSettingName;                         /// Setting name
		uint32_t Options;                              /// options describing where the setting can be found DBT_CIFO_*
		uint32_t Flags;                                /// Valid Flags. DBT_SDF_* describes which following values are valid (internal use)

		TDBTContactHandle FoundInContact;               /// internal use to avoid to do the searching twice
		uint32_t Hash;                                 /// internal used HashValue for settingname
		TDBTSettingHandle FoundHandle;                  /// internal used SettingHandle
	} TDBTSettingDescriptor, * PDBTSettingDescriptor;

/**
	\brief Describes a settings value

	it is never used alone, without a type qualifier
**/
typedef
	union TDBTSettingValue {
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
	} TDBTSettingValue;

/**
	\brief Describes a setting
**/
typedef
	struct TDBTSetting {
		uint32_t cbSize;		          /// size of the structure in bytes
		PDBTSettingDescriptor Descriptor;  /// pointer to a Setting descriptor used to locate the setting
		uint16_t Type;			        /// type of the setting, see DBT_ST_*
		TDBTSettingValue Value;		        /// Value of the setting according to Type
	} TDBTSetting, * PDBTSetting;



/**
	\brief retrieves the handle of the setting
  \param wParam = PDBTSettingDescriptor
  \param lParam = 0

	\return hSetting when found, 0 otherwise
**/
#define MS_DBT_SETTING_FIND  "DBT/Setting/Find"


/**
	\brief deletes the specified Setting
  \param wParam = PDBTSettingDescriptor
  \param lParam = 0

	\return hSetting when found, 0 otherwise
**/
#define MS_DBT_SETTING_DELETE  "DBT/Setting/Delete"

/**
	\brief deletes the specified Setting
  \param wParam = TDBTSettingHandle
  \param lParam = 0

	\return 0 on success
**/
#define MS_DBT_SETTING_DELETEHANDLE  "DBT/Setting/DeleteHandle"


/**
	\brief Write a setting (and creates it if neccessary)
  \param wParam = PDBTSetting
  \param lParam = 0

	\return TDBTSettingHandle on success, 0 otherwise
**/
#define MS_DBT_SETTING_WRITE  "DBT/Setting/Write"

/**
	\brief retrieves the handle of the setting
  \param wParam = PDBTSetting
  \param lParam = TDBTSettingHandle

	\return hSetting when found (could change!), 0 otherwise
**/
#define MS_DBT_SETTING_WRITEHANDLE  "DBT/Setting/WriteHandle"

/**
	\brief retrieves the value of the setting
  \param wParam = PDBTSetting
  \param lParam = 0

	\return SettingHandle
**/
#define MS_DBT_SETTING_READ  "DBT/Setting/Read"

/**
	\brief retrieves the value of the setting

	Also retrieves the SettingDescriptor if it is set and prepared correctly (name buffers set etc)
  \param wParam = PDBTSetting
  \param lParam = TDBTSettingHandle

	\return original settings type
**/
#define MS_DBT_SETTING_READHANDLE  "DBT/Setting/ReadHandle"



/**
	\brief Settings Filter Options for setting iteration
**/
typedef
	struct TDBTSettingIterFilter {
		uint32_t cbSize;								  /// size in bytes of this structure
		uint32_t Options;                 /// DBT_CIFO_* flags
		TDBTContactHandle hContact;        /// hContact which settings should be iterated (or where iteration begins)
		char * NameStart;                 /// if set != NULL the iteration will only return settings which name starts with this string
		uint32_t ExtraCount;              /// count of additional Contacts which settings are enumerated, size of the array pointed by ExtraContacts
		TDBTContactHandle * ExtraContacts; /// pointer to an array with additional Contact handles in prioritized order

		PDBTSettingDescriptor Descriptor;  /// if set, the iteration will fill in the correct data, you may set SettingsNameLength and SettingName to a buffer to recieve the name of each setting
		PDBTSetting Setting;	              /// if set, iteration loads every settings value, except variable length data (blob, strings) but returns their length

	} TDBTSettingIterFilter, *PDBTSettingIterFilter;


/**
	\brief Handle of a Setting-Iteration
**/
typedef uint32_t TDBTSettingIterationHandle;
/**
	\brief initialize an iteration of settings
	\param wParam = PDBTSettingIterFilter
	\param lParam = 0

	\return EnumID
**/
#define MS_DBT_SETTING_ITER_INIT "DBT/Setting/Iter/Init"


/**
	\brief get the next setting
	\param wParam = EnumID returned by MS_DBT_SETTING_ITER_INIT
	\param lParam = 0

	\return hSetting, 0 at the end
**/
#define MS_DBT_SETTING_ITER_NEXT "DBT/Setting/Iter/Next"

/**
	\brief closes an iteration and frees its ressourcs
	\param wParam = IterationHandle returned by MS_DBT_SETTING_ITER_INIT
	\param lParam = 0

	\return 0 on success
**/
#define MS_DBT_SETTING_ITER_CLOSE "DBT/Setting/Iter/Close"


///////////////////////////////////////////////////////////
// Events
///////////////////////////////////////////////////////////

typedef uint32_t TDBTEventHandle;


/**
	\brief this event was sent by the user. If not set this event was received.
**/
static const uint32_t DBT_EF_SENT  = 0x00000002;

/**
	\brief event has been read by the user. It does not need to be processed any more except for history.
**/
static const uint32_t DBT_EF_READ  = 0x00000004;

/**
	\brief event contains the right-to-left aligned text
**/
static const uint32_t DBT_EF_RTL   = 0x00000008;

/**
	\brief event contains a text in utf-8
**/
static const uint32_t DBT_EF_UTF   = 0x00000010;

/**
	\brief event belongs to more than one contact, so it started to count refernce instead of remembering its initial contact
**/
static const uint32_t DBT_EF_REFERENCECOUNTING  = 0x80000000;

/**
	\brief event is virtual. it is not stored to db file yet.
**/
static const uint32_t DBT_EF_VIRTUAL   = 0x00000020;

/**
	\brief describes an event
**/
typedef struct TDBTEvent {
	uint32_t    cbSize;     /// size of the structure in bytes
	char *			ModuleName; /// 
	uint32_t    Timestamp;  /// seconds since 00:00, 01/01/1970. Gives us times until 2106 unless you use the standard C library which is signed and can only do until 2038. In GMT.
	uint32_t    Flags;	    /// the omnipresent flags	
	uint32_t    EventType;  /// module-unique event type ID
	uint32_t    cbBlob;	    /// size of pBlob in bytes
	uint8_t  *  pBlob;	    /// pointer to buffer containing module-defined event data
} TDBTEvent, *PDBTEvent;

static const uint32_t DBT_EventType_Message     = 0;
static const uint32_t DBT_EventType_URL         = 1;
static const uint32_t DBT_EventType_Contacts    = 2;
static const uint32_t DBT_EventType_Added       = 1000;
static const uint32_t DBT_EventType_AuthRequest = 1001;  //specific codes, hence the module-
static const uint32_t DBT_EventType_File        = 1002;  //specific limit has been raised to 2000


/**
	\brief retrieves the blobsize of an event in bytes
  \param wParam = hEvent
  \param lParam = 0

	\return blobsize
**/
#define MS_DBT_EVENT_GETBLOBSIZE "DBT/Event/GetBlobSize"



/**
	\brief retrieves all information of an event
  \param wParam = hEvent
  \param lParam = PDBTEvent

	\return 0 on success
**/
#define MS_DBT_EVENT_GET "DBT/Event/Get"

/**
	\brief retrieves all information of an event
  \param wParam = hContact
  \param lParam = 0

	\return Event count of specified contact on success, DBT_INVALIDPARAM on error
**/
#define MS_DBT_EVENT_GETCOUNT "DBT/Event/GetCount"


/**
	\brief Deletes the specfied event
  \param wParam = hContact
  \param lParam = hEvent

	\return 0 on success
**/
#define MS_DBT_EVENT_DELETE "DBT/Event/Delete"

/**
	\brief Creates a new Event
  \param wParam = hContact
  \param lParam = PDBTEvent

	\return hEvent on success, 0 otherwise
**/

#define MS_DBT_EVENT_ADD "DBT/Event/Add"


/**
	\brief Changes the flags for an event to mark it as read.
  \param wParam = hContact
  \param lParam = hEvent

	\return New flags
**/
#define MS_DBT_EVENT_MARKREAD "DBT/Event/MarkRead"

/**
	\brief Saves a virtual event to file and changes the flags.
  \param wParam = hContact
  \param lParam = hEvent

	\return New flags
**/
#define MS_DBT_EVENT_WRITETODISK  "DBT/Event/WriteToDisk"

/**
	\brief Retrieves a handle to a contact that owns hEvent.
  \param wParam = hEvent
  \param lParam = 0

	\return NULL is a valid return value, meaning, as usual, the user.
					DBT_INVALIDPARAM if hDbEvent is invalid, or the handle to the contact on
					success
**/
#define MS_DBT_EVENT_GETCONTACT  "DBT/Event/GetContact"

/**
	\brief options to create event hard link
**/
typedef 
	struct TDBTEventHardLink {
		uint32_t cbSize;           /// size of the structure
		TDBTEventHandle hEvent;     /// event to link
		TDBTContactHandle hContact; /// contact to link to
		uint32_t Flags;            /// flags, only DBT_EF_VIRTUAL yet
	} TDBTEventHardLink, *PDBTEventHardLink;

/**
	\brief Creates a hard linked event.

	This service adds an existing event to a specific contact.
  \param wParam = PEventHardLink
  \param lParam = 0

	\return 0 on success
**/
#define MS_DBT_EVENT_HARDLINK  "DBT/Event/HardLink"


/**
	\brief Event Filter Options for event iteration
**/
typedef
	struct TDBTEventIterFilter {
		uint32_t cbSize;										/// size in bytes of this structure
		uint32_t Options;										/// DBT_CIFO_* flags
		TDBTContactHandle hContact;					/// hContact which events should be iterated (or where iteration begins)
		uint32_t ExtraCount;								/// count of additional Contacts which settings are enumerated, size of the array pointed by ExtraContacts
		TDBTContactHandle * ExtraContacts;   /// pointer to an array with additional Contact handles in prioritized order

		uint32_t tSince;                    /// timestamp when to start iteration, 0 for first item
		uint32_t tTill;                     /// timestamp when to stop iteration, 0 for last item

		PDBTEvent Event;	                    /// if set every events data gets stored there

	} TDBTEventIterFilter, *PDBTEventIterFilter;


/**
	\brief Handle of a Event-Iteration
**/
typedef uint32_t TDBTEventIterationHandle;
/**
	\brief initialize an iteration of events
	\param wParam = PDBTEventIterFilter
	\param lParam = 0

	\return EnumID
**/
#define MS_DBT_EVENT_ITER_INIT "DBT/Event/Iter/Init"


/**
	\brief get the next event
	\param wParam = EnumID returned by MS_DBT_EVENT_ITER_INIT
	\param lParam = 0

	\return hSetting, 0 at the end
**/
#define MS_DBT_EVENT_ITER_NEXT "DBT/Event/Iter/Next"

/**
	\brief closes an iteration and frees its ressourcs
	\param wParam = IterationHandle returned by MS_DBT_EVENT_ITER_INIT
	\param lParam = 0

	\return 0 on success
**/
#define MS_DBT_EVENT_ITER_CLOSE "DBT/Event/Iter/Close"



#pragma pack(pop)

#endif
