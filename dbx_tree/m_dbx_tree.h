

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
	\brief initialize an enumeration of entries in breadth first search style.
	
	Prefer this one over MS_DB_ENTRY_INITDFS because it is faster
	\param wParam = 0    // maybe some flags based filter struct?
	\param lParam = 0

	\return EnumID
**/
#define MS_DB_ENTRY_ENUM_INITBF "DB/Entry/Enum/InitBF"

/**
	\brief initialize an enumeration of entries in depth first search style.
	
	Prefer MS_DB_ENTRY_INITBFS because it is faster
	\param wParam = 0    // maybe some flags based filter struct?
	\param lParam = 0

	\return EnumID
**/
#define MS_DB_ENTRY_ENUM_INITDF "DB/Entry/Enum/InitDF"

/**
	\brief get the next entry
	\param wParam = EnumID returned by MS_DB_ENTRY_ENUM_INIT*
	\param lParam = 0

	\return hEntry, 0 at the end
**/
#define MS_DB_ENTRY_ENUM_NEXT "DB/Entry/Enum/Next"

/**
	\brief closes an enumeration and frees its ressourcs
	\param wParam = SearchID returned by MS_DB_ENTRY_ENUM_INIT*
	\param lParam = 0

	\return 0 on success
**/
#define MS_DB_ENTRY_ENUM_CLOSE "DB/Entry/Enum/Close"



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
#define MS_DB_ENTRY_ADD  "DB/Entry/Add"


/** 
	\brief 
  \param wParam = hEntry
  \param lParam = hParentEntry

	\return 0 on success
**/
#define MS_DB_ENTRY_SETPARENT  "DB/Entry/SetParent"


/** 
	\brief 
  \param wParam = hEntry to copy
  \param lParam = hParentEntry to place duplicate

	\return hEntry of created duplicate
**/
#define MS_DB_VENTRY_CREATE  "DB/VEntry/Create"

/** 
	\brief 
  \param wParam = hEntry of virtual entry
  \param lParam = 0

	\return hEntry of orginal contact
**/
#define MS_DB_VENTRY_GETPARENT  "DB/VEntry/GetParent"

/** 
	\brief 
  \param wParam = hEntry with virtual copies
  \param lParam

	\return hEntry of first virtual copy
**/
#define MS_DB_VENTRY_GETFIRST  "DB/VEntry/GetFirst"

/** 
	\brief 
  \param wParam = hEntry of virtual entry
  \param lParam = 0

	\return hEntry of next duplicate, 0 otherwise
**/
#define MS_DB_VENTRY_GETNEXT  "DB/VEntry/GetNext"



