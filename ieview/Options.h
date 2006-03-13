/*

IEView Plugin for Miranda IM
Copyright (C) 2005  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#ifndef OPTIONS_INCLUDED
#define OPTIONS_INCLUDED
//#include "FontList.h"
#include "ieview_common.h"

#define DBS_BASICFLAGS  		  	"GeneralFlags"

#define DBS_SRMMFLAGS		  	  	"SRMMFlags"
#define DBS_BACKGROUNDIMAGEFILE   	"BackgroundImageFile"
#define DBS_EXTERNALCSSFILE       	"ExternalCSSFile"
#define DBS_EXTERNALCSSFILE_RTL   	"ExternalCSSFileRTL"
#define DBS_TEMPLATESFILE         	"TemplatesFile"
#define DBS_TEMPLATESFILE_RTL     	"TemplatesFileRTL"

#define DBS_GROUPCHATFLAGS        	"GroupChatFlags"
#define DBS_GROUPCHATCSSFILE      	"GroupChatCSSFile"
#define DBS_GROUPCHATTEMPLATESFILE  "GroupChatTemplatesFile"

#define DBS_HISTORYFLAGS		  	 "HistoryFlags"
#define DBS_HISTORYCSSFILE      	 "HistoryCSSFile"
#define DBS_HISTORYCSSFILE_RTL     	 "HistoryCSSFileRTL"
#define DBS_HISTORYTEMPLATESFILE  	 "HistoryTemplatesFile"
#define DBS_HISTORYTEMPLATESFILE_RTL "HistoryTemplatesFileRTL"


extern int IEViewOptInit(WPARAM wParam, LPARAM lParam);

class ProtocolOptions {
private:
	char *cssFilename;
	char *cssFilenameRtl;
	char *templateFilename;
	char *templateFilenameRtl;
}

class Options {
private:
   	static int 		generalFlags;
   	static char *	bkgFilename;
   	static char *	srmmCSSFilename;
   	static char *	srmmCSSFilenameRTL;

   	static char *	srmmTemplatesFilename;
   	static char *	srmmTemplatesFilenameRTL;
   	static int 		srmmFlags;

   	static int 		groupChatFlags;
   	static char *	groupChatCSSFilename;
   	static char *	groupChatTemplatesFilename;

   	static int 		historyFlags;
   	static char *	historyTemplatesFilename;
   	static char *	historyTemplatesFilenameRTL;
   	static char *	historyCSSFilename;
   	static char *	historyCSSFilenameRTL;

   	static bool     isInited;
	static bool     bMathModule;
	static bool     bSmileyAdd;
	static int      avatarServiceFlags;
public:
	enum OPTIONS {
		GENERAL_ENABLE_BBCODES	= 1,
		GENERAL_ENABLE_MATHMODULE = 2,
		GENERAL_ENABLE_FLASH = 4,
		GENERAL_ENABLE_PNGHACK = 8,
		GENERAL_SMILEYINNAMES  = 16,

		IMAGE_ENABLED         = 1,
		IMAGE_SCROLL          = 2,
		CSS_ENABLED  		  = 4,
		TEMPLATES_ENABLED	  = 8,

		LOG_SHOW_NICKNAMES    = 0x0100,
		LOG_SHOW_TIME         = 0x0200,
		LOG_SHOW_DATE         = 0x0400,
		LOG_SHOW_SECONDS      = 0x0800,
		LOG_LONG_DATE         = 0x1000,
		LOG_RELATIVE_DATE     = 0x2000,
		LOG_GROUP_MESSAGES	  = 0x4000,
	};
	enum AVATARSERVICEFLAGS {
		AVATARSERVICE_PRESENT     = 0x0001,
	};
   	static void     		setGeneralFlags(int flags);
   	static int				getGeneralFlags();
   	static void     		setBkgImageFile(const char *filename);
   	static const char *		getBkgImageFile();
   	static void      		setSRMMCSSFile(const char *filename);
   	static const char *		getSRMMCSSFile();
   	static void      		setSRMMCSSFileRTL(const char *filename);
   	static const char *		getSRMMCSSFileRTL();
   	static void     		setSRMMTemplatesFileRTL(const char *filename);
   	static void     		setSRMMTemplatesFile(const char *filename);
   	static const char *		getSRMMTemplatesFile();
   	static const char *		getSRMMTemplatesFileRTL();
   	static void				setSRMMFlags(int flags);
   	static int				getSRMMFlags();

   	static void      		setGroupChatCSSFile(const char *filename);
   	static const char *		getGroupChatCSSFile();
   	static void				setGroupChatFlags(int flags);
   	static int				getGroupChatFlags();
   	static void     		setGroupChatTemplatesFile(const char *filename);
   	static const char *		getGroupChatTemplatesFile();

   	static void				setHistoryFlags(int flags);
   	static int				getHistoryFlags();
   	static void      		setHistoryCSSFile(const char *filename);
   	static const char *		getHistoryCSSFile();
   	static void      		setHistoryCSSFileRTL(const char *filename);
   	static const char *		getHistoryCSSFileRTL();
   	static void     		setHistoryTemplatesFileRTL(const char *filename);
   	static void     		setHistoryTemplatesFile(const char *filename);
   	static const char *		getHistoryTemplatesFile();
   	static const char *		getHistoryTemplatesFileRTL();

   	static bool             isMathModule();
   	static bool             isSmileyAdd();
   	static int				getAvatarServiceFlags();
   	static void      		init();
};

#endif
