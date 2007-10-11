#include "Settings.h"


CSettings::CSettings(CFileAccess & FileAccess, unsigned int RootNode)
: CFileBTree(FileAccess, RootNode)
{

}

CSettings::~CSettings()
{

}
