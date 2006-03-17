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
class ProtocolSettings;
class Options;
#ifndef OPTIONS_INCLUDED
#define OPTIONS_INCLUDED
//#include "FontList.h"
#include "ieview_common.h"

#define DBS_BASICFLAGS  		  	"GeneralFlags"

#define DBS_SRMMMODE		  	  	"SRMMMode"
#define DBS_SRMMFLAGS		  	  	"SRMMFlags"
#define DBS_BACKGROUNDIMAGEFILE   	"BackgroundImageFile"

#define DBS_GROUPCHATFLAGS        	"GroupChatFlags"
#define DBS_GROUPCHATCSSFILE      	"GroupChatCSSFile"
#define DBS_GROUPCHATTEMPLATESFILE  "GroupChatTemplatesFile"

#define DBS_HISTORYMODE		  	 	 "HistoryMode"
#define DBS_HISTORYFLAGS		  	 "HistoryFlags"
#define DBS_HISTORYCSSFILE      	 "HistoryCSSFile"
#define DBS_HISTORYCSSFILE_RTL     	 "HistoryCSSFileRTL"
#define DBS_HISTORYTEMPLATESFILE  	 "HistoryTemplatesFile"
#define DBS_HISTORYTEMPLATESFILE_RTL "HistoryTemplatesFileRTL"


#define DBS_SRMM_ENABLE          	"SRMMEnable"
#define DBS_SRMM_FLAGS          	"SRMMFlags"
#define DBS_SRMM_BACKGROUND    		"SRMMBackgroundFile"
#define DBS_SRMM_CSS         		"SRMMCSSFile"
#define DBS_SRMM_CSS_RTL     		"SRMMCSSFileRTL"
#define DBS_SRMM_TEMPLATE         	"SRMMTemplateFile"
#define DBS_SRMM_TEMPLATE_RTL     	"SRMMTemplateFileRTL"


extern int IEViewOptInit(WPARAM wParam, LPARAM lParam);

class ProtocolSettings {
private:
	char *protocolName;
	ProtocolSettings *next;

	bool enable;
	int srmmFlags;
	char *srmmBackgroundFilename;
	char *srmmCssFilename;
	char *srmmCssFilenameRtl;
	char *srmmTemplateFilename;
	char *srmmTemplateFilenameRtl;

	bool enableTemp;
	int srmmFlagsTemp;
	char *srmmBackgroundFilenameTemp;
	char *srmmCssFilenameTemp;
	char *srmmCssFilenameRtlTemp;
	char *srmmTemplateFilenameTemp;
	char *srmmTemplateFilenameRtlTemp;

public:
	ProtocolSettings(const char *protocolName);
	~ProtocolSettings();
	void	setNext(ProtocolSettings *next);
	const char *getProtocolName();
	ProtocolSettings *getNext();
	void	setEnable(bool enable);
	bool	isEnable();
	void	setSRMMFlags(int flags);
	int		getSRMMFlags();
	void	setSRMMBackgroundFilename(const char *filename);
	const char *getSRMMBackgroundFilename();
	void	setSRMMCssFilename(const char *filename);
	const char *getSRMMCssFilename();
	void	setSRMMCssFilenameRtl(const char *filename);
	const char *getSRMMCssFilenameRtl();
	void	setSRMMTemplateFilename(const char *filename);
	const char *getSRMMTemplateFilename();
	void	setSRMMTemplateFilenameRtl(const char *filename);
	const char *getSRMMTemplateFilenameRtl();

	void	setEnableTemp(bool enable);
	bool	isEnableTemp();
	void	setSRMMFlagsTemp(int flags);
	int		getSRMMFlagsTemp();
	void	setSRMMBackgroundFilenameTemp(const char *filename);
	const char *getSRMMBackgroundFilenameTemp();
	void	setSRMMCssFilenameTemp(const char *filename);
	const char *getSRMMCssFilenameTemp();
	void	setSRMMCssFilenameRtlTemp(const char *filename);
	const char *getSRMMCssFilenameRtlTemp();
	void	setSRMMTemplateFilenameTemp(const char *filename);
	const char *getSRMMTemplateFilenameTemp();
	void	setSRMMTemplateFilenameRtlTemp(const char *filename);
	const char *getSRMMTemplateFilenameRtlTemp();


	void 	copyToTemp();
	void	copyFromTemp();

};

class Options {
private:
   	static int 		generalFlags;

   	static int 		srmmFlags;
   	static int		srmmMode;

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
	static ProtocolSettings* protocolList;

public:
	enum MODES {
		MODE_COMPATIBLE = 0,
		MODE_CSS = 1,
		MODE_TEMPLATE = 2
	};
	enum OPTIONS {
		GENERAL_ENABLE_BBCODES	= 0x000001,
		GENERAL_ENABLE_MATHMODULE = 0x000002,
		GENERAL_ENABLE_FLASH 	= 0x000004,
		GENERAL_ENABLE_PNGHACK 	= 0x000008,
		GENERAL_SMILEYINNAMES  	= 0x000010,

		LOG_SHOW_NICKNAMES    	= 0x000100,
		LOG_SHOW_TIME         	= 0x000200,
		LOG_SHOW_DATE         	= 0x000400,
		LOG_SHOW_SECONDS      	= 0x000800,
		LOG_LONG_DATE         	= 0x001000,
		LOG_RELATIVE_DATE     	= 0x002000,
		LOG_GROUP_MESSAGES	  	= 0x004000,

		LOG_IMAGE_ENABLED		= 0x010000,
		LOG_IMAGE_SCROLL        = 0x020000

	};
	enum AVATARSERVICEFLAGS {
		AVATARSERVICE_PRESENT     = 0x0001,
	};
   	static void     		setGeneralFlags(int flags);
   	static int				getGeneralFlags();
   	static void				setSRMMMode(int mode);
   	static int				getSRMMMode();
   	static void				setSRMMFlags(int flags);
//   	static int				getSRMMFlags();

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
	static void				saveProtocolSettings();
	static void				resetProtocolSettings();
   	static ProtocolSettings*getProtocolSettings();
   	static ProtocolSettings*getProtocolSettings(const char *protocolName);
};

#endif
