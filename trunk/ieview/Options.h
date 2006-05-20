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

#define DBS_SRMM_ENABLE          	"SRMMEnable"
#define DBS_SRMM_MODE          		"SRMMMode"
#define DBS_SRMM_FLAGS          	"SRMMFlags"
#define DBS_SRMM_BACKGROUND    		"SRMMBackgroundFile"
#define DBS_SRMM_CSS         		"SRMMCSSFile"
#define DBS_SRMM_CSS_RTL     		"SRMMCSSFileRTL"
#define DBS_SRMM_TEMPLATE         	"SRMMTemplateFile"
#define DBS_SRMM_TEMPLATE_RTL     	"SRMMTemplateFileRTL"

#define DBS_CHAT_ENABLE          	"ChatEnable"
#define DBS_CHAT_MODE          		"ChatMode"
#define DBS_CHAT_FLAGS          	"ChatFlags"
#define DBS_CHAT_BACKGROUND    		"ChatBackgroundFile"
#define DBS_CHAT_CSS         		"ChatCSSFile"
#define DBS_CHAT_CSS_RTL     		"ChatCSSFileRTL"
#define DBS_CHAT_TEMPLATE         	"ChatTemplateFile"
#define DBS_CHAT_TEMPLATE_RTL     	"ChatTemplateFileRTL"

#define DBS_HISTORY_ENABLE          "HistoryEnable"
#define DBS_HISTORY_MODE          	"HistoryMode"
#define DBS_HISTORY_FLAGS          	"HistoryFlags"
#define DBS_HISTORY_BACKGROUND    	"HistoryBackgroundFile"
#define DBS_HISTORY_CSS         	"HistoryCSSFile"
#define DBS_HISTORY_CSS_RTL     	"HistoryCSSFileRTL"
#define DBS_HISTORY_TEMPLATE        "HistoryTemplateFile"
#define DBS_HISTORY_TEMPLATE_RTL    "HistoryTemplateFileRTL"

extern int IEViewOptInit(WPARAM wParam, LPARAM lParam);

class ProtocolSettings {
private:
	char *protocolName;
	ProtocolSettings *next;

	bool srmmEnable;
	int srmmMode;
	int srmmFlags;
	char *srmmBackgroundFilename;
	char *srmmCssFilename;
	char *srmmCssFilenameRtl;
	char *srmmTemplateFilename;
	char *srmmTemplateFilenameRtl;

	bool srmmEnableTemp;
	int srmmModeTemp;
	int srmmFlagsTemp;
	char *srmmBackgroundFilenameTemp;
	char *srmmCssFilenameTemp;
	char *srmmCssFilenameRtlTemp;
	char *srmmTemplateFilenameTemp;
	char *srmmTemplateFilenameRtlTemp;

	bool chatEnable;
	int chatMode;
	int chatFlags;
	char *chatBackgroundFilename;
	char *chatCssFilename;
	char *chatCssFilenameRtl;
	char *chatTemplateFilename;
	char *chatTemplateFilenameRtl;

	bool chatEnableTemp;
	int chatModeTemp;
	int chatFlagsTemp;
	char *chatBackgroundFilenameTemp;
	char *chatCssFilenameTemp;
	char *chatCssFilenameRtlTemp;
	char *chatTemplateFilenameTemp;
	char *chatTemplateFilenameRtlTemp;

	bool historyEnable;
	int historyMode;
	int historyFlags;
	char *historyBackgroundFilename;
	char *historyCssFilename;
	char *historyCssFilenameRtl;
	char *historyTemplateFilename;
	char *historyTemplateFilenameRtl;

	bool historyEnableTemp;
	int historyModeTemp;
	int historyFlagsTemp;
	char *historyBackgroundFilenameTemp;
	char *historyCssFilenameTemp;
	char *historyCssFilenameRtlTemp;
	char *historyTemplateFilenameTemp;
	char *historyTemplateFilenameRtlTemp;

public:
	ProtocolSettings(const char *protocolName);
	~ProtocolSettings();
	void	setNext(ProtocolSettings *next);
	const char *getProtocolName();
	ProtocolSettings *getNext();
	void	setSRMMEnable(bool enable);
	bool	isSRMMEnable();
	void	setSRMMMode(int flags);
	int		getSRMMMode();
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

	void	setSRMMEnableTemp(bool enable);
	bool	isSRMMEnableTemp();
	void	setSRMMModeTemp(int flags);
	int		getSRMMModeTemp();
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

	void	setChatEnable(bool enable);
	bool	isChatEnable();
	void	setChatMode(int flags);
	int		getChatMode();
	void	setChatFlags(int flags);
	int		getChatFlags();
	void	setChatBackgroundFilename(const char *filename);
	const char *getChatBackgroundFilename();
	void	setChatCssFilename(const char *filename);
	const char *getChatCssFilename();
	void	setChatCssFilenameRtl(const char *filename);
	const char *getChatCssFilenameRtl();
	void	setChatTemplateFilename(const char *filename);
	const char *getChatTemplateFilename();
	void	setChatTemplateFilenameRtl(const char *filename);
	const char *getChatTemplateFilenameRtl();

	void	setChatEnableTemp(bool enable);
	bool	isChatEnableTemp();
	void	setChatModeTemp(int flags);
	int		getChatModeTemp();
	void	setChatFlagsTemp(int flags);
	int		getChatFlagsTemp();
	void	setChatBackgroundFilenameTemp(const char *filename);
	const char *getChatBackgroundFilenameTemp();
	void	setChatCssFilenameTemp(const char *filename);
	const char *getChatCssFilenameTemp();
	void	setChatCssFilenameRtlTemp(const char *filename);
	const char *getChatCssFilenameRtlTemp();
	void	setChatTemplateFilenameTemp(const char *filename);
	const char *getChatTemplateFilenameTemp();
	void	setChatTemplateFilenameRtlTemp(const char *filename);
	const char *getChatTemplateFilenameRtlTemp();

	void	setHistoryEnable(bool enable);
	bool	isHistoryEnable();
	void	setHistoryMode(int flags);
	int		getHistoryMode();
	void	setHistoryFlags(int flags);
	int		getHistoryFlags();
	void	setHistoryBackgroundFilename(const char *filename);
	const char *getHistoryBackgroundFilename();
	void	setHistoryCssFilename(const char *filename);
	const char *getHistoryCssFilename();
	void	setHistoryCssFilenameRtl(const char *filename);
	const char *getHistoryCssFilenameRtl();
	void	setHistoryTemplateFilename(const char *filename);
	const char *getHistoryTemplateFilename();
	void	setHistoryTemplateFilenameRtl(const char *filename);
	const char *getHistoryTemplateFilenameRtl();

	void	setHistoryEnableTemp(bool enable);
	bool	isHistoryEnableTemp();
	void	setHistoryModeTemp(int flags);
	int		getHistoryModeTemp();
	void	setHistoryFlagsTemp(int flags);
	int		getHistoryFlagsTemp();
	void	setHistoryBackgroundFilenameTemp(const char *filename);
	const char *getHistoryBackgroundFilenameTemp();
	void	setHistoryCssFilenameTemp(const char *filename);
	const char *getHistoryCssFilenameTemp();
	void	setHistoryCssFilenameRtlTemp(const char *filename);
	const char *getHistoryCssFilenameRtlTemp();
	void	setHistoryTemplateFilenameTemp(const char *filename);
	const char *getHistoryTemplateFilenameTemp();
	void	setHistoryTemplateFilenameRtlTemp(const char *filename);
	const char *getHistoryTemplateFilenameRtlTemp();

	void 	copyToTemp();
	void	copyFromTemp();

};

class Options {
private:
   	static int 		generalFlags;
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
		GENERAL_NO_BORDER  		= 0x000020,

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
