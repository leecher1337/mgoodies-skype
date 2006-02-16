#ifndef NUDGE_H
#define NUDGE_H

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
	unsigned int popupBackColor;
	unsigned int popupTextColor;
	int popupTimeSec;
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