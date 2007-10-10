#pragma once
#include "FileBTree.h"

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

/**
	\brief Key Type of the SettingsBTree

	The setting names (UTF8) are hashed, so that they can easily be accessed.
	The settings are grouped by the module hash, where they are stored.
	That is for enumeration of the settings of a module.
**/
typedef struct TSettingKey {
	unsigned short ModuleHash;  /// 16 bit hash of the Module name
	unsigned short SettingHash; /// 16 bit hash of the Setting name

	bool operator <  (const TSettingKey & Other);
	//bool operator <= (const TSettingKey & Other);
	bool operator == (const TSettingKey & Other);
	//bool operator >= (const TSettingKey & Other);
	//bool operator >  (const TSettingKey & Other);
} TSettingKey;


/**
	\brief The data of a setting

	A setting's data is variable length. The data is a TSetting-structure followed by varaible length data.
	- fixed data
	- ModuleName (UTF8)
	- SettingName (UTF8)
	- maybe blob data
**/
typedef struct TSetting {
	unsigned int Signature;    /// Signature
	unsigned short Flags;      /// flags
	unsigned short Type;       /// setting type
	unsigned short ModuleLen;  /// modulename length
	unsigned short SettingLen; /// settingname length
	union {
		unsigned int Data;       /// if type is fixed length, the data is stored rigth here
		unsigned int BlobLen;    /// if type is variable length this describes the length of the data in bytes
	};
} TSetting;

#pragma pack(pop)

/**
	\brief Manages the Settings in the Database
**/
class CSettings :	private CFileBTree<TSettingKey, unsigned int, 8, false>
{
private:

protected:
	virtual void RootChanged();

public:
	CSettings(CFileAccess & FileAccess);
	virtual ~CSettings();
};

