#ifndef NUDGE_H
#define NUDGE_H

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