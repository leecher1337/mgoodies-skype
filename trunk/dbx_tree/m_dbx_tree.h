#ifndef M_DBX_TREE_H__

#define M_DBX_TREE_H__ 1

#include "stdint.h"
#pragma pack(push, 8)


/**
	\brief general return value if invalid param or invalid combination of params specified
**/
static const unsigned int DB_INVALIDPARAM = 0xFFFFFFFF;


///////////////////////////////////////////////////////////
// Contacts
///////////////////////////////////////////////////////////

/**
	\brief A handle to a Contact
**/
typedef uint32_t TDBContactHandle;


static const uint32_t DB_CF_IsRoot      = 0x00000001;  /// Contact is the Root
static const uint32_t DB_CF_IsGroup     = 0x00000002;  /// Contact is group
static const uint32_t DB_CF_HasChildren = 0x00000004;  /// Contact has Children (for Groups and Metacontacts)
static const uint32_t DB_CF_IsVirtual   = 0x00000008;  /// Contact is a Virtual duplicate
static const uint32_t DB_CF_HasVirtuals = 0x00000010;  /// Contact has min. one Virtual duplicate

///////////////////////////////////////////////////////////
// Contacts
///////////////////////////////////////////////////////////

/**
	\brief
	\param wParam = 0
	\param lParam = 0

	\return Handle to root Contact
**/
#define MS_DB_CONTACT_GETROOT "DB/Contact/GetRoot"


/**
	\brief
	\param wParam = hContact
	\param lParam = 0

	\return ChildCount of specified Contact
**/
#define MS_DB_CONTACT_CHILDCOUNT "DB/Contact/ChildCount"


/**
	\brief
	\param wParam = hContact
	\param lParam = 0

	\return Parent hContact of specified Contact
**/
#define MS_DB_CONTACT_GETPARENT "DB/Contact/GetParent"


/**
	\brief
  \param wParam = hContact
  \param lParam = hParentContact

	\return 0 on success
**/
#define MS_DB_CONTACT_SETPARENT  "DB/Contact/SetParent"


/**
	\brief
	\param wParam = hContact
	\param lParam = 0

	\return First Child
**/
#define MS_DB_CONTACT_GETFIRSTCHILD "DB/Contact/GetFirstChild"


/**
	\brief
	\param wParam = hContact
	\param lParam = 0

	\return Last Child
**/
#define MS_DB_CONTACT_GETLASTCHILD "DB/Contact/GetLastChild"


/**
	\brief
	\param wParam = hContact
	\param lParam = 0

	\return Next Contact with same parent
**/
#define MS_DB_CONTACT_GETNEXTSILBING "DB/Contact/GetNextSilbing"


/**
	\brief
	\param wParam = hContact
	\param lParam = 0

	\return Previous Contact with same parent
**/
#define MS_DB_CONTACT_GETPREVSILBING "DB/Contact/GetPrevSilbing"

/**
	\brief Read the flags of an Contact
  \param wParam = hContact
  \param lParam = 0

	\return Flags
**/
#define MS_DB_CONTACT_GETFLAGS "DB/Contact/GetFlags"



static const uint32_t DB_CIFO_OSC_AC   = 0x00000001;                  /// onStartContact - AddChildren
static const uint32_t DB_CIFO_OSC_AP   = 0x00000002;                  /// onStartContact - AddParent
static const uint32_t DB_CIFO_OSC_AO   = 0x00000004;                  /// onStartContact - AddOriginal (only if contact is virtual)
static const uint32_t DB_CIFO_OSC_AOC  = 0x00000008 | DB_CIFO_OSC_AO; /// onStartContact - AddOriginalChildren (only if contact is virtual)
static const uint32_t DB_CIFO_OSC_AOP  = 0x00000010 | DB_CIFO_OSC_AO; /// onStartContact - AddOriginalParent (only if contact is virtual)

static const uint32_t DB_CIFO_OC_AC    = 0x00000001 <<8;                 /// onChildContact - AddChildren
//static const uint32_t DB_LC_OC_AP      = 0x00000002 <<8;                // invalid for children
static const uint32_t DB_CIFO_OC_AO    = 0x00000004 <<8;                 /// onChildContact - AddOriginal (only if contact is virtual)
static const uint32_t DB_CIFO_OC_AOC   = 0x00000008 <<8 | DB_CIFO_OC_AO; /// onChildContact - AddOriginalChildren (only if contact is virtual)
static const uint32_t DB_CIFO_OC_AOP   = 0x00000010 <<8 | DB_CIFO_OC_AO; /// onChildContact - AddOriginalParent (only if contact is virtual)

static const uint32_t DB_CIFO_OP_AC    = 0x00000001 <<16;                 /// onParentContact - AddChildren
static const uint32_t DB_CIFO_OP_AP    = 0x00000002 <<16;                 /// onParentContact - AddParent
static const uint32_t DB_CIFO_OP_AO    = 0x00000004 <<16;                 /// onParentContact - AddOriginal (only if contact is virtual)
static const uint32_t DB_CIFO_OP_AOC   = 0x00000008 <<16 | DB_CIFO_OP_AO; /// onParentContact - AddOriginalChildren (only if contact is virtual)
static const uint32_t DB_CIFO_OP_AOP   = 0x00000010 <<16 | DB_CIFO_OP_AO; /// onParentContact - AddOriginalParent (only if contact is virtual)

static const uint32_t DB_CIFO_GF_DEPTHFIRST = 0x01000000;  /// general flags - depth first iteration instead of breath first
static const uint32_t DB_CIFO_GF_USEROOT    = 0x02000000;  /// general flags - use root as fallback, only for settings
static const uint32_t DB_CIFO_GF_VL1        = 0x10000000;  /// general flags - limit virtual lookup depth to 1
static const uint32_t DB_CIFO_GF_VL2        = 0x20000000;  /// general flags - limit virtual lookup depth to 2
static const uint32_t DB_CIFO_GF_VL3        = 0x30000000;  /// general flags - limit virtual lookup depth to 3
static const uint32_t DB_CIFO_GF_VL4        = 0x40000000;  /// general flags - limit virtual lookup depth to 4

/**
	\brief Contactfilter options for Contact iteration
**/
typedef
	struct TDBContactIterFilter
	{
		uint32_t cbSize;					/// size of the structur in bytes
		uint32_t Options;					/// Options for iteration: DB_EIFO_*
		uint32_t fHasFlags;				/// flags an Contact must have to be iterated
		uint32_t fDontHasFlags;		/// flags an Contact have not to have to be iterated
	} TDBContactIterFilter, *PDBContactIterFilter;

/**
	\brief Handle of an Contact-Iteration
**/
typedef uint32_t TDBContactIterationHandle;
/**
	\brief initialize an iteration of Contacts
	\param wParam = PDBContactIterFilter, NULL to iterate all Contacts (breadthfirst, all but root)
	\param lParam = TDBContactHandle Contact, where iteration starts

	\return EnumID
**/
#define MS_DB_CONTACT_ITER_INIT "DB/Contact/Iter/Init"


/**
	\brief get the next Contact
	\param wParam = EnumID returned by MS_DB_CONTACT_ITER_INIT
	\param lParam = 0

	\return hContact, 0 at the end
**/
#define MS_DB_CONTACT_ITER_NEXT "DB/Contact/Iter/Next"

/**
	\brief closes an iteration and frees its ressourcs
	\param wParam = IterationHandle returned by MS_DB_CONTACT_ITER_INIT
	\param lParam = 0

	\return 0 on success
**/
#define MS_DB_CONTACT_ITER_CLOSE "DB/Contact/Iter/Close"

/**
	\brief Deletes an Contact.

	All children will be moved to its parent.
	If the Contact has virtual copies, history and settings will be transfered to the first duplicate.

	\param wParam = hContact
  \param lParam = 0

	\return 0 on success
**/
#define MS_DB_CONTACT_DELETE  "DB/Contact/Delete"


/**
	\brief Creates a new Contact.
  \param wParam = hParentContact
  \param lParam = Flags, only DB_CF_IsGroup is allowed here.

	\return hContact on success, 0 otherwise
**/
#define MS_DB_CONTACT_CREATE  "DB/Contact/Create"


///////////////////////////////////////////////////////////
// Virtual Contacts
///////////////////////////////////////////////////////////

/**
	\brief Creates a virtual duplicate of an Contact
  \param wParam = hContact to duplicate, couldn't be a group (DB_CF_IsGroup set to 0)
  \param lParam = hParentContact to place duplicate

	\return hContact of created duplicate
**/
#define MS_DB_VIRTUALCONTACT_CREATE  "DB/VirtualContact/Create"

/**
	\brief Retrieves the original Contact, which this is a duplicate of
  \param wParam = hContact of virtual Contact
  \param lParam = 0

	\return hContact of original contact
**/
#define MS_DB_VIRTUALCONTACT_GETPARENT  "DB/VirtualContact/GetParent"

/**
	\brief Retrieves the first virtual duplicate of an Contact (if any)
  \param wParam = hContact with virtual copies
  \param lParam

	\return hContact of first virtual duplicate
**/
#define MS_DB_VIRTUALCONTACT_GETFIRST  "DB/VirtualContact/GetFirst"

/**
	\brief Retrieves the following duplicate
  \param wParam = hVirtualContact of virtual Contact
  \param lParam = 0

	\return hContact of next duplicate, 0 if hVirtualContact was the last duplicate
**/
#define MS_DB_VIRTUALCONTACT_GETNEXT  "DB/VirtualContact/GetNext"


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
	\brief Describes a setting, its name and location
**/
typedef
	struct TDBSettingDescriptor {
		uint32_t cbSize;                               /// size of the structure in bytes
		TDBContactHandle Contact;                      /// Contacthandle where the setting can be found, or where searching starts
		char * pszSettingName;                         /// Setting name
		uint32_t Flags;                                /// flags describing where the setting can be found DB_CIFO_*

		TDBContactHandle FoundInContact;               /// internal use to avoid to do the searching twice
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
	\brief Settings Filter Options for setting iteration
**/
typedef
	struct TDBSettingIterFilter {
		uint32_t cbSize;								  /// size in bytes of this structure
		uint32_t Options;                 /// DB_CIFO_* flags
		TDBContactHandle hContact;        /// hContact which settings should be iterated (or where iteration begins)
		char * NameStart;                 /// if set != NULL the iteration will only return settings which name starts with this string
		uint32_t ExtraCount;              /// count of additional Contacts which settings are enumerated, size of the array pointed by ExtraContacts
		TDBContactHandle * ExtraContacts; /// pointer to an array with additional Contact handles in prioritized order

		PDBSettingDescriptor Descriptor;  /// if set, the iteration will fill in the correct data, you may set SettingsNameLength and SettingName to a buffer to recieve the name of each setting
		PDBSetting Setting;	              /// if set, iteration loads every settings value, except variable length data (blob, strings) but returns their length

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

/**
	\brief Event Type Specification for register
**/
typedef struct TDBEventTypeDescriptor
{
	uint32_t cbSize;      /// structure size in bytes
	char *   ModuleName;  /// event module name
	uint32_t EventType;   /// event id, unique for this module
	char *   Description; /// event type description (i.e. "File Transfer")
}	TDBEventTypeDescriptor, *PDBEventTypeDescriptor;

#define MS_DB_EVENT_REGISTERTYPE  "DB/EventType/Register"

/** 
	\brief Retrieves the previously registered database event type, by module & id.
  \param wParam = ModuleName, if == NULL EventType means global ID, module specific else
  \param lParam = EventType

	\return PDBEventTypeDescriptor or NULL, if a type isn't found.
**/

#define MS_DB_EVENT_GETTYPE "DB/EventType/Get"



typedef uint32_t TDBEventHandle;


/**
	\brief this event was sent by the user. If not set this event was received.
**/
static const uint32_t DB_EF_SENT  = 0x00000002;

/**
	\brief event has been read by the user. It does not need to be processed any more except for history.
**/
static const uint32_t DB_EF_READ  = 0x00000004;

/**
	\brief event contains the right-to-left aligned text
**/
static const uint32_t DB_EF_RTL   = 0x00000008;

/**
	\brief event contains a text in utf-8
**/
static const uint32_t DB_EF_UTF   = 0x00000010;

/**
	\brief event belongs to more than one contact, so it started to count refernce instead of remembering its initial contact
**/
static const uint32_t DB_EF_REFERENCECOUNTING  = 0x80000000;

/**
	\brief event is virtual. it is not stored to db file yet.
**/
static const uint32_t DB_EF_VIRTUAL   = 0x00000020;

/**
	\brief describes an event
**/
typedef struct TDBEvent {
	uint32_t    cbSize;     /// size of the structure in bytes
	char *			ModuleName; /// 
	uint32_t    Timestamp;  /// seconds since 00:00, 01/01/1970. Gives us times until 2106 unless you use the standard C library which is signed and can only do until 2038. In GMT.
	uint32_t    Flags;	    /// the omnipresent flags	
	uint32_t    EventType;  /// module-unique event type ID
	uint32_t    cbBlob;	    /// size of pBlob in bytes
	uint8_t  *  pBlob;	      /// pointer to buffer containing module-defined event data
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
  \param wParam = hContact
  \param lParam = hEvent

	\return 0 on success
**/
#define MS_DB_EVENT_DELETE "DB/Event/Delete"

/**
	\brief Creates a new Event
  \param wParam = hContact
  \param lParam = PDBEvent

	\return hEvent on success, 0 otherwise
**/

#define MS_DB_EVENT_ADD "DB/Event/Add"


/**
	\brief Changes the flags for an event to mark it as read.
  \param wParam = hContact
  \param lParam = hEvent

	\return New flags
**/
#define MS_DB_EVENT_MARKREAD "DB/Event/MarkRead"

/**
	\brief Saves a virtual event to file and changes the flags.
  \param wParam = hContact
  \param lParam = hEvent

	\return New flags
**/
#define MS_DB_EVENT_WRITETODISK  "DB/Event/WriteToDisk"

/**
	\brief Retrieves a handle to a contact that owns hEvent.
  \param wParam = hEvent
  \param lParam = 0

	\return NULL is a valid return value, meaning, as usual, the user.
					DB_INVALIDPARAM if hDbEvent is invalid, or the handle to the contact on
					success
**/
#define MS_DB_EVENT_GETCONTACT  "DB/Event/GetContact"

/**
	\brief options to create event hard link
**/
typedef 
	struct TDBEventHardLink {
		uint32_t cbSize;           /// size of the structure
		TDBEventHandle hEvent;     /// event to link
		TDBContactHandle hContact; /// contact to link to
		uint32_t Flags;            /// flags, only DB_EF_VIRTUAL yet
	} TDBEventHardLink, *PEventHardLink;

/**
	\brief Creates a hard linked event.

	This service adds an existing event to a specific contact.
  \param wParam = PEventHardLink
  \param lParam = 0

	\return 0 on success
**/
#define MS_DB_EVENT_CREATEHARDLINK  "DB/Event/CreateHardLink"


/**
	\brief Event Filter Options for event iteration
**/
typedef
	struct TDBEventIterFilter {
		uint32_t cbSize;										/// size in bytes of this structure
		uint32_t Options;										/// DB_CIFO_* flags
		TDBContactHandle hContact;					/// hContact which events should be iterated (or where iteration begins)
		uint32_t ExtraCount;								/// count of additional Contacts which settings are enumerated, size of the array pointed by ExtraContacts
		TDBContactHandle * ExtraContacts;   /// pointer to an array with additional Contact handles in prioritized order

		uint32_t tSince;                    /// timestamp when to start iteration, 0 for first item
		uint32_t tTill;                     /// timestamp when to stop iteration, 0 for last item

		PDBEvent Event;	                    /// if set every events data gets stored there

	} TDBEventIterFilter, *PDBEventIterFilter;


/**
	\brief Handle of a Event-Iteration
**/
typedef uint32_t TDBEventIterationHandle;
/**
	\brief initialize an iteration of events
	\param wParam = PDBEventIterFilter
	\param lParam = 0

	\return EnumID
**/
#define MS_DB_EVENT_ITER_INIT "DB/Event/Iter/Init"


/**
	\brief get the next event
	\param wParam = EnumID returned by MS_DB_EVENT_ITER_INIT
	\param lParam = 0

	\return hSetting, 0 at the end
**/
#define MS_DB_EVENT_ITER_NEXT "DB/Event/Iter/Next"

/**
	\brief closes an iteration and frees its ressourcs
	\param wParam = IterationHandle returned by MS_DB_EVENT_ITER_INIT
	\param lParam = 0

	\return 0 on success
**/
#define MS_DB_EVENT_ITER_CLOSE "DB/Event/Iter/Close"



#pragma pack(pop)

#endif
