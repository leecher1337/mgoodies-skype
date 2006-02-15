struct CNudgeElement
{
	char ProtocolName[64];
	char NudgeSoundname[100];
	bool showPopup;
	bool showEvent;
	COLORREF back;
	COLORREF text;
	int popupTimeSec;
	CNudgeElement *next;
} CNUDGEELEMENT;