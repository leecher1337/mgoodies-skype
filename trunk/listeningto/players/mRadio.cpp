/* 
ListeningTo plugin for Miranda IM
==========================================================================
Copyright	(C) 2005-2011 Ricardo Pescuma Domenecci
			(C) 2010-2011 Merlin_de

PRE-CONDITION to use this code under the GNU General Public License:
 1. you do not build another Miranda IM plugin with the code without written permission
    of the autor (peace for the project).
 2. you do not publish copies of the code in other Miranda IM-related code repositories.
    This project is already hosted in a SVN and you are welcome to become a contributing member.
 3. you do not create listeningTo-derivatives based on this code for the Miranda IM project.
    (feel free to do this for another project e.g. foobar)
 4. you do not distribute any kind of self-compiled binary of this plugin (we want continuity
    for the plugin users, who should know that they use the original) you can compile this plugin
    for your own needs, friends, but not for a whole branch of people (e.g. miranda plugin pack).
 5. This isn't free beer. If your jurisdiction (country) does not accept
    GNU General Public License, as a whole, you have no rights to the software
    until you sign a private contract with its author. !!!
 6. you always put these notes and copyright notice at the beginning of your code.
==========================================================================

in case you accept the pre-condition,
this is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "..\commons.h"

/////////////////////////////////////////////////////////////////////////////
// main

mRadio::mRadio(int index)
: Player(index)
{
	m_proto				= "mRadio";
	m_name				= _T("mRadio");
	m_version			= 0;

	m_hContact			= INVALID_HANDLE_VALUE;
	m_ptszURL			= NULL;
	m_state				= PL_OFFLINE;

	m_hRadioStatus		= 0;
	m_hProtoAck			= 0;	//only for mradio + 0.0.1.x
	m_hDbSettingChanged	= 0;	//only for mradio + 0.0.1.x
}

mRadio::~mRadio()
{
	COM_Stop();
	FreeData();
}

void 
mRadio::EnableDisable()
{
	static BOOL old = FALSE;
	if(m_enabled == old) {
		return;
	}
	else if(m_enabled){
		COM_Start();
	}
	else {
		COM_Stop();
	}
	old = m_enabled;
}

/////////////////////////////////////////////////////////////////////////////
// PLUGIN ...mRadio

BOOL 
mRadio::COM_Start()
{
	m_version = (DWORD)DBGetContactSettingDword(0, m_proto, "version", 0);
	if(m_version >= 0x00000201) {
		//check if radio station is online
		m_ptszURL = DBGetStringT(NULL, m_proto, "ActiveURL");
		if(m_ptszURL) {
			m_hContact	= FindContact(m_ptszURL);
			m_state		= PL_PLAYING;
			SetActivePlayer(m_index, m_index);
			NotifyInfoChanged();
		}
		//hook ME_RADIO_STATUS
		int (__cdecl mRadio::*hookProc)(WPARAM, LPARAM);
		hookProc = &mRadio::EVT_RadioStatus;
		m_hRadioStatus = HookEventObj(ME_RADIO_STATUS, (MIRANDAHOOKOBJ)*(void **)&hookProc, this);
		return TRUE;
	}
	else
	if(m_version) {		//only for mradio + 0.0.1.x
		//hook ME_PROTO_ACK 
		//ACKTYPE_STATUS - ID_STATUS_ONLINE : hook ME_RADIO_STATUS
		//ACKTYPE_STATUS - ID_STATUS_OFFLINE: UnhookRadioStatus
		int (__cdecl mRadio::*hookProc)(WPARAM, LPARAM);
		hookProc = &mRadio::EVT_ProtoAck;
		m_hProtoAck = HookEventObj(ME_PROTO_ACK, (MIRANDAHOOKOBJ)*(void **)&hookProc, this);

		//check if radio proto is online and simulate a ME_PROTO_ACK event
		if(ID_STATUS_ONLINE == CallProtoService(m_proto, PS_GETSTATUS, NULL, NULL)) {
			ACKDATA ack		= {0};
			ack.cbSize		= sizeof(ack);
			ack.type		= ACKTYPE_STATUS;
			ack.result		= ACKRESULT_SUCCESS;
			ack.szModule	= m_proto;
			ack.lParam		= ID_STATUS_ONLINE;
			EVT_ProtoAck(NULL, (LPARAM)&ack);
		}
		//check if radio station is online and simulate a ME_RADIO_STATUS event
		if(MRC_PLAY == CallService(MS_RADIO_COMMAND, (WPARAM)MRC_STATUS, (LPARAM)RD_STATUS_GET)) {
			DEBUGOUT("mRadio_Start:\tStatus = ","MRC_PLAY");
			EVT_RadioStatus_1x(MRC_PLAY, NULL);
		}
		return TRUE;
	}
	return FALSE;
}

BOOL 
mRadio::COM_Stop()
{
	EVT_Unhook(&m_hRadioStatus);
	if(m_version < 0x00000201) {
		EVT_Unhook(&m_hProtoAck);
		COM_ReleaseServer();
		return TRUE;
	}
	m_version	= 0;
	m_hContact	= INVALID_HANDLE_VALUE;
	m_state		= PL_OFFLINE;
	MIR_FREE(m_ptszURL);
	return TRUE;
}

int __cdecl
mRadio::EVT_RadioStatus(WPARAM wParam, LPARAM lParam)
{
	switch((int)wParam) {
		case RD_STATUS_NEWTAG:		//111 tag data changed
			DEBUGOUT("mRadio_Evt:\tRadioStatus = ","RD_STATUS_NEWTAG");
			if(m_ptszURL) NotifyInfoChanged();
			return 0;
		case RD_STATUS_NOSTATION:	//000 no active station found
			DEBUGOUT("mRadio_Evt:\tRadioStatus = ","RD_STATUS_NOSTATION");
			m_state		= PL_OFFLINE;
			m_hContact	= INVALID_HANDLE_VALUE;
			MIR_FREE(m_ptszURL);
			if(loaded) NotifyInfoChanged();
			return 0;
		case RD_STATUS_NEWSTATION:	//112, lParam: contact handle
			m_hContact = (HANDLE)lParam;
			if(PtrIsValid(m_hContact)) {
				DEBUGOUT("mRadio_Evt:\tRadioStatus = ","RD_STATUS_NEWSTATION (on)");
			}
			else {
				DEBUGOUT("mRadio_Evt:\tRadioStatus = ","RD_STATUS_NEWSTATION (off)");
			}
			return 0;
		case RD_STATUS_NEWTRACK:	//110, LParam - url 
			//at this point RD_STATUS_PLAYING  is true (RD_STATUS_PLAYING is nevere send)
			DEBUGOUT("mRadio_Evt:\tRadioStatus = ","RD_STATUS_NEWTRACK");
			MIR_FREE(m_ptszURL);
			if(lParam) {
				m_ptszURL = mir_u2t((LPWSTR)lParam);
				m_state = PL_PLAYING;
				SetActivePlayer(m_index, m_index);
				NotifyInfoChanged();
			}
			return 0;
		case RD_STATUS_PLAYING:		//001 media is playing
			DEBUGOUT("mRadio_Evt:\tRadioStatus = ","RD_STATUS_PLAYING");
			m_state = PL_PLAYING;
			break;
		case RD_STATUS_PAUSED:		//002, lParam: 1 - pause, 0 - continued
			DEBUGOUT("mRadio_Evt:\tRadioStatus = ", lParam ? "MRC_PAUSE (pause)" : "MRC_PAUSE (play)");
			m_state = lParam ? PL_PAUSED : PL_PLAYING;
			break;
		case RD_STATUS_STOPPED:		//003 media is stopped (only for playlists)
			DEBUGOUT("mRadio_Evt:\tRadioStatus = ","RD_STATUS_STOPPED");
			m_state = PL_STOPPED;
			break;
		case RD_STATUS_CONNECT:		//004 plugin try to connect to the station
			DEBUGOUT("mRadio_Evt:\tRadioStatus = ","RD_STATUS_CONNECT");
			m_state = PL_STOPPED;
			return 0;
		case RD_STATUS_ABORT:		//005 plugin want to abort while try to connect
			DEBUGOUT("mRadio_Evt:\tRadioStatus = ","RD_STATUS_ABORT");
			m_state = PL_OFFLINE;
			return 0;
		case RD_STATUS_POSITION:	//107 position was changed
			DEBUGOUT("mRadio_Evt:\tRadioStatus = ","RD_STATUS_POSITION");
			break;
		case RD_STATUS_MUTED:		//108 Mute/Unmute command was sent
			DEBUGOUT("mRadio_Evt:\tRadioStatus = ","RD_STATUS_MUTED");
			return 0;
		case RD_STATUS_RECORD:		//109 "Record" action called
			DEBUGOUT("mRadio_Evt:\tRadioStatus = ", lParam ? "RD_STATUS_RECORD (on)" : "RD_STATUS_RECORD (off)");
			return 0;
		#ifdef DEBUG
		case MRC_RECORD:	//, LParam - 0 (stop) / 1 (record)
			DEBUGOUT("mRadio_Evt:\tRadioStatus = ", lParam ? "MRC_RECORD (on)" : "MRC_RECORD (off)");
			return 0;
		default:
			DEBUGOUT("mRadio_Evt:\tRadioStatus = ","??");
			DebugBreak();	//shoud not happen
			return 0;
		#endif
		} //end switch
	NotifyInfoChanged();
	return 0;
}

HANDLE		/* based on pl_mradio.pas function Fill*/
mRadio::FindContact(LPTSTR ptszURL)
{
	//walk through all the contacts stored in the DB
	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact != NULL) {
		if(ptszURL) {
			LPTSTR ptszDB = DBGetStringT(hContact,m_proto, "StationURL");
			if(ptszDB) {
				if(!_tcsicmp(ptszDB,ptszURL)) {
					mir_free(ptszDB);
					return hContact;
				}
				mir_free(ptszDB);
			}
		}
		else
		if(ID_STATUS_ONLINE == DBGetContactSettingWord(hContact,m_proto,"Status",WORD(WAT_RES_NOTFOUND))) {
			return hContact;
		}
		hContact  = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}
	return INVALID_HANDLE_VALUE;
}

BOOL 
mRadio::GetListeningInfo(LISTENINGTOINFO *lti)
{
	FreeData();
	return COM_infoCache() ? Player::GetListeningInfo(lti) : FALSE;
}

BOOL 
mRadio::COM_infoCache()
{
	if(	(m_state <= PL_STOPPED) ||
		(m_hContact == INVALID_HANDLE_VALUE) ||
		(!m_hContact) )
		return FALSE;

	m_listening_info.cbSize		= sizeof(m_listening_info);
	m_listening_info.ptszPlayer	= mir_tstrdup(m_name);
	m_listening_info.ptszType	= mir_tstrdup(_T("Radio"));
	m_listening_info.dwFlags	= LTI_TCHAR;

	//  Copy new data
	//  m_listening_info.ptszAlbum	= not supportet
	//  m_listening_info.ptszLength	= not supportet (Length of the track, formatted as [HH:]MM:SS.)
	//  m_listening_info.ptszTrack	= not supportet
	//  m_listening_info.ptszYear,	= not supportet
	//  m_listening_info.????		= DBGetStringT(NULL,		m_proto,	"ActiveCodec");

	if(!(m_listening_info.ptszArtist	= DBGetStringT(NULL,		m_proto,	"Artist"))) {
		m_listening_info.ptszArtist		= DBGetStringT(m_hContact,	"CList",	"MyHandle");
		//strUnknown
	}

	if(!(m_listening_info.ptszTitle		= DBGetStringT(NULL,		m_proto,	"Title"))) {
		if(!(m_listening_info.ptszTitle	= DBGetStringT(m_hContact,	"CList",	"StatusMsg"))) {
			m_listening_info.ptszTitle	= DBGetStringT(NULL,		m_proto,	"ActiveURL");
		}
	}

	//check if Title == Artist
	if(	m_listening_info.ptszArtist &&
		m_listening_info.ptszTitle &&
		!_tcscmp(m_listening_info.ptszArtist, m_listening_info.ptszTitle))
	{
		MIR_FREE(m_listening_info.ptszArtist);
		m_listening_info.ptszArtist		= DBGetStringT(m_hContact,	"CList",	"MyHandle");
	}

	if(!(m_listening_info.ptszGenre		= DBGetStringT(NULL,		m_proto,	"Genre"))) {
		m_listening_info.ptszGenre		= DBGetStringT(m_hContact,	m_proto,	"Genre");
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// only for mradio + 0.0.1.x (never need for + 0.0.2.x)

BOOL		/* only for mradio + 0.0.1.x - based on pl_mradio.pas function InitmRadio*/
mRadio::COM_ConnectServer()
{
	if(m_version >= 0x00000201) {
		m_state = PL_PLAYING;
		SetActivePlayer(m_index, m_index);
		NotifyInfoChanged();
		DEBUGOUT("mRadio_Player:\tServer = ","on");
		return TRUE;
	}

	//first check Status
	if( PtrIsValid(m_hContact) && 
		(ID_STATUS_ONLINE == (WORD)DBGetContactSettingWord(m_hContact, m_proto, "Status", ID_STATUS_OFFLINE)))
	{
		DEBUGOUT("mRadio_Player:\tServer = ","station is online");
		m_state = PL_PLAYING;
		SetActivePlayer(m_index, m_index);
		NotifyInfoChanged();
	}
	//hook ME_DB_CONTACT_SETTINGCHANGED
	if(!m_hDbSettingChanged) {
		int (__cdecl mRadio::*hookProc)(WPARAM, LPARAM);
		hookProc = &mRadio::EVT_DbSettingChanged;
		m_hDbSettingChanged = HookEventObj(ME_DB_CONTACT_SETTINGCHANGED, (MIRANDAHOOKOBJ)*(void **)&hookProc, this);
	}
	DEBUGOUT("mRadio_Player:\tServer = ","on");

	return TRUE;
}

void		/* only for mradio + 0.0.1.x - based on pl_mradio.pas function ClearmRadio*/
mRadio::COM_ReleaseServer()
{
	EVT_Unhook(&m_hDbSettingChanged);
	DEBUGOUT("mRadio_Player:\tServer = ","off");
	m_hContact	= INVALID_HANDLE_VALUE;
	MIR_FREE(m_ptszURL);

	m_state		= PL_OFFLINE;
	m_version	= 0;
	if(loaded)
		NotifyInfoChanged();
}

int __cdecl	/* only for mradio + 0.0.1.x*/
mRadio::EVT_RadioStatus_1x(WPARAM wParam, LPARAM lParam)
{
	switch((int)wParam) {
		case MRC_STOP:		//, LParam - 0
			DEBUGOUT("mRadio_Evt:\tRadioStatus = ","MRC_STOP");
			//mradio + 0.0.1.8
			if(m_state == PL_PLAYING) {
				COM_ReleaseServer();
				return 0;
			}
			if(PtrIsValid(m_hContact)) {
				m_state = PL_STOPPED;
				NotifyInfoChanged();
			}
			//mradio <= 0.0.1.7 fire ME_RADIO_STATUS - MRC_STOP too late 
			else 
			if(m_hDbSettingChanged){
				COM_ReleaseServer();
			}
			break;
		case MRC_PLAY:				//, LParam - url 0.0.1.x
			MIR_FREE(m_ptszURL);
			//mradio <= 0.0.1.7 
			//...fire ME_RADIO_STATUS - MRC_PLAY too early
			//...have no way to get m_hContact (active station)
			if(lParam) {
				m_ptszURL = mir_u2t((LPWSTR)lParam);
				m_hContact = FindContact(m_ptszURL);
			}
			else {
				m_hContact = FindContact();
			}

			//RadioStatus 0.0.1.x is buggy we need to check the contact status
			if(PtrIsValid(m_hContact)) {
				DEBUGOUT("mRadio_Evt:\tRadioStatus = ","MRC_PLAY");
				COM_ConnectServer();
			}
			else {
				DEBUGOUT("mRadio_Evt:\tRadioStatus = ","MRC_PLAY (PL_OFFLINE)");
				COM_ReleaseServer();
			}
			break;
		case MRC_PAUSE:		//, LParam - 0 (pause) / 1 (play)
			DEBUGOUT("mRadio_Evt:\tRadioStatus = ", lParam ? "MRC_PAUSE (play)" : "MRC_PAUSE (pause)");
			m_state = lParam ? PL_PLAYING : PL_PAUSED;
			NotifyInfoChanged();
			break;
		case MRC_SEEK:		//, LParam - lParam is value in sec
			DEBUGOUT("mRadio_Evt:\tRadioStatus = ","MRC_SEEK");
			break;
		case MRC_RECORD:	//, LParam - 0 (stop) / 1 (record)
			DEBUGOUT("mRadio_Evt:\tRadioStatus = ", lParam ? "MRC_RECORD (on)" : "MRC_RECORD (off)");
			break;
	}
	return 0;
}

int __cdecl	/* only for mradio + 0.0.1.x - based on pl_mradio.pas function SettingsChanged*/
mRadio::EVT_ProtoAck(WPARAM wParam,LPARAM lParam)
{
	ACKDATA *ack = ( ACKDATA* )lParam;
	if(ack->type   != ACKTYPE_STATUS)
		return 0; 
	if(ack->result != ACKRESULT_SUCCESS)
		return 0;
	if(strcmp(ack->szModule, m_proto))
		return 0;
#ifdef DEBUG
	if(wParam) DebugBreak();		//shoud not happen
	//int dummy = (int)ack->lParam;
#endif
	switch( (int)ack->lParam ) {
		case ID_STATUS_ONLINE:
			DEBUGOUT("mRadio_Evt:\tProtoAck = ","ID_STATUS_ONLINE");
			if(!m_hRadioStatus) {
				//hook ME_RADIO_STATUS
				int (__cdecl mRadio::*hookProc)(WPARAM, LPARAM);
				hookProc = &mRadio::EVT_RadioStatus_1x;
				m_hRadioStatus = HookEventObj(ME_RADIO_STATUS, (MIRANDAHOOKOBJ)*(void **)&hookProc, this);
			}
			break;
		case ID_STATUS_OFFLINE:
			DEBUGOUT("mRadio_Evt:\tProtoAck = ","ID_STATUS_OFFLINE");
			EVT_Unhook(&m_hDbSettingChanged);
			EVT_Unhook(&m_hRadioStatus);
			break;
	}
	return 0;
}

int __cdecl	/* only for mradio + 0.0.1.x - based on pl_mradio.pas function SettingsChanged*/
mRadio::EVT_DbSettingChanged(WPARAM wParam,LPARAM lParam)
{
	//return as fast as posibil
	DBCONTACTWRITESETTING* cws = (DBCONTACTWRITESETTING*)lParam;
	if(!strcmp(cws->szModule,m_proto)) {
		//ugly process for Status change coz mradio <= 0.0.1.7 fire 
		//ME_RADIO_STATUS - MRC_PLAY too early
		//ME_RADIO_STATUS - MRC_STOP too late
		if(	cws->value.type == DBVT_WORD &&
			!strcmp(cws->szSetting, "Status"))
		{
			if(cws->value.wVal != ID_STATUS_ONLINE && m_hContact == (HANDLE)wParam) {
				DEBUGOUT("mRadio_Evt:\tDbSettingChanged = ","ID_STATUS_OFFLINE");
				m_hContact = INVALID_HANDLE_VALUE;
				m_state = PL_STOPPED;	//COM_ReleaseServer set PL_OFFLINE later)
			}
			else
			if(cws->value.wVal == ID_STATUS_ONLINE) {
				DEBUGOUT("mRadio_Evt:\tDbSettingChanged = ","ID_STATUS_ONLINE");
				m_hContact = (HANDLE)wParam;
				m_state = PL_PLAYING;
				SetActivePlayer(m_index, m_index);
				NotifyInfoChanged();
			}
		}
		else //songinfo change ?
		if(	!wParam &&	
			PtrIsValid(m_hContact) && (
			!strcmp(cws->szSetting, "Artist") ||
			!strcmp(cws->szSetting, "Title")  ||
			!strcmp(cws->szSetting, "Genre")  ))
		{
			DEBUGOUT("mRadio_Evt:\tDbSettingChanged = ",cws->szSetting);
			m_state = PL_PLAYING;
			NotifyInfoChanged();
		}
	}
	return 0;
}


