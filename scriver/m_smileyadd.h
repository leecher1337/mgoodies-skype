/*
Miranda SmileyAdd Plugin
Plugin support header file
Copyright (C) 2004 Rein-Peter de Boer (peacow) and followers

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


//replace smileys in a rich edit control... 
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_RICHEDIT2*) &smre;  //pointer to SMADD_RICHEDIT2
//return: TRUE if replacement succeeded, FALSE if not (disable by user?).

typedef struct 
{
	int cbSize;					//size of the structure
	HWND hwndRichEditControl;	//handle to the rich edit control
	CHARRANGE* rangeToReplace;	//same meaning as for normal Richedit use (NULL = replaceall)
	const char* Protocolname;	//protocol to use... if you have defined a protocol, u can 
								//use your own protocol name. SmileyAdd will automatically 
								//select the smileypack that is defined for your protocol.
								//Or, use "Standard" for standard smiley set. Or "ICQ", "MSN"
								//if you prefer those icons. 
								//If not found or NULL, "Standard" will be used
	BOOL useSounds;				//NOT IMPLEMENTED YET, set to FALSE
	BOOL disableRedraw;			//Parameter have been depricated, have no effect on operation
} SMADD_RICHEDIT2;

#define MS_SMILEYADD_REPLACESMILEYS  "SmileyAdd/ReplaceSmileys"


typedef struct 
{
	int cbSize;					//size of the structure
	char* Protocolname;			//protocol to use... if you have defined a protocol, you can 
								//use your own protocol name. Smiley add will automatically 
								//select the smileypack that is defined for your protocol.
								//Or, use "Standard" for standard smiley set. Or "ICQ", "MSN"
								//if you prefer those icons. 
								//If not found or NULL: "Standard" will be used
	int xPosition;				//Postition to place the selectwindow
	int yPosition;				// "
	int Direction;				//Direction (i.e. size upwards/downwards/etc) of the window 0, 1, 2, 3

	HWND hwndTarget;			//Window, where to send the message when smiley is selected.
	UINT targetMessage;			//Target message, to be sent.
	LPARAM targetWParam;		//Target WParam to be sent (LParam will be char* to select smiley)
								//see the example file.
	HWND hwndParent;			//Parent window for smiley dialog 
} SMADD_SHOWSEL2;

#define MS_SMILEYADD_SHOWSELECTION  "SmileyAdd/ShowSmileySelection"


//get smiley button icon
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_INFO*) &smgi;  //pointer to SMADD_INFO
typedef struct 
{
	int cbSize;             //size of the structure
	char* Protocolname;     //   "             "
	HICON ButtonIcon;       //RETURN VALUE: this is filled with the icon handle
							//of the smiley that can be used on the button
							//if used with GETINFO do not destroy!
							//if used with GETINFO2 handle must be destroyed by user!
							//NULL if the buttonicon is not defined...
	int NumberOfVisibleSmileys;    //Number of visible smileys defined.
	int NumberOfSmileys;    //Number of total smileys defined
} SMADD_INFO;

#define MS_SMILEYADD_GETINFO2 "SmileyAdd/GetInfo2"

// Event notifies that options have changed 
// Message dialogs usually need to redraw it's content on reception of this event
#define ME_SMILEYADD_OPTIONSCHANGED  "SmileyAdd/OptionsChanged"

//find smiley in text, API could be called iterativly, on each iteration the remainder 
//of the string after last smiley processed  
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_PARSE*) &smgp;  //pointer to SMADD_PARSE
typedef struct 
{
	int cbSize;                 //size of the structure
	const char* Protocolname;	//protocol to use... if you have defined a protocol, u can 
								//use your own protocol name. Smiley add wil automatically 
								//select the smileypack that is defined for your protocol.
								//Or, use "Standard" for standard smiley set. Or "ICQ", "MSN"
								//if you prefer those icons. 
								//If not found or NULL: "Standard" will be used
	char* str;                  //String to parse 
	HICON SmileyIcon;           //RETURN VALUE: the Icon handle is responsibility of the reciever 
							    //it must be destroyed with DestroyIcon when not needed.
	unsigned startChar;         //Starting smiley character 
								//Because of iterative nature of the API caller should set this 
								//parameter to correct value
	unsigned size;              //Number of characters in smiley (0 if not found)
								//Because of iterative nature of the API caller should set this 
								//parameter to correct value 
} SMADD_PARSE;

#define MS_SMILEYADD_PARSE "SmileyAdd/Parse"

//find smiley in text, API could be called iterativly, on each iteration the remainder 
//of the string after last smiley processed  
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_PARSE*) &smgp;  //pointer to SMADD_PARSE
typedef struct 
{
	int cbSize;                 //size of the structure
	const char* Protocolname;	//protocol to use... if you have defined a protocol, u can 
								//use your own protocol name. Smiley add wil automatically 
								//select the smileypack that is defined for your protocol.
								//Or, use "Standard" for standard smiley set. Or "ICQ", "MSN"
								//if you prefer those icons. 
								//If not found or NULL: "Standard" will be used
	wchar_t* str;                  //String to parse 
	HICON SmileyIcon;           //RETURN VALUE: the Icon handle is responsibility of the reciever 
							    //it must be destroyed with DestroyIcon when not needed.
	unsigned startChar;         //Starting smiley character 
								//Because of iterative nature of the API caller should set this 
								//parameter to correct value
	unsigned size;              //Number of characters in smiley (0 if not found)
								//Because of iterative nature of the API caller should set this 
								//parameter to correct value 
} SMADD_PARSEW;

#define MS_SMILEYADD_PARSEW "SmileyAdd/ParseW"
//find smiley in text
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_REGCAT*) &smgp;  //pointer to SMADD_REGCAT
typedef struct 
{
	int cbSize;                 //size of the structure
	char* name;                 //smiley category name for reference
	char* dispname;             //smiley category name for display 
} SMADD_REGCAT;

#define MS_SMILEYADD_REGISTERCATEGORY "SmileyAdd/RegisterCategory"


//
//
//Below are some older structures used for previous SmileyAdd versions, still supported
//
//


// This API have been depricated in favor of MS_SMILEYADD_GETINFO2
#define MS_SMILEYADD_GETINFO  "SmileyAdd/GetInfo"


//find smiley, this API have been supreceeded with MS_SMILEYADD_PARSE[W]
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_GETICON*) &smgi;  //pointer to SMADD_GETICON
//return: TRUE if SmileySequence starts with a smiley, FALSE if not
typedef struct 
{
	int cbSize;             //size of the structure
	char* Protocolname;     //   "             "
	char* SmileySequence;   //character string containing the smiley 
	HICON SmileyIcon;       //RETURN VALUE: this is filled with the icon handle... 
							//do not destroy!
	int  Smileylength;		//length of the smiley that is found.
} SMADD_GETICON;

#define MS_SMILEYADD_GETSMILEYICON "SmileyAdd/GetSmileyIcon"


//version for smileyadd < 1.5
typedef struct 
{
	int cbSize;					//size of the structure
	const char* Protocolname;	//protocol to use... if you have defined a protocol, u can 
								//use your own protocol name. Smiley add wil automatically 
								//select the smileypack that is defined for your protocol.
								//Or, use "Standard" for standard smiley set. Or "ICQ", "MSN"
								//if you prefer those icons. 
								//If not found or NULL: "Standard" will be used
	int xPosition;				//Postition to place the selectwindow
	int yPosition;				// "
	int Direction;				//Direction (i.e. size upwards/downwards/etc) of the window 0, 1, 2, 3

	HWND hwndTarget;			//Window, where to send the message when smiley is selected.
	UINT targetMessage;			//Target message, to be sent.
	LPARAM targetWParam;		//Target WParam to be sent (LParam will be char* to select smiley)
								//see the example file.
} SMADD_SHOWSEL;


//version for smileyadd < 1.2
typedef struct 
{
	int cbSize;					//size of the structure
	HWND hwndRichEditControl;	//handle to the rich edit control
	CHARRANGE* rangeToReplace;	//same meaning as for normal Richedit use (NULL = replaceall)
	char* Protocolname;			//protocol to use... if you have defined a protocol, u can 
								//use your own protocol name. Smiley add wil automatically 
								//select the smileypack that is defined for your protocol.
								//Or, use "Standard" for standard smiley set. Or "ICQ", "MSN"
								//if you prefer those icons. 
								//If not found or NULL: "Standard" will be used
 } SMADD_RICHEDIT;

