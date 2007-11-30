#pragma once
#include "windows.h"
#include <newpluginapi.h>
#include <m_plugins.h>

#define DB_NOHELPERFUNCTIONS
	#include <m_database.h>
#undef DB_NOHELPERFUNCTIONS

#include <m_langpack.h>

#include "m_dbx_tree.h"

extern HINSTANCE   hInstance;
extern PLUGINLINK *pluginLink;