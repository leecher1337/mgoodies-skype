#include "Settings.h"

__forceinline bool TSettingKey::operator <  (const TSettingKey & Other) const
{
	if (ModuleHash != Other.ModuleHash) return ModuleHash < Other.ModuleHash;
	if (SettingHash != Other.SettingHash) return SettingHash < Other.SettingHash;
	return false;
}

__forceinline bool TSettingKey::operator == (const TSettingKey & Other) const
{
	return (ModuleHash == Other.ModuleHash) && (SettingHash == Other.SettingHash);
}

__forceinline bool TSettingKey::operator >  (const TSettingKey & Other) const
{	
	if (ModuleHash != Other.ModuleHash) return ModuleHash > Other.ModuleHash;
	if (SettingHash != Other.SettingHash) return SettingHash > Other.SettingHash;
	return false;
}

CSettings::CSettings(CFileAccess & FileAccess, CMultiReadExclusiveWriteSynchronizer & Synchronize, unsigned int RootNode)
: CFileBTree(FileAccess, RootNode),
	m_Sync(Synchronize)
{

}

CSettings::~CSettings()
{

}
