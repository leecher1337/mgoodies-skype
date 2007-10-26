#include "DatabaseLink.h"

/*
returns what the driver can do given the flag
*/
static int getCapability(int flag)
{
	return 0;
}

/*
	buf: pointer to a string buffer
	cch: length of buffer
	shortName: if true, the driver should return a short but descriptive name, e.g. "3.xx profile"
	Affect: The database plugin must return a "friendly name" into buf and not exceed cch bytes,
	e.g. "Database driver for 3.xx profiles"
	Returns: 0 on success, non zero on failure
*/

static int getFriendlyName(char* buf, size_t cch, int shortName)
{
	if (shortName)
		strncpy_s(buf, cch, "dbx tree", 8);
	else
		strncpy_s(buf, cch, "Miranda tree database driver", 28);
	return 0;
}

/*
	profile: pointer to a string which contains full path + name
	Affect: The database plugin should create the profile, the filepath will not exist at
		the time of this call, profile will be C:\..\<name>.dat
	Note: Do not prompt the user in anyway about this operation.
	Note: Do not initialise internal data structures at this point!
	Returns: 0 on success, non zero on failure - error contains extended error information, see EMKPRF_*
*/
static int makeDatabase(char* profile, int* error)
{
	if (gDataBase) delete gDataBase;
  gDataBase = new CDataBase(profile);

	*error = gDataBase->CreateDB();
	return *error;
}

/*
	profile: [in] a null terminated string to file path of selected profile
	error: [in/out] pointer to an int to set with error if any
	Affect: Ask the database plugin if it supports the given profile, if it does it will
		return 0, if it doesnt return 1, with the error set in error -- EGROKPRF_* can be valid error
		condition, most common error would be [EGROKPRF_UNKHEADER]
	Note: Just because 1 is returned, doesnt mean the profile is not supported, the profile might be damaged
		etc.
	Returns: 0 on success, non zero on failure
*/
static int grokHeader(char* profile, int* error)
{
	if (gDataBase) delete gDataBase;
	gDataBase = new CDataBase(profile);

	*error = gDataBase->CheckDB();	
	return *error;
}

/*
Affect: Tell the database to create all services/hooks that a 3.xx legecy database might support into link,
	which is a PLUGINLINK structure
Returns: 0 on success, nonzero on failure
*/
static int Load(char* profile, void* link)
{
	if (gDataBase) delete gDataBase;
	gDataBase = new CDataBase(profile);

	gPluginLink = (PLUGINLINK*)link;

	RegisterServices();

	return gDataBase->OpenDB();
}

/*
Affect: The database plugin should shutdown, unloading things from the core and freeing internal structures
Returns: 0 on success, nonzero on failure
Note: Unload() might be called even if Load() was never called, wasLoaded is set to 1 if Load() was ever called.
*/
static int Unload(int wasLoaded)
{
	if (gDataBase)
		delete gDataBase;

	gDataBase = NULL;
	return 0;
}


DATABASELINK gDBLink = {
	sizeof(DATABASELINK),
	getCapability,
	getFriendlyName,
	makeDatabase,
	grokHeader,
	Load,
	Unload,
};

PLUGINLINK *gPluginLink = NULL;