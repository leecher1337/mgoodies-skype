/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.
Copyright ( c ) 2003-5 George Hazan.
Copyright ( c ) 2002-3 Richard Hughes ( original version ).

Miranda IM: the free icq client for MS Windows
Copyright ( C ) 2000-2002 Richard Hughes, Roland Rabien & Tristan Van de Vreede

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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_std.cpp,v $
Revision       : $Revision: 1.9 $
Last change on : $Date: 2006/05/12 20:13:35 $
Last change by : $Author: ghazan $

*/

#include "jabber.h"

HANDLE __stdcall JCreateServiceFunction(
	const char* szService,
	MIRANDASERVICE serviceProc )
{
	char str[ MAXMODULELABELLENGTH ];
	strcpy( str, jabberProtoName );
	strcat( str, szService );
	return CreateServiceFunction( str, serviceProc );
}

#if !defined( _DEBUG )
int __stdcall JCallService( const char* szSvcName, WPARAM wParam, LPARAM lParam )
{
	return CallService( szSvcName, wParam, lParam );
}
#endif

void __stdcall JDeleteSetting( HANDLE hContact, const char* valueName )
{
   DBDeleteContactSetting( hContact, jabberProtoName, valueName );
}

DWORD __stdcall JGetByte( const char* valueName, int parDefltValue )
{
	return DBGetContactSettingByte( NULL, jabberProtoName, valueName, parDefltValue );
}

DWORD __stdcall JGetByte( HANDLE hContact, const char* valueName, int parDefltValue )
{
	return DBGetContactSettingByte( hContact, jabberProtoName, valueName, parDefltValue );
}

char* __stdcall JGetContactName( HANDLE hContact )
{
	return ( char* )JCallService( MS_CLIST_GETCONTACTDISPLAYNAME, WPARAM( hContact ), 0 );
}

DWORD __stdcall JGetDword( HANDLE hContact, const char* valueName, DWORD parDefltValue )
{
	return DBGetContactSettingDword( hContact, jabberProtoName, valueName, parDefltValue );
}

int __stdcall JGetStaticString( const char* valueName, HANDLE hContact, char* dest, int dest_len )
{
	DBVARIANT dbv;
	dbv.pszVal = dest;
	dbv.cchVal = dest_len;
	dbv.type = DBVT_ASCIIZ;

	DBCONTACTGETSETTING sVal;
	sVal.pValue = &dbv;
	sVal.szModule = jabberProtoName;
	sVal.szSetting = valueName;
	if ( JCallService( MS_DB_CONTACT_GETSETTINGSTATIC, ( WPARAM )hContact, ( LPARAM )&sVal ) != 0 )
		return 1;

	return ( dbv.type != DBVT_ASCIIZ );
}

int __stdcall JGetStringUtf( HANDLE hContact, char* valueName, DBVARIANT* dbv )
{
	return DBGetContactSettingStringUtf( hContact, jabberProtoName, valueName, dbv );
}

int __stdcall JGetStringT( HANDLE hContact, char* valueName, DBVARIANT* dbv )
{
	return DBGetContactSettingTString( hContact, jabberProtoName, valueName, dbv );
}

WORD __stdcall JGetWord( HANDLE hContact, const char* valueName, int parDefltValue )
{
	return DBGetContactSettingWord( hContact, jabberProtoName, valueName, parDefltValue );
}

void __fastcall JFreeVariant( DBVARIANT* dbv )
{
	DBFreeVariant( dbv );
}

int __stdcall JSendBroadcast( HANDLE hContact, int type, int result, HANDLE hProcess, LPARAM lParam )
{
	ACKDATA ack = {0};
	ack.cbSize = sizeof( ACKDATA );
	ack.szModule = jabberProtoName;
	ack.hContact = hContact;
	ack.type = type;
	ack.result = result;
	ack.hProcess = hProcess;
	ack.lParam = lParam;
	return JCallService( MS_PROTO_BROADCASTACK, 0, ( LPARAM )&ack );
}

DWORD __stdcall JSetByte( const char* valueName, int parValue )
{
	return DBWriteContactSettingByte( NULL, jabberProtoName, valueName, parValue );
}

DWORD __stdcall JSetByte( HANDLE hContact, const char* valueName, int parValue )
{
	return DBWriteContactSettingByte( hContact, jabberProtoName, valueName, parValue );
}

DWORD __stdcall JSetDword( HANDLE hContact, const char* valueName, DWORD parValue )
{
	return DBWriteContactSettingDword( hContact, jabberProtoName, valueName, parValue );
}

DWORD __stdcall JSetString( HANDLE hContact, const char* valueName, const char* parValue )
{
	return DBWriteContactSettingString( hContact, jabberProtoName, valueName, parValue );
}

DWORD __stdcall JSetStringT( HANDLE hContact, const char* valueName, const TCHAR* parValue )
{
	return DBWriteContactSettingTString( hContact, jabberProtoName, valueName, parValue );
}

DWORD __stdcall JSetStringUtf( HANDLE hContact, const char* valueName, const char* parValue )
{
	return DBWriteContactSettingStringUtf( hContact, jabberProtoName, valueName, parValue );
}

DWORD __stdcall JSetWord( HANDLE hContact, const char* valueName, int parValue )
{
	return DBWriteContactSettingWord( hContact, jabberProtoName, valueName, parValue );
}

char* __stdcall JTranslate( const char* str )
{
	return Translate( str );
}
