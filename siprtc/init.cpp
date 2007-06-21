/*

SIP RTC Plugin for Miranda IM

Copyright 2007 Paul Shmakov

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

#include "stdafx.h"

#include "module.h"
//--------------------------------------------------------------------------------------------------

CMirandaPluginLink g_pluginLink;
CSipRtcPlugin g_plugin;
//--------------------------------------------------------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID)
{
    return g_plugin.DllMain(hInst, reason);
}
//--------------------------------------------------------------------------------------------------

extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
    if(mirandaVersion < PLUGIN_MAKE_VERSION(0, 4, 3, 0))
    {
        MessageBoxA(0, "The SIP RTC protocol plugin cannot be loaded. It requires Miranda IM 0.4.3 (Unicode) or later.",
            "SIP RTC Protocol Plugin",
            MB_OK | MB_ICONWARNING | MB_SETFOREGROUND | MB_TOPMOST);
        return NULL;
    }

    return g_plugin.MirandaPluginInfo(mirandaVersion);
}
//--------------------------------------------------------------------------------------------------

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
    if(mirandaVersion < PLUGIN_MAKE_VERSION(0, 4, 3, 0))
    {
        MessageBoxA(0, "The SIP RTC protocol plugin cannot be loaded. It requires Miranda IM 0.4.3 (Unicode) or later.",
            "SIP RTC Protocol Plugin",
            MB_OK | MB_ICONWARNING | MB_SETFOREGROUND | MB_TOPMOST);
        return NULL;
    }

    return g_plugin.MirandaPluginInfoEx(mirandaVersion);
}
//--------------------------------------------------------------------------------------------------

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
    return g_plugin.MirandaPluginInterfaces();
}
//--------------------------------------------------------------------------------------------------

extern "C" int __declspec(dllexport) Load(PLUGINLINK* link)
{
    return g_plugin.Load(link);
}
//--------------------------------------------------------------------------------------------------

extern "C" int __declspec(dllexport) Unload(void)
{
    return g_plugin.Unload();
}
//--------------------------------------------------------------------------------------------------
