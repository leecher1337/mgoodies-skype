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
#include "ieview_services.h"
#include "SRMMHTMLBuilder.h"
#include "SRMM2HTMLBuilder.h"
#include "TabSRMMHTMLBuilder.h"
#include "TemplateHTMLBuilder.h"
#include "IEView.h"
#include "Smiley.h"
#include "m_ieview.h"
#include "Options.h"
#include "ieview_common.h"

int HandleIEWindow(WPARAM wParam, LPARAM lParam) {
	IEVIEWWINDOW *window = (IEVIEWWINDOW *) lParam;
	if (window->iType == IEW_CREATE) {
		HTMLBuilder *builder;
		if (Options::getTemplatesFlags() & Options::TEMPLATES_ENABLED) {
            builder = new TemplateHTMLBuilder();
		} else {
			if (window->dwMode == IEWM_TABSRMM) {
				builder = new TabSRMMHTMLBuilder();
			} else if (window->dwMode == IEWM_SRMM2) {
				builder = new SRMM2HTMLBuilder();
			} else {
				builder = new SRMMHTMLBuilder();
			}
		}
		IEView * view = new IEView(window->parent, builder, window->x, window->y, window->cx, window->cy);
		window->hwnd = view->getHWND();
	} else if (window->iType == IEW_SETPOS) {
		IEView * view = IEView::get(window->hwnd);
		if (view!=NULL) {
			view->setWindowPos(window->x, window->y, window->cx,window->cy);
		}
	} else if (window->iType == IEW_DESTROY) {
		IEView * view = IEView::get(window->hwnd);
		if (view!=NULL) {
			delete view;
		}
	}
	return 0;
}

int HandleIEEvent(WPARAM wParam, LPARAM lParam) {
	IEVIEWEVENT *event = (IEVIEWEVENT *) lParam;
	IEView * view = IEView::get(event->hwnd);
	if (event->iType == IEE_LOG_EVENTS) {
		if (view != NULL) {
			view->appendEvent(event);
		}
	} else if (event->iType == IEE_CLEAR_LOG) {
		if (view != NULL) {
			view->clear();
		}
	}
	return 0;
}

int HandleSmileyShowSelection(WPARAM wParam, LPARAM lParam) {
	SMADD_SHOWSEL *smInfo = (SMADD_SHOWSEL *) lParam;
	SmileyMap* map;
	if (!(Options::getSmileyFlags() & Options::SMILEY_ENABLED)) return 0;
	if (Options::getSmileyFlags() & Options::SMILEY_PROTOCOLS) {
 		map = SmileyMap::getSmileyMap(smInfo->Protocolname);
	} else {
		map = SmileyMap::getSmileyMap("");
	}    
	if (map!=NULL) {
		map->getWindow()->show(smInfo->hwndTarget, smInfo->targetMessage,smInfo->targetWParam,
	                        smInfo->xPosition, smInfo->yPosition);
	}
	return 0;
}
	
int HandleSmileyReplaceSmileys(WPARAM wParam, LPARAM lParam) {
	return 0;
}

int HandleSmileyGetSmileyIcon(WPARAM wParam, LPARAM lParam) {
	return 0;
}

int HandleSmileyGetInfo(WPARAM wParam, LPARAM lParam) {
	SMADD_INFO *smInfo = (SMADD_INFO *) lParam;
	SmileyMap* map;
	if (!(Options::getSmileyFlags() & Options::SMILEY_ENABLED)) return 0;
 	if (Options::getSmileyFlags() & Options::SMILEY_PROTOCOLS) {
 		map = SmileyMap::getSmileyMap(smInfo->Protocolname);
	} else {
		map = SmileyMap::getSmileyMap("");
	}    
	if (map!=NULL) {
		smInfo->ButtonIcon = NULL;
		smInfo->NumberOfVisibleSmileys = map->getSmileyNum();
		smInfo->NumberOfSmileys = map->getSmileyNum();
	}
	return 0;
}


