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
class ScriverHTMLBuilder;

#ifndef SCRIVERHTMLBUILDER_INCLUDED
#define SCRIVERHTMLBUILDER_INCLUDED

#include "HTMLBuilder.h"

class ScriverHTMLBuilder:public HTMLBuilder
{
protected:
    virtual void loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour);
	virtual bool isDbEventShown(DWORD dwFlags, DBEVENTINFO * dbei);
	virtual char *timestampToString(DWORD dwFlags, time_t check, int groupStart);
	time_t 		startedTime;
public:
    ScriverHTMLBuilder();
	void buildHead(IEView *, IEVIEWEVENT *event);
	void appendEvent(IEView *, IEVIEWEVENT *event);
	void clear();
};

#endif
