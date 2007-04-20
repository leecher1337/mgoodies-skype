/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-07  George Hazan
Copyright ( C ) 2007     Artem Shpynov

Module implements an XMPP protocol extension for reporting and executing ad-hoc, 
human-oriented commands according to XEP-0050: Ad-Hoc Commands
http://www.xmpp.org/extensions/xep-0050.html

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

File name      : $URL: /cvsroot/miranda/miranda/protocols/JabberG/jabber_adhoc.cpp,v $
Revision       : $Revision: 5276 $
Last change on : $Date: 2007-04-20 08:47:42 +0100 $
Last change by : $Author: ashpynov $

*/

#include <CommCtrl.h>
#include "jabber.h"
#include "jabber_iq.h"
#include "resource.h"
#include "m_clui.h"

#define ShowDlgItem( a, b, c )	 ShowWindow( GetDlgItem( a, b ), c )
#define EnableDlgItem( a, b, c ) EnableWindow( GetDlgItem( a, b ), c )

typedef struct _tagJabberAdHocData
{
	int CurrentHeight;
	int curPos;
	int frameHeight;
	RECT frameRect;
	XmlNode * AdHocNode;
	XmlNode * CommandsNode;
	TCHAR * ResponderJID;

}JabberAdHocData;
enum 
{
	JAHM_COMMANDLISTRESULT = WM_USER+1,
	JAHM_PROCESSRESULT
};


static HWND hwndCommandWindow=NULL;

//Declarators
static HWND sttGetWindowFromIq( XmlNode *iqNode );
static BOOL CALLBACK sttDeleteChildWindowsProc( HWND hwnd, LPARAM lParam );
static void JabberIqResult_ListOfCommands( XmlNode *iqNode, void *userdata );
static void JabberIqResult_CommandExecution( XmlNode *iqNode, void *userdata );
static int	JabberAdHoc_RequestListOfCommands( TCHAR * szResponder, HWND hwndDlg );
static int	JabberAdHoc_ExecuteCommand( HWND hwndDlg, TCHAR * jid, JabberAdHocData* dat );
static int	JabberAdHoc_SubmitCommandForm( HWND hwndDlg, JabberAdHocData * dat, char * action );
static int	JabberAdHoc_OnJAHMCommandListResult( HWND hwndDlg, XmlNode * iqNode, JabberAdHocData* dat );
static int	JabberAdHoc_OnJAHMProcessResilt( HWND hwndDlg, XmlNode *workNode, JabberAdHocData* dat );
static int	JabberAdHoc_AddCommandRadio( HWND hFrame, TCHAR * labelStr, int id, int ypos, int value );
static void JabberAdHoc_RefreshFrameScroll( HWND hwndDlg, JabberAdHocData * dat );
static BOOL CALLBACK JabberAdHoc_CommandDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam );

//implementations

// convert iqID to dialog hwnd
static HWND sttGetWindowFromIq( XmlNode *iqNode )
{
	TCHAR * id=JabberXmlGetAttrValue(iqNode, "id");
	if (_tcslen(id)>4)	return (HWND)_tcstol(id+4,NULL,10);
	return hwndCommandWindow;

}
// Callback to clear form content
static BOOL CALLBACK sttDeleteChildWindowsProc( HWND hwnd, LPARAM lParam )
{
	DestroyWindow( hwnd );
	return TRUE;
}

static void sttEnableControls( HWND hwndDlg, BOOL bEnable, int * controlsID )
{
	int i=0;
	while ( controlsID[i]!=0 ) 
		EnableDlgItem( hwndDlg, controlsID[i++], bEnable );
}

static void sttShowControls( HWND hwndDlg, BOOL bShow, int * controlsID )
{
	int i=0;
	while ( controlsID[i]!=0 )
		ShowDlgItem( hwndDlg, controlsID[i++], (bShow) ? SW_SHOW : SW_HIDE );
}
//////////////////////////////////////////////////////////////////////////
// Iq handlers
// Forwards to dialog window procedure
static void JabberIqResult_ListOfCommands( XmlNode *iqNode, void *userdata )
{
	SendMessage( sttGetWindowFromIq( iqNode ), JAHM_COMMANDLISTRESULT, 0, (LPARAM)iqNode );
}

static void JabberIqResult_CommandExecution( XmlNode *iqNode, void *userdata )
{
	SendMessage( sttGetWindowFromIq( iqNode ), JAHM_PROCESSRESULT, (WPARAM)JabberXmlCopyNode( iqNode ), 0 );
}

static int JabberAdHoc_RequestListOfCommands( TCHAR * szResponder, HWND hwndDlg )
{
	int iqId = (int)hwndDlg;
	XmlNodeIq iq( "get", iqId, szResponder );
	XmlNode* query = iq.addChild( "query" );
	query->addAttr( "xmlns", "http://jabber.org/protocol/disco#items" );
	query->addAttr("node", "http://jabber.org/protocol/commands" );
	JabberIqAdd( iqId, IQ_PROC_DISCOCOMMANDS, JabberIqResult_ListOfCommands );
	JabberSend( jabberThreadInfo->s, iq );
	return iqId;
}

static int JabberAdHoc_ExecuteCommand( HWND hwndDlg, TCHAR * jid, JabberAdHocData* dat )
{
	for ( int i = 1; i <= dat->CommandsNode->numChild; i++ )
		if ( IsDlgButtonChecked( GetDlgItem( hwndDlg, IDC_FRAME ), i )) {
			XmlNode *itemNode = JabberXmlGetNthChild( dat->CommandsNode, "item", i );
			if ( itemNode ) {
				TCHAR * node = JabberXmlGetAttrValue( itemNode, "node" );
				if (node)	{
					TCHAR * jid			= JabberXmlGetAttrValue( itemNode, "jid" );
					TCHAR * action		= _T( "execute" );
					TCHAR * description = JabberXmlGetAttrValue( itemNode,"name" );
					if ( !description )	description = node;   

					int iqId = (int)hwndDlg;
					XmlNodeIq iq( "set", iqId, jid );
					XmlNode* query = iq.addChild( "command" );
					query->addAttr( "xmlns", "http://jabber.org/protocol/commands" );
					query->addAttr( "node", node );
					query->addAttr( "action", action );
					JabberIqAdd( iqId, IQ_PROC_EXECCOMMANDS, JabberIqResult_CommandExecution );
					JabberSend( jabberThreadInfo->s, iq );

					EnableDlgItem( hwndDlg, IDC_SUBMIT, FALSE );
					SetDlgItemText( hwndDlg, IDC_SUBMIT, TranslateT( "OK" ) );
		}	}	}
		if ( dat->CommandsNode ) delete dat->CommandsNode;
		dat->CommandsNode = NULL;
		return (TRUE);
}

//Messages handlers
static int JabberAdHoc_OnJAHMCommandListResult( HWND hwndDlg, XmlNode * iqNode, JabberAdHocData* dat )
{
	int nodeIdx = 0;
	TCHAR * type = JabberXmlGetAttrValue( iqNode, "type" );
	if ( !type || !_tcscmp( type, _T( "error" ) ) ) {
		// error ocured here
		TCHAR buff[255];
		TCHAR * code		= NULL;
		TCHAR * description = NULL;
		
		XmlNode* errorNode = JabberXmlGetChild( iqNode, "error" );
		if ( errorNode ) {
			code = JabberXmlGetAttrValue( errorNode, "code" );
			description = errorNode->text;
		}
		_sntprintf( buff, SIZEOF(buff), TranslateT( "Error %s %s" ), (code) ? code : _T(""), (description) ? description : _T("") );	
		SetDlgItemText( hwndDlg, IDC_INSTRUCTION, buff );
	} 
	else if ( !_tcscmp( type, _T("result") ) ) {	
		BOOL validResponse = FALSE;
		EnumChildWindows( GetDlgItem( hwndDlg, IDC_FRAME ), sttDeleteChildWindowsProc, 0 );
		dat->CurrentHeight = 0;
		dat->curPos = 0;
		SetScrollPos( GetDlgItem( hwndDlg, IDC_VSCROLL ), SB_CTL, 0, FALSE );
		XmlNode * queryNode = JabberXmlGetChild( iqNode, "query" );
		if ( queryNode ) {
			TCHAR * xmlns = JabberXmlGetAttrValue( queryNode, "xmlns" );
			TCHAR * node  = JabberXmlGetAttrValue( queryNode, "node" );
			if ( xmlns && node
					&& !_tcscmp( xmlns, _T( "http://jabber.org/protocol/disco#items" ) )
					&& !_tcscmp( node,  _T( "http://jabber.org/protocol/commands" ) ) )
				validResponse = TRUE;
		}
		if ( queryNode && queryNode->numChild > 0 && validResponse ) {
			if ( dat->CommandsNode ) delete dat->CommandsNode;
			dat->CommandsNode = JabberXmlCopyNode( queryNode);

			nodeIdx = 1;
			int ypos = 20;
			int maxChild = queryNode->numChild;
			for (nodeIdx = 1; nodeIdx <= maxChild; nodeIdx++ ) {
				XmlNode *itemNode = JabberXmlGetNthChild( queryNode, "item", nodeIdx );
				if (itemNode) {
					TCHAR *name = JabberXmlGetAttrValue( itemNode, "name" );
					if (!name) name = JabberXmlGetAttrValue( itemNode, "node" );
					ypos = JabberAdHoc_AddCommandRadio( GetDlgItem( hwndDlg,IDC_FRAME ), TranslateTS(name), nodeIdx, ypos, (nodeIdx==1) ? 1 : 0);
					dat->CurrentHeight = ypos;
				}
				else break;
		}	}

		if (nodeIdx>1) {
			SetDlgItemText( hwndDlg, IDC_INSTRUCTION, TranslateT("Select Command") );				
			ShowDlgItem( hwndDlg, IDC_FRAME, SW_SHOW);
			ShowDlgItem( hwndDlg, IDC_VSCROLL, SW_SHOW);
			EnableDlgItem( hwndDlg, IDC_SUBMIT, TRUE);
		} else {
			SetDlgItemText(hwndDlg, IDC_INSTRUCTION, TranslateT("Not supported") );
	}	}

	JabberAdHoc_RefreshFrameScroll( hwndDlg, dat );
	return (TRUE);
}

static int JabberAdHoc_OnJAHMProcessResilt(HWND hwndDlg, XmlNode *workNode, JabberAdHocData* dat)
{
	EnumChildWindows( GetDlgItem( hwndDlg, IDC_FRAME ), sttDeleteChildWindowsProc, 0 );
	dat->CurrentHeight = 0;
	dat->curPos = 0;
	SetScrollPos( GetDlgItem( hwndDlg, IDC_VSCROLL ), SB_CTL, 0, FALSE );

	if ( dat->AdHocNode ) delete dat->AdHocNode;
	dat->AdHocNode = workNode;
	
	if ( dat->AdHocNode == NULL ) return TRUE;

	TCHAR *type;
	if (( type = JabberXmlGetAttrValue( workNode, "type" )) == NULL ) return TRUE;
	if ( !lstrcmp( type, _T("result") ) ) { 
		// wParam = <iq/> node from responder as a result of command execution
		int ypos;
		int id;

		XmlNode *commandNode, *xNode, *n;
		if ( ( commandNode=JabberXmlGetChild( dat->AdHocNode, "command" ) ) == NULL ) return TRUE;
		id = 0;
		ypos = 14;
		if (( xNode = JabberXmlGetChild( commandNode, "x" )) != NULL ) {
			// use jabber:x:data form
			HWND hFrame = GetDlgItem( hwndDlg, IDC_FRAME );
			HFONT hFont = ( HFONT ) SendMessage( hFrame, WM_GETFONT, 0, 0 );
			ShowWindow( GetDlgItem( hwndDlg, IDC_FRAME_TEXT ), SW_HIDE );
			if (( n=JabberXmlGetChild( xNode, "instructions" ))!=NULL && n->text!=NULL )
				SetDlgItemText( hwndDlg, IDC_INSTRUCTION, n->text );
			else
				SetDlgItemText( hwndDlg, IDC_INSTRUCTION, NULL );
			JabberFormCreateUI( hFrame, xNode, &dat->CurrentHeight );
			ShowDlgItem(  hwndDlg, IDC_FRAME , SW_SHOW);
		} 
		else {
			//NO X FORM
			HWND hFrame = GetDlgItem( hwndDlg, IDC_FRAME );

			int toHide[]={ IDC_FRAME_TEXT, IDC_FRAME, IDC_VSCROLL,   0}; 
			sttShowControls(hwndDlg, FALSE, toHide );

			TCHAR * noteText=NULL;
			XmlNode * noteNode=JabberXmlGetChild(commandNode, "note");
			if (noteNode) noteText=noteNode->text;
			SetDlgItemText(hwndDlg, IDC_INSTRUCTION, noteText?noteText:_T(""));

		}
		//check actions
		XmlNode * actionsNode=JabberXmlGetChild(commandNode,"actions");
		if (actionsNode!=NULL) {
			ShowDlgItem( hwndDlg, IDC_PREV, (JabberXmlGetChild(actionsNode,"prev")!=NULL) ? SW_SHOW : SW_HIDE);
			ShowDlgItem( hwndDlg, IDC_NEXT, (JabberXmlGetChild(actionsNode,"next")!=NULL) ? SW_SHOW : SW_HIDE);
			ShowDlgItem( hwndDlg, IDC_COMPLETE, (JabberXmlGetChild(actionsNode,"complete")!=NULL) ? SW_SHOW : SW_HIDE);
			ShowDlgItem( hwndDlg, IDC_SUBMIT, SW_HIDE);
			
			int toEnable[]={ IDC_PREV, IDC_NEXT, IDC_COMPLETE,   0}; 
			sttEnableControls( hwndDlg, TRUE, toEnable );
		} else 	{
			int toHide[]={ IDC_PREV, IDC_NEXT, IDC_COMPLETE,   0}; 
 			sttShowControls(hwndDlg, FALSE, toHide );

			ShowDlgItem(hwndDlg,IDC_SUBMIT,	SW_SHOW);
			EnableDlgItem(hwndDlg,IDC_SUBMIT, TRUE);
		}

		TCHAR * status=JabberXmlGetAttrValue(commandNode,"status");
		if (!status || _tcscmp(status,_T("executing"))) {
			ShowDlgItem( hwndDlg, IDC_SUBMIT, SW_HIDE);
			SetWindowText(GetDlgItem(hwndDlg,IDCANCEL), TranslateT("Done"));
	} 	} 
	else if ( !lstrcmp( type, _T("error"))) {	
		// error occurred here
		int toHide[]={ IDC_FRAME, IDC_FRAME_TEXT, IDC_VSCROLL,
						IDC_PREV, IDC_NEXT, IDC_COMPLETE, IDC_SUBMIT,  0}; 
		
		sttShowControls(hwndDlg, FALSE, toHide );
		
		TCHAR * code=NULL;
		TCHAR * description=NULL;
		TCHAR buff[255];
		XmlNode* errorNode=JabberXmlGetChild(workNode,"error");

		if ( errorNode ) {
			code=JabberXmlGetAttrValue(errorNode,"code");
			description=errorNode->text;
		}
		_sntprintf(buff,SIZEOF(buff),TranslateT("Error %s %s"),code ? code : _T(""),description?description:_T(""));	
		SetDlgItemText(hwndDlg,IDC_INSTRUCTION,buff);
	}
	JabberAdHoc_RefreshFrameScroll( hwndDlg, dat );
	return TRUE;
}

static int JabberAdHoc_SubmitCommandForm(HWND hwndDlg, JabberAdHocData * dat, char * action)
{
	XmlNode * commandNode = JabberXmlGetChild( dat->AdHocNode, "command" );
	XmlNode * xNode		  = JabberXmlGetChild( commandNode, "x" );
	XmlNode * dataNode    = JabberFormGetData( GetDlgItem( hwndDlg, IDC_FRAME ), xNode);

	int iqId = (int)hwndDlg;
	XmlNodeIq iq( "set", iqId, JabberXmlGetAttrValue(dat->AdHocNode, "from"));
	XmlNode* command = iq.addChild( "command" );			
	command->addAttr( "xmlns", "http://jabber.org/protocol/commands" );
	
	TCHAR * sessionId = JabberXmlGetAttrValue(commandNode, "sessionid");
	if ( sessionId ) 
		command->addAttr( "sessionid", sessionId );
	
	TCHAR * node = JabberXmlGetAttrValue(commandNode, "node");
	if ( node ) 
		command->addAttr( "node", node );
	
	if ( action ) 
		command->addAttr( "action", action );
	
	command->addChild( dataNode );
	JabberIqAdd( iqId, IQ_PROC_EXECCOMMANDS, JabberIqResult_CommandExecution );
	JabberSend( jabberThreadInfo->s, iq );
	
	SetWindowText( GetDlgItem(hwndDlg,IDC_INSTRUCTION),TranslateT("In progress. Please Wait..."));
	
	int toDisable[]={IDC_SUBMIT, IDC_PREV, IDC_NEXT, IDC_COMPLETE, 0};
	sttEnableControls( hwndDlg, FALSE, toDisable);
	
	return TRUE;
}


static int JabberAdHoc_AddCommandRadio(HWND hFrame, TCHAR * labelStr, int id, int ypos, int value)
{
	int labelHeight;
	RECT strRect={0};

	int verticalStep=4;
	int ctrlMinHeight=18;
	HWND hCtrl=NULL;

	RECT rcFrame;
	GetClientRect(hFrame,&rcFrame);

	int ctrlOffset=20;
	int ctrlWidth=rcFrame.right-ctrlOffset;

	HDC hdc = GetDC( hFrame );
	labelHeight = max(ctrlMinHeight, DrawText( hdc , labelStr, -1, &strRect, DT_CALCRECT ));
	ctrlWidth=min( ctrlWidth, strRect.right-strRect.left+20 );
	ReleaseDC( hFrame, hdc );

	hCtrl = CreateWindowEx( 0, _T("button"), labelStr, WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_AUTORADIOBUTTON, ctrlOffset, ypos, ctrlWidth, labelHeight, hFrame, ( HMENU ) id, hInst, NULL );
	SendMessage( hCtrl, WM_SETFONT, ( WPARAM ) SendMessage( GetParent(hFrame), WM_GETFONT, 0, 0 ), 0 );
	SendMessage( hCtrl, BM_SETCHECK, value, 0 );
	return (ypos + labelHeight + verticalStep);

}


static void JabberAdHoc_RefreshFrameScroll(HWND hwndDlg, JabberAdHocData * dat)
{
	HWND hFrame = GetDlgItem( hwndDlg, IDC_FRAME );
	HWND hwndScroll = GetDlgItem( hwndDlg, IDC_VSCROLL );
	RECT rc;
	RECT rcScrollRc;
	GetClientRect( hFrame, &rc );
	GetClientRect( hFrame, &dat->frameRect );
	GetWindowRect( hwndScroll, &rcScrollRc );
	dat->frameRect.right-=(rcScrollRc.right-rcScrollRc.left);
	dat->frameHeight = rc.bottom-rc.top;
	if ( dat->frameHeight < dat->CurrentHeight) {
		ShowWindow( hwndScroll, SW_SHOW );
		EnableWindow( hwndScroll, TRUE );
	}
	else ShowWindow( hwndScroll, SW_HIDE );

	SetScrollRange( hwndScroll, SB_CTL, 0, dat->CurrentHeight-dat->frameHeight, FALSE );

}

static BOOL CALLBACK JabberAdHoc_CommandDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	JabberAdHocData* dat = ( JabberAdHocData* )GetWindowLong( hwndDlg, GWL_USERDATA );
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			dat=(JabberAdHocData *)malloc(sizeof(JabberAdHocData));
			memset(dat,0,sizeof(JabberAdHocData));
			SetWindowLong(hwndDlg,GWL_USERDATA,(LPARAM)dat);
			SendMessage( hwndDlg, WM_SETICON, ICON_BIG, ( LPARAM )iconList[21]/*LoadIconEx( "adhoc" )*/);
			hwndCommandWindow = hwndDlg;
			TranslateDialogDefault( hwndDlg );

			//Firstly hide frame
			LONG frameExStyle = GetWindowLong( GetDlgItem( hwndDlg, IDC_FRAME ), GWL_EXSTYLE );
			frameExStyle |= WS_EX_CONTROLPARENT;

			SetWindowLong( GetDlgItem( hwndDlg, IDC_FRAME ), GWL_EXSTYLE, frameExStyle );
	
			int toHide[]={ IDC_FRAME, IDC_VSCROLL, IDC_PREV, IDC_NEXT, IDC_COMPLETE, IDC_FRAME_TEXT, 0}; 
			sttShowControls(hwndDlg, FALSE, toHide );
		
			int toShow[]={ IDC_INSTRUCTION, IDC_SUBMIT, IDCANCEL, 0}; 
			sttShowControls(hwndDlg, TRUE, toShow );

			EnableDlgItem(hwndDlg,IDC_VSCROLL,TRUE);
	
			SetWindowPos(GetDlgItem(hwndDlg,IDC_VSCROLL),HWND_BOTTOM,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);

			SetDlgItemText(hwndDlg,IDC_SUBMIT, TranslateT("Execute"));
			SetDlgItemText(hwndDlg,IDC_INSTRUCTION, TranslateT("Requesting command list. Please wait..."));

			TCHAR *jid=(TCHAR *)lParam;
			JabberAdHoc_RequestListOfCommands(jid, hwndDlg);
			if (dat->ResponderJID) mir_free(dat->ResponderJID);
			dat->ResponderJID=mir_tstrdup(jid);

			TCHAR Caption[200];
			_sntprintf(Caption,SIZEOF(Caption),_T("%s %s"), TranslateT("Jabber Add-Hoc commands at"), dat->ResponderJID );
			SetWindowText(hwndDlg, Caption);

			return TRUE;
		}
	case WM_COMMAND:
		{	
			switch ( LOWORD( wParam )) 
			{

			case IDC_PREV:
				return JabberAdHoc_SubmitCommandForm(hwndDlg,dat,"prev");
			case IDC_NEXT:
				return JabberAdHoc_SubmitCommandForm(hwndDlg,dat,"next");
			case IDC_COMPLETE:
				return JabberAdHoc_SubmitCommandForm(hwndDlg,dat,"complete");
			case IDC_SUBMIT:
				if (!dat->AdHocNode && dat->CommandsNode && LOWORD( wParam )==IDC_SUBMIT) 
					return JabberAdHoc_ExecuteCommand(hwndDlg,dat->ResponderJID, dat);
				else
					return JabberAdHoc_SubmitCommandForm(hwndDlg,dat, NULL);
			case IDCLOSE:
			case IDCANCEL:
				if ( dat->AdHocNode )
					delete dat->AdHocNode;
				dat->AdHocNode=NULL;
				DestroyWindow( hwndDlg );
				return TRUE;
			}
			break;
		}
	case JAHM_COMMANDLISTRESULT:
		return (JabberAdHoc_OnJAHMCommandListResult(hwndDlg,(XmlNode *)lParam,dat));	
	case JAHM_PROCESSRESULT:
		return JabberAdHoc_OnJAHMProcessResilt(hwndDlg,(XmlNode *) wParam,dat);

	case WM_MOUSEWHEEL:
		{
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			if ( zDelta ) {
				int nScrollLines=0;
				SystemParametersInfo(SPI_GETWHEELSCROLLLINES,0,(void*)&nScrollLines,0);
				for (int i=0; i<(nScrollLines+1)/2; i++)
					SendMessage(hwndDlg,WM_VSCROLL, (zDelta<0)?SB_LINEDOWN:SB_LINEUP,0);
			}	}
		return TRUE;

	case WM_VSCROLL:
		{
			int pos;
			if ( dat != NULL ) {
				pos = dat->curPos;
				switch ( LOWORD( wParam ))
				{
				case SB_LINEDOWN:
					pos += 10;
					break;
				case SB_LINEUP:
					pos -= 10;
					break;
				case SB_PAGEDOWN:
					pos += ( dat->CurrentHeight - 10 );
					break;
				case SB_PAGEUP:
					pos -= ( dat->CurrentHeight - 10 );
					break;
				case SB_THUMBTRACK:
					pos = HIWORD( wParam );
					break;
				}
				if ( pos > ( dat->CurrentHeight - dat->frameHeight ))
					pos = dat->CurrentHeight - dat->frameHeight;
				if ( pos < 0 )
					pos = 0;
				if ( dat->curPos != pos ) {
					ScrollWindow( GetDlgItem( hwndDlg, IDC_FRAME ), 0, dat->curPos - pos, NULL ,  &( dat->frameRect ));
					SetScrollPos( GetDlgItem( hwndDlg, IDC_VSCROLL ), SB_CTL, pos, TRUE );
					RECT Invalid=dat->frameRect;
					if (dat->curPos - pos >0)
						Invalid.bottom=Invalid.top+(dat->curPos - pos);
					else
						Invalid.top=Invalid.bottom+(dat->curPos - pos);

					RedrawWindow(GetDlgItem( hwndDlg, IDC_FRAME ), NULL, NULL, RDW_UPDATENOW |RDW_ALLCHILDREN);
					dat->curPos = pos;
				}	}	}
	case WM_DESTROY:
		{
			hwndCommandWindow = NULL;
			if (dat->AdHocNode) delete dat->AdHocNode;
			dat->AdHocNode=NULL;
			if (dat->ResponderJID) mir_free(dat->ResponderJID);
			dat->ResponderJID = NULL;
			if (dat->CommandsNode) delete dat->CommandsNode;
			dat->CommandsNode=NULL;
			break;

			dat=NULL;
		}
	}
	return FALSE;
}




int JabberContactMenuRunCommands(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact;
	DBVARIANT dbv;
	int res=-1;
	JABBER_LIST_ITEM * item=NULL;
	
	if ((( hContact=( HANDLE ) wParam )!=NULL || (lParam!=0)) && jabberOnline ) {
		if ( wParam && !JGetStringT( hContact, "jid", &dbv )) {
			TCHAR jid[200];
			int selected=0;
			_tcsncpy(jid, dbv.ptszVal, SIZEOF(jid));

			JabberListLock();
			{
				item=JabberListGetItemPtr( LIST_ROSTER, jid);
				if (item)
				{
					if (item->resourceCount>1)
					{
						HMENU hMenu=CreatePopupMenu();
						for (int i=0; i<item->resourceCount; i++)
							AppendMenu(hMenu,MF_STRING,i+1, item->resource[i].resourceName);
						HWND hwndTemp=CreateWindowEx(WS_EX_TOOLWINDOW,_T("button"),_T("PopupMenuHost"),0,0,0,10,10,NULL,NULL,hInst,NULL);
						SetForegroundWindow(hwndTemp);
						POINT pt;
						GetCursorPos(&pt);
						RECT rc;
						selected=TrackPopupMenu(hMenu,TPM_RETURNCMD,pt.x,pt.y,0,hwndTemp,&rc);
						DestroyMenu(hMenu);
						DestroyWindow(hwndTemp);
					}
					else selected=1;

					if (selected>0) 
					{
						selected--;
						if (item->resource)
						{
							_tcsncat(jid,_T("/"),SIZEOF(jid));
							_tcsncat(jid,item->resource[selected].resourceName,SIZEOF(jid));
						}
						selected=1;
					}				
				}  
			}
			JabberListUnlock();
			
			if (!item || selected)
				CreateDialogParam( hInst, MAKEINTRESOURCE( IDD_FORM ), NULL, JabberAdHoc_CommandDlgProc, ( LPARAM )(jid) );
			JFreeVariant( &dbv );
			
		}
		else if (lParam!=0)
			CreateDialogParam( hInst, MAKEINTRESOURCE( IDD_FORM ), NULL, JabberAdHoc_CommandDlgProc, lParam );
	}
	return res;
}
