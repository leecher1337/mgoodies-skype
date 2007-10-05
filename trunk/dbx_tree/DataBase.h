#pragma once

#include "Events.h"
#include "Settings.h"
#include "Virtuals.h"
#include "Entries.h"

#include "FileAccess.h"
#include "MappedMemory.h"
#include "DirectAccess.h"




#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct TSettingsHeader {
	char Signature[20];
	unsigned int Version;
	unsigned int Settings;
	unsigned int FileSize;
	unsigned int WastedBytes;
	char Reserved[92];
} TSettingsHeader;



typedef struct TDataHeader {
	char Signature[20];
	unsigned int Version;
	unsigned int RootEntry;
	unsigned int Entries;
	unsigned int Virtuals;
	unsigned int FileSize;
	unsigned int WastedBytes;
	unsigned int EventIndex;
	char Reserved[80];
} TDataHeader;

#pragma pack(pop)


class CDataBase
{
private:

protected:

public:
	CDataBase(char* FileName);
	~CDataBase();
};

