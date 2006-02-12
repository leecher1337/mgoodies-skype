// Copyright Scott Ellis (mail@scottellis.com.au) 2005
// This software is licenced under the GPL (General Public Licence)
// available at http://www.gnu.org/copyleft/gpl.html

#ifndef _FONT_SERVICE_API_INC
#define _FONT_SERVICE_API_INC

// style flags
#define DBFONTF_BOLD       1
#define DBFONTF_ITALIC     2
#define DBFONTF_UNDERLINE  4

// flags for compatibility
#define FIDF_APPENDNAME			1		// append 'Name' to the setting used to store font face (as CLC settings require)
#define FIDF_NOAS				2		// disable the <prefix>As setting to prevent 'same as' behaviour
#define FIDF_SAVEACTUALHEIGHT	4		// write the actual height of a test string to the db

#define FIDF_DEFAULTVALID		32		// the default font settings are valid - else, just use generic default
#define FIDF_NEEDRESTART		64		// setting changes will not take effect until miranda is restarted

typedef struct FontSettings_tag
{
    COLORREF colour;
    char size;
    BYTE style;
    BYTE charset;
    char szFace[LF_FACESIZE];
} FontSettings;

typedef struct FontID_tag {
	int cbSize;
	char group[64];
	char name[64];
	char dbSettingsGroup[32];
	char prefix[32];
	DWORD flags;
	FontSettings deffontsettings; // defaults, if flags & FIDF_DEFAULTVALID
	int order;
} FontID;

typedef struct ColourID_tag {
	int cbSize;
	char group[64];
	char name[64];
	char dbSettingsGroup[32];
	char setting[32];
	DWORD flags;		// not used
	COLORREF defcolour; // default value
	int order;
} ColourID;

// register a font
// wparam = (FontID *)&font_id
// lparam = 0
#define MS_FONT_REGISTER		"Font/Register"

// get a font
// wparam = (FontID *)&font_id (only name and group matter)
// lParam = (LOGFONT *)&logfont
#define MS_FONT_GET				"Font/Get"

// fired when a user modifies font settings, so reget your fonts
#define ME_FONT_RELOAD			"Font/Reload"

// register a colour (this should be used for everything except actual text colour for registered fonts)
// [note - a colour with name 'Background' has special meaning and will be used as the background colour of
// the font list box in the options, for the given group]
// wparam = (ColourID *)&colour_id
// lparam = 0
#define MS_COLOUR_REGISTER		"Colour/Register"

// get a colour
// wparam = (ColourID *)&colour_id (only name and group matter)
// lParam = (LOGFONT *)&logfont
#define MS_COLOUR_GET				"Colour/Get"

// fired when a user modifies font settings, so reget your fonts and colours
#define ME_COLOUR_RELOAD			"Colour/Reload"


#endif
