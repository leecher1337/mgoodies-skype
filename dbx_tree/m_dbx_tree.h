#ifndef M_DBX_TREE_H__

#define M_DBX_TREE_H__ 1

/**
	\brief 
**/
typedef unsigned int TEntryHandle;

/**
	\brief Entry isn't a Contact
**/
static const unsigned int DB_EF_IsGroup   = 0x00000001;
/**
	\brief Entry has Childs (Groups and Metacontacts)
**/
static const unsigned int DB_EF_HasChilds = 0x00000002;
/**
	\brief Entry is a Virtual Copy
**/
static const unsigned int DB_EF_IsVirtual = 0x00000004;

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


typedef
	struct TDBENTRYITERFILTER
	{
		unsigned int cbSize;
		unsigned int fHasFlags;
		unsigned int fDontHaveFlags;
		TEntryHandle hStartWith;		
	} TDBENTRYITERFILTER, *PDBENTRYITERFILTER;

/**
	\brief initialize an enumeration of entries in breadth first search style.
	
	Prefer this one over MS_DB_ENTRY_ITER_INITBF because it is faster
	\param wParam = PDBENTRYITERFILTER
	\param lParam = 0

	\return EnumID
**/
#define MS_DB_ENTRY_ITER_INITBF "DB/Entry/Iter/InitBF"

/**
	\brief initialize an enumeration of entries in depth first search style.
	
	Prefer MS_DB_ENTRY_ITER_INITBF because it is faster
	\param wParam = PDBENTRYITERFILTER
	\param lParam = 0

	\return EnumID
**/
#define MS_DB_ENTRY_ITER_INITDF "DB/Entry/Iter/InitDF"

/**
	\brief get the next entry
	\param wParam = EnumID returned by MS_DB_ENTRY_ITER_INIT*
	\param lParam = 0

	\return hEntry, 0 at the end
**/
#define MS_DB_ENTRY_ITER_NEXT "DB/Entry/Iter/Next"

/**
	\brief closes an enumeration and frees its ressourcs
	\param wParam = SearchID returned by MS_DB_ENTRY_ITER_INIT*
	\param lParam = 0

	\return 0 on success
**/
#define MS_DB_ENTRY_ITER_CLOSE "DB/Entry/Iter/Close"

/** 
	\brief 
  \param wParam = hEntry
  \param lParam = 0

	\return 0 on success
**/
#define MS_DB_ENTRY_DELETE  "DB/Entry/Delete"


/** 
	\brief 
  \param wParam = hParentEntry
  \param lParam = 0

	\return hEntry on success, 0 otherwise
**/
#define MS_DB_ENTRY_CREATE  "DB/Entry/Create"




///////////////////////////////////////////////////////////
// Virtual Entries
///////////////////////////////////////////////////////////

/** 
	\brief 
  \param wParam = hEntry to copy
  \param lParam = hParentEntry to place duplicate

	\return hEntry of created duplicate
**/
#define MS_DB_VIRTUALENTRY_CREATE  "DB/VirtualEntry/Create"

/** 
	\brief 
  \param wParam = hEntry of virtual entry
  \param lParam = 0

	\return hEntry of orginal contact
**/
#define MS_DB_VIRTUALENTRY_GETPARENT  "DB/VirtualEntry/GetParent"

/** 
	\brief 
  \param wParam = hEntry with virtual copies
  \param lParam

	\return hEntry of first virtual copy
**/
#define MS_DB_VIRTUALENTRY_GETFIRST  "DB/VirtualEntry/GetFirst"

/** 
	\brief 
  \param wParam = hEntry of virtual entry
  \param lParam = 0

	\return hEntry of next duplicate, 0 otherwise
**/
#define MS_DB_VIRTUALENTRY_GETNEXT  "DB/VirtualEntry/GetNext"





typedef unsigned int TEventHandle;

#endif