#ifndef __M_CUSTOMSMILEYS_H__
# define __M_CUSTOMSMILEYS_H__


#define CUSTOMSMILEY_STATE_RECEIVED		1
#define CUSTOMSMILEY_STATE_DOWNLOADED	2
#define CUSTOMSMILEY_UNICODE			0x100

#ifdef UNICODE
# define CUSTOMSMILEY_TCHAR CUSTOMSMILEY_UNICODE
#else
# define CUSTOMSMILEY_TCHAR 0
#endif

typedef struct
{
	int cbSize;
	HANDLE hContact;
	union {
		const char *pszText;		// Valid only during the notification. Optional if CUSTOMSMILEY_STATE_DOWNLOADED
		const TCHAR *ptszText;		// Valid only during the notification. Optional if CUSTOMSMILEY_STATE_DOWNLOADED
		const WCHAR *pwszText;		// Valid only during the notification. Optional if CUSTOMSMILEY_STATE_DOWNLOADED
	};
	const char *pszFilename;		// Valid only during the notification
	int flags;						// One of CUSTOMSMILEY_STATE_*
	BOOL download;					// "Return" value. Someone have to change it to TRUE for it to be downloaded
} CUSTOMSMILEY;


// Fired when a custom smiley is received from a contact.
// This can is fired 2 times: 
// 1. When received the text, with flag CUSTOMSMILEY_STATE_RECEIVED. If someone hooks this message and
// wants the custom smiley, it has to change the download field to TRUE
// 2. If needed, when the image was downloaded, with flag CUSTOMSMILEY_STATE_DOWNLOADED
// If the protocol receives the smiley text and image at the same time, it can fire it only once, with
// flag (CUSTOMSMILEY_STATE_RECEIVED | CUSTOMSMILEY_STATE_DOWNLOADED)
// 
// wParam = 0
// lParam = CUSTOMSMILEY *
#define ME_CUSTOMSMILEY_RECEIVED "/CustomSmileyReceived"











#endif __M_CUSTOMSMILEYS_H__
