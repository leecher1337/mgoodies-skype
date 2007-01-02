/* 
Copyright (C) 2006 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#ifndef __M_VOICE_H__
# define __M_VOICE_H__


#define VOICE_UNICODE	0x80000000

#ifdef UNICODE
# define VOICE_TCHAR VOICE_UNICODE
#else
# define VOICE_TCHAR 0
#endif

typedef struct {
	int cbSize;				// Struct size
	const char *szModule;	// The name of the protocol module (used only in notifications)
	char *id;				// Protocol especific ID for this call
	int flags;				// Can be VOICE_CALL_CONTACT or VOICE_CALL_STRING (VOICE_UNICODE to say the string is unicode)
	union {					// Who to call
		HANDLE hContact;
		TCHAR *ptszContact;
		char *pszContact;
		WCHAR *pwszContact;
	};

} VOICE_CALL;


/*
Notifies that someone wants to call the user

wParam: const VOICE_CALL *
lParam: ignored
return: 0 on success
*/
#define PE_VOICE_RINGING				"/Voice/Ringing"

/*
Notifies that a call has ended

wParam: const VOICE_CALL *
lParam: ignored
return: 0 on success
*/
#define PE_VOICE_ENDEDCALL				"/Voice/EndedCall"

/*
Notifies that a call has started

wParam: const VOICE_CALL *
lParam: ignored
return: 0 on success
*/
#define PE_VOICE_STARTEDCALL			"/Voice/StartedCall"

/*
Notifies that a call has been put on hold

wParam: const VOICE_CALL *
lParam: ignored
return: 0 on success
*/
#define PE_VOICE_HOLDEDCALL				"/Voice/HoldedCall"


#define VOICE_SUPPORTED			1	// Set if proto support voice calls. Probabilly will be 1 ;)
#define VOICE_CALL_CONTACT		2	// Set if a call can be made to a hContact
#define VOICE_CALL_STRING		4	// Set if a call can be made to some string (PS_VOICE_CALL_STRING_VALID is used to validate the string)
#define VOICE_CAN_SET_DEVICE	8	// Set if the devices to mic in and sound out can be set (or the protocol will handle it internally)
#define VOICE_CAN_HOLD			16	// Set if a call can be put on hold
/*
Get protocol voice support flags

wParam: ignored
lParam: ignored
return: 0 on success
*/
#define PS_VOICE_GETINFO				"/Voice/GetInfo"

/*
Service called to make the protocol answer a call.
It is an async call. If the call was answered, the PE_VOICE_STARTEDCALL
notification will be fired.

wParam: (const char *) id
lParam: ignored
return: 0 on success
*/
#define PS_VOICE_ANSWERCALL				"/Voice/AnswerCall"

/*
Service called to make the protocol answer a call. This can be called if the 
call is ringing or has started. If called any other time it should be ignored.
It is an async call. If the call was droped, the PE_VOICE_ENDEDCALL
notification will be fired.

wParam: (const char *) id
lParam: ignored
return: 0 on success
*/
#define PS_VOICE_DROPCALL				"/Voice/DropCall"

/*
Service called to make the protocol hold a call. This means that the call should not
be droped, but it should be muted and put in a hold, to allow other call to be answered.
If the protocol can't hold a cal, it should be droped.

This can be called if the call has started. If called any other time it should be ignored.
It is an async call. If the call was droped, the PE_VOICE_HOLDEDCALL
notification will be fired.

wParam: (const char *) id
lParam: ignored
return: 0 on success
*/
#define PS_VOICE_HOLDCALL				"/Voice/HoldCall"

/*
Used if protocol support VOICE_CALL_STRING. The call string is passed as
wParam and the proto should validate it. 

wParam: (const TCHAR *) call string
lParam: ignored
return: 0 if wrong, 1 if correct
*/
#define PS_VOICE_CALL_STRING_VALID		"/Voice/CallStringValid"





#endif // __M_VOICE_H__
