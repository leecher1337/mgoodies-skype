/*
Miranda SmileyAdd Plugin
Plugin support header file
Copyright (C) 2004 Rein-Peter de Boer (peacow)

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

//examples!!!!!


//replace smileys in a rich edit control... 
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_RICHEDIT*) &smre;  //pointer to SmAddRicheditStructure
//return: TRUE if replacement succeeded, FALSE if not (disable by user?).

typedef struct 
{
  int cbSize;                //size of the structure
	HWND hwndRichEditControl;  //handle to the rich edit control
	CHARRANGE* rangeToReplace; //same meaning as for normal Richedit use (NULL = replaceall)
  const char* Protocolname;  //protocol to use... if you have defined a protocol, u can 
                             //use your own protocol name. SmileyAdd wil automatically 
                             //select the smileypack that is defined for your protocol.
                             //Or, use "Standard" for standard smiley set. Or "ICQ", "MSN"
                             //if you prefer those icons. 
                             //If not found or NULL, "Standard" will be used
  BOOL useSounds;            //NOT IMPLEMENTED YET, set to FALSE
  BOOL disableRedraw;        //If true then no redraw/refresh will be done. For performance
                             //this is recommend. Disable redraw, etc yourself first, and then
                             //refresh after the call to smileyadd.. This will give YOU the 
                             //control on what en where to refresh. 
                             //->>everything will be screwed up and not restored.
                             //TRUE is recommended -> disable&save redraw, restore yourself.
} SMADD_RICHEDIT2;

#define MS_SMILEYADD_REPLACESMILEYS  "SmileyAdd/ReplaceSmileys"


typedef struct 
{
  int cbSize;                //size of the structure
  const char* Protocolname;  //protocol to use... if you have defined a protocol, u can 
                             //use your own protocol name. Smiley add wil automatically 
                             //select the smileypack that is defined for your protocol.
                             //Or, use "Standard" for standard smiley set. Or "ICQ", "MSN"
                             //if you prefer those icons. 
                             //If not found or NULL: "Standard" will be used
  int xPosition;             //Postition to place the selectwindow
  int yPosition;             // "
  int Direction;             //Direction (i.e. size upwards/downwards/etc) of the window 0, 1, 2, 3
  
	HWND hwndTarget;           //Window, where to send the message when smiley is selected.
  UINT targetMessage;        //Target message, to be sent.
  LPARAM targetWParam;       //Target WParam to be sent (LParam will be char* to select smiley)
                             //see the example file.
} SMADD_SHOWSEL;

#define MS_SMILEYADD_SHOWSELECTION  "SmileyAdd/ShowSmileySelection"



//find smiley
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_GETICON*) &smgi;  //pointer to SmAddRicheditStructure
//return: TRUE if SmileySequence starts with a smiley, FALSE if not
typedef struct 
{
  int cbSize;             //same as in SMADD_RICHEDIT
  char* Protocolname;     //   "             "
  char* SmileySequence;   //character string containing the smiley 
  HICON SmileyIcon;       //RETURN VALUE: this is filled with the icon handle... 
                          //do not destroy!
  int Smileylength;       //length of the smiley that is found.
} SMADD_GETICON;

#define MS_SMILEYADD_GETSMILEYICON "SmileyAdd/GetSmileyIcon"


//get smiley button icon
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_GETINFO*) &smgi;  //pointer to SmAddRicheditStructure
typedef struct 
{
  int cbSize;             //same as in SMADD_RICHEDIT
  char* Protocolname;     //   "             "
  HICON ButtonIcon;       //RETURN VALUE: this is filled with the icon handle
                          //of the smiley that can be used on the button
                          //do not destroy! NULL if the buttonicon is not defined...
  int NumberOfVisibleSmileys;    //Number of visible smileys defined.
  int NumberOfSmileys;    //Number of total smileys defined

} SMADD_INFO;

#define MS_SMILEYADD_GETINFO "SmileyAdd/GetInfo"






//
//
//Below are some older structures used for previous smileyadd versions, still supported
//
//


//version for smileyadd < 1.2
typedef struct 
{
  int cbSize;                //size of the structure
	HWND hwndRichEditControl;  //handle to the rich edit control
	CHARRANGE* rangeToReplace; //same meaning as for normal Richedit use (NULL = replaceall)
  char* Protocolname;        //protocol to use... if you have defined a protocol, u can 
                             //use your own protocol name. Smiley add wil automatically 
                             //select the smileypack that is defined for your protocol.
                             //Or, use "Standard" for standard smiley set. Or "ICQ", "MSN"
                             //if you prefer those icons. 
                             //If not found or NULL: "Standard" will be used
 } SMADD_RICHEDIT;



#define MS_SMILEYADD_GETSMILEYICON "SmileyAdd/GetSmileyIcon"

