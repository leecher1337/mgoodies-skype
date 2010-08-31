/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2010 Michael "Protogenes" Kunz,

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

#pragma once

#include <vector>
#include <time.h>
#include "Interface.h"

class CLogger
{
	public:
		enum TLevel
		{
			logNOTICE,
			logWARNING,
			logERROR,
			logCRITICAL
		};

		CLogger();
		virtual ~CLogger();

		virtual void Append(TLevel Level, const TCHAR * Message, ...);

		virtual TLevel ShowMessage(TLevel CanAsyncTill = logERROR);

		static CLogger & Instance()
			{	return _Instance; };

	protected:
		std::vector<TCHAR *> m_Messages;
		size_t m_Length;
		TLevel m_Level;

		static void MessageBoxAsync(void * MsgBoxParams);
	private:
		static CLogger _Instance;

};
