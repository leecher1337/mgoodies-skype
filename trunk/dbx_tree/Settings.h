#pragma once
#include "FileBTree.h"

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct TSettingKey {
	unsigned short ModuleHash;
	unsigned short SettingHash;

	bool operator <  (const TSettingKey & Other);
	//bool operator <= (const TSettingKey & Other);
	bool operator == (const TSettingKey & Other);
	//bool operator >= (const TSettingKey & Other);
	//bool operator >  (const TSettingKey & Other);
} TSettingKey;



typedef struct TSetting {
	unsigned int Signature;
	unsigned short Flags;
	unsigned short Type;
	unsigned short ModuleLen;
	unsigned short SettingLen;
	union {
		unsigned int Data;
		unsigned int BlobLen;
	};
} TSetting;

#pragma pack(pop)

class CSettings :	private CFileBTree<TSettingKey, unsigned int, 4, false>
{
private:

protected:
	virtual void RootChanged();

public:
	CSettings(CFileAccess & FileAccess);
	~CSettings();
};

