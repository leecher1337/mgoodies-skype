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

#define DBS_BACKGROUNDIMAGEFILE   "BackgroundImageFile"
#define DBS_BACKGROUNDIMAGEFLAGS  "BackgroundImageFlags"

#define DBS_EXTERNALCSSFILE       "ExternalCSSFile"
#define DBS_EXTERNALCSSFILE_RTL   "ExternalCSSFileRTL"
#define DBS_EXTERNALCSSFLAGS	  "ExternalCSSFlags"

#define DBS_TEMPLATESFILE         "TemplatesFile"
#define DBS_TEMPLATESFILE_RTL     "TemplatesFileRTL"
#define DBS_TEMPLATESFLAGS		  "TemplatesFlags"

#define DBS_SMILEYSFLAGS  		  "SmileyFlags"

extern int IEViewOptInit(WPARAM wParam, LPARAM lParam);

class Options {
private:
   	static char *	bkgFilename;
   	static int 		bkgFlags;
   	static int 		smileyFlags;
   	static char *	externalCSSFilename;
   	static char *	externalCSSFilenameRTL;
   	static int 		externalCSSFlags;
   	
   	static char *	templatesFilename;
   	static char *	templatesFilenameRTL;
   	static int 		templatesFlags;
public:
	enum OPTIONS {
		BKGIMAGE_ENABLED        = 1,
		BKGIMAGE_SCROLL         = 2,

		EXTERNALCSS_ENABLED		= 1,

        SMILEY_ENABLED          = 1,
		SMILEY_ISOLATED         = 2,
		SMILEY_SURROUND      	= 4,
		SMILEY_PROTOCOLS        = 8,
		SMILEY_SMILEYADD        = 0x100,

		TEMPLATES_ENABLED		= 1,
		
		LOG_SHOW_FILE           = 2,
		LOG_SHOW_URL            = 4,
		LOG_SHOW_STATUSCHANGE   = 8,

		LOG_SHOW_NICKNAMES      = 0x0100,
		LOG_SHOW_TIME           = 0x0200,
		LOG_SHOW_DATE           = 0x0400,
		LOG_SHOW_SECONDS        = 0x0800,
		LOG_LONG_DATE       	= 0x1000,
		LOG_RELATIVE_DATE       = 0x2000,

		LOG_GROUP_MESSAGES		= 0x4000,

	};
   	static void     		setSmileyFile(const char *proto, const char *filename);
   	static const char *		getSmileyFile(const char *proto);
   	static void     		setSmileyFlags(int flags);
   	static int				getSmileyFlags();
   	static void     		setBkgImageFile(const char *filename);
   	static const char *		getBkgImageFile();
   	static void				setBkgImageFlags(int flags);
   	static int				getBkgImageFlags();
   	static void      		setExternalCSSFile(const char *filename);
   	static const char *		getExternalCSSFile();
   	static void      		setExternalCSSFileRTL(const char *filename);
   	static const char *		getExternalCSSFileRTL();
   	static void				setExternalCSSFlags(int flags);
   	static int				getExternalCSSFlags();
   	static void     		setTemplatesFileRTL(const char *filename);
   	static void     		setTemplatesFile(const char *filename);
   	static const char *		getTemplatesFile();
   	static const char *		getTemplatesFileRTL();
   	static void				setTemplatesFlags(int flags);
   	static int				getTemplatesFlags();
   	static void      		init();
};

#endif
