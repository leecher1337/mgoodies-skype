#include "Entries.h"


CEntries::CEntries(CFileAccess & FileAccess, unsigned int RootNode)
: CFileBTree(FileAccess, RootNode)
{

}

CEntries::~CEntries()
{

}
