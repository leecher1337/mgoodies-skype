#ifndef NUDGE_H
#define NUDGE_H

//	NUDGE account status flags
#define	NUDGE_ACC_ST0	0x00000001		//Check (countdown) when Offline
#define	NUDGE_ACC_ST1	0x00000002		//Check (countdown) when Online
#define	NUDGE_ACC_ST2	0x00000004		//Check (countdown) when Away
#define	NUDGE_ACC_ST3	0x00000008		//Check (countdown) when N/A
#define	NUDGE_ACC_ST4	0x00000010		//Check (countdown) when Occupied
#define NUDGE_ACC_ST5	0x00000020		//Check (countdown) when DND
#define NUDGE_ACC_ST6	0x00000040		//Check (countdown) when Free for chat
#define NUDGE_ACC_ST7   0x00000080		//Check (countdown) when Invisible
#define NUDGE_ACC_ST8   0x00000100		//Check (countdown) when On the phone
#define NUDGE_ACC_ST9   0x00000200		//Check (countdown) when Out to lunch

class CNudge
{
public:
	bool useByProtocol;

	void Load(void);
	void Save(void);
};

class CNudgeElement
{
public:
	char ProtocolName[64];
	char NudgeSoundname[100];
	bool showPopup;
	bool showEvent;
	bool popupWindowColor;
	bool shakeClist;
	bool shakeChat;
	bool enabled;
	DWORD statusFlags;
	unsigned int popupBackColor;
	unsigned int popupTextColor;
	int popupTimeSec;
	int iProtoNumber;
	HICON hIcon;

	void Load(void);
	void Save(void);
};

typedef struct NudgeElementList
{
	CNudgeElement item;
	NudgeElementList *next;
} NUDGEELEMENTLIST;

#endif