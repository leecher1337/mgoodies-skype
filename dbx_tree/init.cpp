#include <windows.h>
#include "Interface.h"
#include "DatabaseLink.h"

HINSTANCE  hInstance = NULL;

static const DWORD gVersion = 0x00000001;
static const DWORD gMinMirVer = 0x00080000;
static const MUUID gInterfaces[] = {MIID_DATABASE, MIID_LAST};
// {28F45248-8C9C-4bee-9307-7BCF3E12BF99}
static const MUUID gGUID = 
{ 0x28f45248, 0x8c9c, 0x4bee, { 0x93, 0x07, 0x7b, 0xcf, 0x3e, 0x12, 0xbf, 0x99 } };


static PLUGININFOEX gPluginInfoEx = {
	sizeof(PLUGININFOEX),
	"Miranda tree database driver",
	gVersion,
	"Provides extended Miranda database support: global settings, entities, history, settings per entity - build " __DATE__ " @ " __TIME__,
	"Michael \"Protogenes\" Kunz",
	"Michael.Kunz@2005.tu-chemnitz.de",
	"2007, 2008 Michael Kunz",
	"",
	UNICODE_AWARE,
	DEFMOD_DB,
  gGUID
};

extern "C" __declspec(dllexport) DATABASELINK* DatabasePluginInfo(void * Reserved)
{
	return &gDBLink;
}

extern "C" __declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD MirandaVersion)
{
	if (MirandaVersion < gMinMirVer)
	{
		MessageBox( 0, "The dbx_tree plugin cannot be loaded. It requires Miranda IM 0.8.0.0 or later.", "dbx_tree Plugin", MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST );
		return NULL;
	}
	return &gPluginInfoEx;
}

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return gInterfaces;
}

extern "C" __declspec(dllexport) int Load(PLUGINLINK * Link)
{
	return 1;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}


BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved)
{
	hInstance = hInstDLL;
	return TRUE;
}