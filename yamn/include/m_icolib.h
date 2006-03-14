// ---------------------------------------------------------------------------80
//         Icons Library Manager plugin for Miranda Instant Messenger
//         __________________________________________________________
//
// Copyright © 2005 Denis Stanishevskiy // StDenis
// Copyright © 2006 Joe Kucera
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// -----------------------------------------------------------------------------

#define SKINICONDESC_SIZE     sizeof(SKINICONDESC)
#define SKINICONDESC_SIZE_V1  0x18
#define SKINICONDESC_SIZE_V2  0x1C
#define SKINICONDESC_SIZE_V3  0x24

typedef struct {
  int cbSize;
  union {
    char *pszSection;         // section name used to group icons
    TCHAR *ptszSection;
    wchar_t *pwszSection;
  };
  union {
    char *pszDescription;     // description for options dialog
    TCHAR *ptszDescription;
    wchar_t *pwszDescription;
  };
  char *pszName;              // name to refer to icon when playing and in db
  char *pszDefaultFile;       // default icon file to use
  int  iDefaultIndex;         // index of icon in default file
  HICON hDefaultIcon;         // handle to default icon
  int cx,cy;                  // dimensions of icon
  int flags; 
} SKINICONDESC;

#define SIDF_UNICODE  0x100   // Section and Description are in UCS-2

#if defined(_UNICODE)
  #define SIDF_TCHAR  SIDF_UNICODE
#else
  #define SIDF_TCHAR  0
#endif

//
//  Add a icon into options UI
//
//  wParam = (WPARAM)0
//  lParam = (LPARAM)(SKINICONDESC*)sid;
//
#define MS_SKIN2_ADDICON "Skin2/Icons/AddIcon"

//
//  Retrieve HICON with name specified in lParam
//  Returned HICON SHOULDN'T be destroyed, it is managed by IcoLib
//

#define MS_SKIN2_GETICON "Skin2/Icons/GetIcon"

//
//  Icons change notification
//
#define ME_SKIN2_ICONSCHANGED "Skin2/IconsChanged"
