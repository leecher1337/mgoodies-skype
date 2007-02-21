/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-07  George Hazan

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

File name      : $URL$
Revision       : $Revision: 2866 $
Last change on : $Date: 2006-05-16 20:39:40 +0400 (Вт, 16 май 2006) $
Last change by : $Author: ghazan $

*/

#include "jabber.h"

// basic class - provides interface for various Jabber auth

class TJabberAuth
{

protected:  bool        bIsValid;
            char*       szName;
			ThreadData* info;

public:
            TJabberAuth( ThreadData* );
	virtual ~TJabberAuth();

	virtual	char* getInitialRequest();
	virtual	char* getChallenge( const TCHAR* challenge );

	inline   char* getName() const
				{	return szName;
				}

	inline   bool isValid() const
   			{	return bIsValid;
   			}
	bool wasTokenRequested();
};

// plain auth - the most simple one

class TGoogleAuth : public TJabberAuth
{

public:		TGoogleAuth( ThreadData* );
	virtual ~TGoogleAuth();

	virtual	char* getInitialRequest();
	virtual bool wasTokenRequested();
private: 
	char *currentToken;
	bool bWasGoogleTokenRequested;

};
// plain auth - the most simple one

class TPlainAuth : public TJabberAuth
{

public:		TPlainAuth( ThreadData* );
	virtual ~TPlainAuth();

	virtual	char* getInitialRequest();

};

// md5 auth - digest-based authorization

class TMD5Auth : public TJabberAuth
{
				int iCallCount;
public:		
				TMD5Auth( ThreadData* );
	virtual ~TMD5Auth();

	virtual	char* getChallenge( const TCHAR* challenge );
};

// ntlm auth - LanServer based authorization

class TNtlmAuth : public TJabberAuth
{
				HANDLE hProvider;
public:		
				TNtlmAuth( ThreadData* );
	virtual ~TNtlmAuth();

	virtual	char* getInitialRequest();
	virtual	char* getChallenge( const TCHAR* challenge );
};

