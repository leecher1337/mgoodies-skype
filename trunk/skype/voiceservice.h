#ifndef _VOICESERVICE_H_
#define _VOICESERVICE_H_

#include "sdk/m_voice.h"


BOOL HasVoiceService();
void VoiceServiceInit();
void VoiceServiceExit();
void VoiceServiceModulesLoaded();
void NofifyVoiceService(HANDLE hContact, char *callId, int state) ;



#endif // _VOICESERVICE_H_

