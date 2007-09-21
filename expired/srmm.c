/*
Scriver

Copyright 2000-2003 Miranda ICQ/IM project,
Copyright 2005 Piotr Piastucki

all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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
#include "commonheaders.h"

int LoadSendRecvMessageModule(void);
int SplitmsgShutdown(void);
extern void Chat_Load(PLUGINLINK *link);
extern void Chat_Unload();

struct MM_INTERFACE memoryManagerInterface;

PLUGINLINK *pluginLink;
HINSTANCE g_hInst;

PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
#ifdef _UNICODE
	"Scriver (Unicode)",
#else
	"Scriver",
#endif
	PLUGIN_MAKE_VERSION(2, 5, 9, 15),
#ifdef _UNICODE
	"Scriver - send and receive instant messages (Unicode)",
#else
	"Scriver - send and receive instant messages",
#endif
	"Miranda IM Development Team",
	"the_leech@users.berlios.de",
	"Copyright © 2000-2007 Miranda IM Project",
	"http://mgoodies.berlios.de",
	UNICODE_AWARE,
	DEFMOD_SRMESSAGE            // replace internal version (if any)
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hInst = hinstDLL;
	return TRUE;
}

__declspec(dllexport)
	 PLUGININFO *MirandaPluginInfo(DWORD mirandaVersion)
{
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 6, 0, 0))
		return NULL;
	return &pluginInfo;
}

int __declspec(dllexport) Load(PLUGINLINK * link)
{
	pluginLink = link;
	// set the memory manager
	memoryManagerInterface.cbSize = sizeof(struct MM_INTERFACE);
	CallService(MS_SYSTEM_GET_MMI,0,(LPARAM)&memoryManagerInterface);

	Chat_Load(link);
	return LoadSendRecvMessageModule();
}

int __declspec(dllexport) Unload(void)
{
	Chat_Unload();
	return SplitmsgShutdown();
}
