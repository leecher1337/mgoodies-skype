#ifndef __MAIN_H
#define __MAIN_H

#ifdef __GNUC__
	#define __try
	#define __except(x) if (0) /* don't execute handler */
	#define __finally
	#define _try __try
	#define _except __except
	#define _finally __finally 
#endif
//For updater
//#define YAMN_9x
#ifdef YAMN_9x
	#define YAMN_SHORTNAME "YAMN tweety win9x"
	#define YAMN_FILENAME "yamn_9x"
#else
	#define YAMN_SHORTNAME "YAMN tweety"
	#define YAMN_FILENAME "yamn"
#endif

#define	YAMN_VERSION			PLUGIN_MAKE_VERSION(0,0,1,7) 	//ok but do not forget rewrite version for debug release in debug.cpp and POP3 filter in pop3comm.cpp
#define	YAMN_VERSION_C			"0.0.1.6"
#define YAMN_NEWMAILSNDDESC		"YAMN: new mail"
#define YAMN_CONNECTFAILSNDDESC	"YAMN: connect failed"
#define	YAMN_CONNECTFAILSOUND	"YAMN/Sound/ConnectFail"
#define	YAMN_NEWMAILSOUND		"YAMN/Sound/NewMail"

#define YAMN_DBMODULE		"YAMN"
#define YAMN_DBPOSX			"MailBrowserWinX"
#define YAMN_DBPOSY			"MailBrowserWinY"
#define YAMN_DBSIZEX		"MailBrowserWinW"
#define YAMN_DBSIZEY		"MailBrowserWinH"
#define YAMN_HKCHECKMAIL	"HKCheckMail"
#define	YAMN_TTBFCHECK		"ForceCheckTTB"

#define YAMN_DEFAULTHK		MAKEWORD(VK_F12,MOD_CONTROL)

#endif

