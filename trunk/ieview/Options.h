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

#define DBS_BASICFLAGS  		  "GeneralFlags"
#define DBS_SRMMFLAGS		  	  "SRMMFlags"

#define DBS_BACKGROUNDIMAGEFILE   "BackgroundImageFile"
#define DBS_EXTERNALCSSFILE       "ExternalCSSFile"
#define DBS_EXTERNALCSSFILE_RTL   "ExternalCSSFileRTL"
#define DBS_TEMPLATESFILE         "TemplatesFile"
#define DBS_TEMPLATESFILE_RTL     "TemplatesFileRTL"

#define DBS_GROUPCHATFLAGS        "GroupChatFlags"
#define DBS_GROUPCHATCSSFILE      "GroupChatCSSFile"
#define DBS_GROUPCHATTEMPLATESFILE   "GroupChatTemplatesFile"

extern int IEViewOptInit(WPARAM wParam, LPARAM lParam);

class Options {
private:
   	static int 		generalFlags;
   	static char *	bkgFilename;
   	static char *	externalCSSFilename;
   	static char *	externalCSSFilenameRTL;

   	static char *	templatesFilename;
   	static char *	templatesFilenameRTL;
   	static int 		srmmFlags;

   	static int 		groupChatFlags;
   	static char *	groupChatCSSFilename;
   	static char *	groupChatTemplatesFilename;

	static bool     bMathModule;
   	static bool     isInited;
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

		LOG_SHOW_FILE         = 0x20,
		LOG_SHOW_URL          = 0x40,
		LOG_SHOW_STATUSCHANGE = 0x80,
		LOG_SHOW_NICKNAMES    = 0x0100,
		LOG_SHOW_TIME         = 0x0200,
		LOG_SHOW_DATE         = 0x0400,
		LOG_SHOW_SECONDS      = 0x0800,
		LOG_LONG_DATE         = 0x1000,
		LOG_RELATIVE_DATE     = 0x2000,
		LOG_GROUP_MESSAGES	  = 0x4000,


	};
   	static void     		setGeneralFlags(int flags);
   	static int				getGeneralFlags();
   	static void     		setBkgImageFile(const char *filename);
   	static const char *		getBkgImageFile();
   	static void      		setExternalCSSFile(const char *filename);
   	static const char *		getExternalCSSFile();
   	static void      		setExternalCSSFileRTL(const char *filename);
   	static const char *		getExternalCSSFileRTL();
   	static void     		setTemplatesFileRTL(const char *filename);
   	static void     		setTemplatesFile(const char *filename);
   	static const char *		getTemplatesFile();
   	static const char *		getTemplatesFileRTL();
   	static void				setSRMMFlags(int flags);
   	static int				getSRMMFlags();
   	static void      		setGroupChatCSSFile(const char *filename);
   	static const char *		getGroupChatCSSFile();
   	static void				setGroupChatFlags(int flags);
   	static int				getGroupChatFlags();
   	static void     		setGroupChatTemplatesFile(const char *filename);
   	static const char *		getGroupChatTemplatesFile();
   	static bool             isMathModule();
   	static void      		init();
};

#endif
