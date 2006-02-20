#ifndef SHAKE_H
#define SHAKE_H

class CShake
{
public:
	bool Shaking;
	bool ShakingChat;
	int nScaleClist;
	int nScaleChat;
	int nMoveClist;
	int nMoveChat;

	void Load(void);
	void Save(void);
	int ShakeClist(HWND hWnd);
	int ShakeChat(HWND hWnd);
};

int ShakeClist(WPARAM,LPARAM);
int ShakeChat(WPARAM,LPARAM);
int TriggerShakeChat(WPARAM,LPARAM);
int TriggerShakeClist(WPARAM,LPARAM);

#endif