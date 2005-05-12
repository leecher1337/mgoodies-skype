/*
  Name: NewGenerationNotify - Plugin for Miranda ICQ
  File: main.c - Main DLL procedures
  Version: 0.0.4
  Description: Notifies you about some events
  Author: prezes, <prezesso@klub.chip.pl>
  Date: 01.09.04 / Update: 12.05.05 17:00
  Copyright: (C) 2002 Starzinger Michael

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "newgenerationnotify.h"
#include <m_database.h>
#include <m_skin.h>
#include <m_clist.h>
#include <m_protocols.h>
//needed for ICQEVENTTYPE_* (Webpager & Emailexpress)
#include <m_protosvc.h>
#include <m_icq.h>
//needed for reply instead of read
#include <m_message.h>
#include <m_popup.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

extern int g_IsServiceAvail;

static int PopupCount = 0;

PLUGIN_DATA* PopUpList[20];

/*
TIME NowTime()
{
	time_t actTime;
	TIME endTime;
	time(&actTime);
	strftime(endTime.time,sizeof(endTime.time), "%H:%M", localtime(&actTime));
	strftime(endTime.date,sizeof(endTime.date), "%Y.%m.%d", localtime(&actTime));
	strftime(endTime.all,sizeof(endTime.all), "%Y.%m.%d %H:%M", localtime(&actTime));
	return endTime;
}
*/

int NumberPopupData(HANDLE hContact)
{
	int n;

	for (n=0;n<20;n++)
	{
		if (!PopUpList[n] && !hContact)
			return n;

		if (PopUpList[n] && PopUpList[n]->hContact == hContact)
			return n;
	}
	return -1;
}



int PopupAct(HWND hWnd, UINT mask, PLUGIN_DATA* pdata)
{

	EVENT_DATA_EX* eventData;

	if (mask & MASK_OPEN)
    {
        // do MS_MSG_SENDMESSAGE instead if wanted to reply and not read!
        if (pdata->pluginOptions->bMsgReplywindow && pdata->eventType == EVENTTYPE_MESSAGE)
        {
            _Workaround_CallService(MS_MSG_SENDMESSAGE, (WPARAM)pdata->hContact, (LPARAM)NULL);
        }
        else
        {
            CLISTEVENT* cle;
//BUG!! gets the first message for the contact, doesn't have to be that one we wanted
            cle = (CLISTEVENT*)CallService(MS_CLIST_GETEVENT, (WPARAM)pdata->hContact, 0);
            if (cle)
            {
                if (ServiceExists(cle->pszService))
                    _Workaround_CallService(cle->pszService, (WPARAM)NULL, (LPARAM)cle);
            }
        }
    }

    if (mask & MASK_REMOVE)
    {
	
		eventData = pdata->lastEventData;
		while (eventData && eventData->prev)
		{
			CallService(MS_CLIST_REMOVEEVENT, (WPARAM)pdata->hContact, (LPARAM)eventData->hEvent);
			CallService(MS_DB_EVENT_MARKREAD, (WPARAM)pdata->hContact, (LPARAM)eventData->hEvent);
			eventData = eventData->prev;
			free(eventData->next);
			eventData->next = NULL;
		}
		free(eventData);
		pdata->lastEventData = pdata->firstEventData = pdata->firstShowEventData = NULL;
		PopUpList[NumberPopupData(pdata->hContact)] = NULL;
    }

    if (mask & MASK_DISMISS)
	{
        PUDeletePopUp(hWnd);
	}
		
	return 0;
}

static BOOL CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PLUGIN_DATA* pdata = NULL;

    pdata = (PLUGIN_DATA*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd, (LPARAM)pdata);
    if (!pdata) return FALSE;

    switch (message)
    {
        case WM_COMMAND:
			PopupAct(hWnd, pdata->pluginOptions->maskActL, pdata);
            break;
        case WM_CONTEXTMENU:
			PopupAct(hWnd, pdata->pluginOptions->maskActR, pdata);
            break;
		case UM_FREEPLUGINDATA:
			PopupCount--;
            free(pdata);
			return TRUE;
		case UM_INITPOPUP:
			pdata->hWnd = hWnd;
			if (pdata->iSeconds != -1)
				SetTimer(hWnd, TIMER_TO_ACTION, pdata->iSeconds * 1000, NULL);
			break;
		case WM_MOUSEWHEEL:
			if ((short)HIWORD(wParam) > 0 && pdata->firstShowEventData->prev)
			{
				pdata->firstShowEventData = pdata->firstShowEventData->prev;
				PopupUpdate(pdata->hContact, NULL);
			}
			if ((short)HIWORD(wParam) < 0 && pdata->firstShowEventData->next && 
				pdata->countEvent - pdata->firstShowEventData->number >= pdata->pluginOptions->iNumberMsg) 
			{
				pdata->firstShowEventData = pdata->firstShowEventData->next;
				PopupUpdate(pdata->hContact, NULL);
			}
				break;
		case WM_SETCURSOR:
			SetFocus(hWnd);
			break;
		case WM_TIMER:
			if (wParam != TIMER_TO_ACTION)
				break;
			if (pdata->iSeconds != -1)
				KillTimer(hWnd, TIMER_TO_ACTION);
			PopupAct(hWnd, pdata->pluginOptions->maskActTE, pdata);
			break;
		default:
			break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

char* GetPreview(UINT eventType, char* pBlob)
{
    char* comment1 = NULL;
    char* comment2 = NULL;
    char* commentFix = NULL;

    //now get text
    switch(eventType)
    {
		case EVENTTYPE_MESSAGE:
			if (pBlob) comment1 = pBlob;
			commentFix = Translate(POPUP_COMMENT_MESSAGE);
			break;

		case EVENTTYPE_URL:
			if (pBlob) comment2 = pBlob;
			if (pBlob) comment1 = pBlob + strlen(comment2) + 1;
			commentFix = Translate(POPUP_COMMENT_URL);
			break;

		case EVENTTYPE_FILE:
			if (pBlob) comment2 = pBlob + 4;
			if (pBlob) comment1 = pBlob + strlen(comment2) + 5;
			commentFix = Translate(POPUP_COMMENT_FILE);
			break;

		case EVENTTYPE_CONTACTS:
		    commentFix = Translate(POPUP_COMMENT_CONTACTS);
		    break;
		case EVENTTYPE_ADDED:
		    commentFix = Translate(POPUP_COMMENT_ADDED);
		    break;
		case EVENTTYPE_AUTHREQUEST:
		    commentFix = Translate(POPUP_COMMENT_AUTH);
		    break;

//blob format is:
//ASCIIZ    text, usually "Sender IP: xxx.xxx.xxx.xxx\r\n%s"
//ASCIIZ    from name
//ASCIIZ    from e-mail
        case ICQEVENTTYPE_WEBPAGER:
			if (pBlob) comment1 = pBlob;
//			if (pBlob) comment1 = pBlob + strlen(comment2) + 1;
		    commentFix = Translate(POPUP_COMMENT_WEBPAGER);
		    break;
//blob format is:
//ASCIIZ    text, usually of the form "Subject: %s\r\n%s"
//ASCIIZ    from name
//ASCIIZ    from e-mail
        case ICQEVENTTYPE_EMAILEXPRESS:
			if (pBlob) comment1 = pBlob;
//			if (pBlob) comment1 = pBlob + strlen(comment2) + 1;
		    commentFix = Translate(POPUP_COMMENT_EMAILEXP);
		    break;

		default:
			commentFix = Translate(POPUP_COMMENT_OTHER);
   			break;
    }

    if (comment1)
        if (strlen(comment1) > 0)
                return comment1;
    if (comment2)
        if (strlen(comment2) > 0)
                return comment2;

    return commentFix;
}

int PopupUpdate(HANDLE hContact, HANDLE hEvent)
{
	PLUGIN_DATA* pdata;
	DBEVENTINFO dbe;
	EVENT_DATA_EX* eventData;
	char lpzText[MAX_SECONDLINE] = "";
	char timestamp[MAX_DATASIZE] = "";
	char formatTime[MAX_DATASIZE] = "";
	int iEvent = 0;

	pdata = (PLUGIN_DATA*)PopUpList[NumberPopupData(hContact)];
	
	if (hEvent)
	{
		pdata->countEvent++;

		pdata->lastEventData->next = malloc(sizeof(EVENT_DATA_EX));
		pdata->lastEventData->next->prev = pdata->lastEventData;
		pdata->lastEventData = pdata->lastEventData->next;
		pdata->lastEventData->hEvent = hEvent;
		pdata->lastEventData->number = pdata->lastEventData->prev->number + 1;
		pdata->lastEventData->next = NULL;
		if ( pdata->pud && pdata->pud->iSeconds > 1 )
			pdata->pud->iSeconds = pdata->pluginOptions->iDelayMsg;
		//pdata->pud->iSeconds = pdata->pluginOptions->iDelayMsg;
		if (!pdata->pluginOptions->bShowON && pdata->countEvent > pdata->pluginOptions->iNumberMsg && pdata->pluginOptions->iNumberMsg)
			pdata->firstShowEventData = pdata->firstShowEventData->next;
	}

	if (pdata->pluginOptions->bShowHeaders)
		_snprintf(lpzText, sizeof(lpzText), "[b]%s %d[/b]\n", Translate("Number of new message: "), pdata->countEvent);
	
	if (pdata->firstShowEventData != pdata->firstEventData)
			_snprintf(lpzText, sizeof(lpzText), "%s...\n", lpzText);

	eventData = pdata->firstShowEventData;

	while (TRUE)
	{
		if (iEvent)
			eventData = eventData->next;
		iEvent++;
		//get DBEVENTINFO with pBlob if preview is needed (when is test then is off)
		dbe.pBlob = NULL;
		if (pdata->pluginOptions->bPreview && hContact)
		{
			dbe.cbSize = sizeof(dbe);
			dbe.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)eventData->hEvent, 0);
			dbe.pBlob = (PBYTE)malloc(dbe.cbBlob);
			CallService(MS_DB_EVENT_GET, (WPARAM)eventData->hEvent, (LPARAM)&dbe);
		}
		if (pdata->pluginOptions->bShowDate || pdata->pluginOptions->bShowTime)
		{
			strncpy(formatTime,"",sizeof(formatTime));
			if (pdata->pluginOptions->bShowDate)
				strncpy(formatTime, "%Y.%m.%d ", sizeof(formatTime));
			if (pdata->pluginOptions->bShowTime)
				strncat(formatTime, "%H:%M", sizeof(formatTime));
			strftime(timestamp,sizeof(timestamp), formatTime, localtime(&dbe.timestamp));
			_snprintf(lpzText, sizeof(lpzText), "%s[b][i]%s[/i][/b]\n", lpzText, timestamp);
		}
		_snprintf(lpzText, sizeof(lpzText), "%s%s", lpzText, GetPreview(dbe.eventType, dbe.pBlob));
		if (dbe.pBlob)
			free(dbe.pBlob);
		if ((iEvent >= pdata->pluginOptions->iNumberMsg && pdata->pluginOptions->iNumberMsg) || !eventData->next)
			break;
		_snprintf(lpzText, sizeof(lpzText), "%s\n", lpzText);
	}
	if (pdata->pluginOptions->iNumberMsg && pdata->countEvent > pdata->pluginOptions->iNumberMsg)
	{
		if (pdata->lastEventData != eventData)
			_snprintf(lpzText, sizeof(lpzText), "%s\n...", lpzText);
	}
	CallService(MS_POPUP_CHANGETEXT, (WPARAM)pdata->hWnd, (LPARAM)lpzText);
	return 0;
}

int PopupShow(PLUGIN_OPTIONS* pluginOptions, HANDLE hContact, HANDLE hEvent, UINT eventType)
{
    POPUPDATAEX pud;
    PLUGIN_DATA* pdata;
    DBEVENTINFO dbe;
	EVENT_DATA_EX* eventData;
	char* sampleEvent;
	long iSeconds;
  
	//there has to be a maximum number of popups shown at the same time
    if (PopupCount >= MAX_POPUPS)
        return 2;

	//check if we should report this kind of event
    //get the prefered icon as well
	//CHANGE: iSeconds is -1 because I use my timer to hide popup
	switch (eventType)
    {
        case EVENTTYPE_MESSAGE:
                if (!(pluginOptions->maskNotify&MASK_MESSAGE)) return 1;
                pud.lchIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
				pud.colorBack = pluginOptions->bDefaultColorMsg ? 0 : pluginOptions->colBackMsg;
				pud.colorText = pluginOptions->bDefaultColorMsg ? 0 : pluginOptions->colTextMsg;
				pud.iSeconds = -1; 
				iSeconds = pluginOptions->iDelayMsg;
				sampleEvent = Translate("This is a sample message event :-)");
                break;
        case EVENTTYPE_URL:
                if (!(pluginOptions->maskNotify&MASK_URL)) return 1;
                pud.lchIcon = LoadSkinnedIcon(SKINICON_EVENT_URL);
				pud.colorBack = pluginOptions->bDefaultColorUrl ? 0 : pluginOptions->colBackUrl;
				pud.colorText = pluginOptions->bDefaultColorUrl ? 0 : pluginOptions->colTextUrl;
				pud.iSeconds = -1; 
				iSeconds = pluginOptions->iDelayUrl;
				sampleEvent = Translate("This is a sample URL event ;-)");
                break;
        case EVENTTYPE_FILE:
                if (!(pluginOptions->maskNotify&MASK_FILE)) return 1;
                pud.lchIcon = LoadSkinnedIcon(SKINICON_EVENT_FILE);
				pud.colorBack = pluginOptions->bDefaultColorFile ? 0 : pluginOptions->colBackFile;
				pud.colorText = pluginOptions->bDefaultColorFile ? 0 : pluginOptions->colTextFile;
				pud.iSeconds = -1;
				iSeconds = pluginOptions->iDelayFile;
				sampleEvent = Translate("This is a sample file event :-D");
                break;
        default:
                if (!(pluginOptions->maskNotify&MASK_OTHER)) return 1;
       			pud.lchIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
				pud.colorBack = pluginOptions->bDefaultColorOthers ? 0 : pluginOptions->colBackOthers;
				pud.colorText = pluginOptions->bDefaultColorOthers ? 0 : pluginOptions->colTextOthers;
				pud.iSeconds = -1;
				iSeconds = pluginOptions->iDelayOthers;
				sampleEvent = Translate("This is a sample other event ;-D");
       			break;
    }

    //get DBEVENTINFO with pBlob if preview is needed (when is test then is off)
    dbe.pBlob = NULL;
    
	if (pluginOptions->bPreview && hContact)
    {
        dbe.cbSize = sizeof(dbe);
        dbe.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hEvent, 0);
        dbe.pBlob = (PBYTE)malloc(dbe.cbBlob);
        CallService(MS_DB_EVENT_GET, (WPARAM)hEvent, (LPARAM)&dbe);
    }
	
    eventData = (EVENT_DATA_EX*)malloc(sizeof(EVENT_DATA_EX));
	eventData->hEvent = hEvent;
	eventData->number = 1;
	eventData->next = NULL;
	eventData->prev = NULL;

	//set plugin_data ... will be useable within PopupDlgProc
    pdata = (PLUGIN_DATA*)malloc(sizeof(PLUGIN_DATA));
    pdata->eventType = eventType;
    pdata->hContact = hContact;
    pdata->pluginOptions = pluginOptions;
	pdata->countEvent = 1;
	pdata->pud = &pud;
	pdata->iSeconds = iSeconds ? iSeconds : pluginOptions->iDelayDefault;
	pdata->firstEventData = pdata->firstShowEventData = pdata->lastEventData = eventData;
    
	//finally create the popup
	pud.lchContact = hContact;
	pud.PluginWindowProc = (WNDPROC)PopupDlgProc;
    pud.PluginData = pdata;
	
	//if hContact is NULL, then popup is only Test
	if (hContact)
	{
		//get the needed event data
		strncpy(pud.lpzContactName, (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0), MAX_CONTACTNAME);
		strncpy(pud.lpzText, GetPreview(dbe.eventType, dbe.pBlob), MAX_SECONDLINE);
	}
    else
	{
		strncpy(pud.lpzContactName, "Plugin Test", MAX_CONTACTNAME);
		strncpy(pud.lpzText, sampleEvent, MAX_SECONDLINE);
	}
    
	PopupCount++;

	PopUpList[NumberPopupData(NULL)] = pdata;
	//send data to popup plugin
	
	CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&pud, 0);
    if (dbe.pBlob)
        free(dbe.pBlob);

    return 0;
}

int PopupPreview(PLUGIN_OPTIONS* pluginOptions)
{
    PopupShow(pluginOptions, NULL, NULL, EVENTTYPE_MESSAGE);
    PopupShow(pluginOptions, NULL, NULL, EVENTTYPE_URL);
    PopupShow(pluginOptions, NULL, NULL, EVENTTYPE_FILE);
    PopupShow(pluginOptions, NULL, NULL, -1);

    return 0;
}

