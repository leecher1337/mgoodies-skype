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

#pragma once

#include "dbg.h"
#include "database.h"
//--------------------------------------------------------------------------------------------------

class CEnvironment
{
public:
                        CEnvironment(void);

    const char*         ProtocolName(void) const;
    const CSipRtcTrace& Trace(void) const;
    HMODULE             Instance(void) const;
    CDatabase&          DB(void);

    void                SetProtocolName(const char* protocol);
    void                SetInstance(HMODULE instance);

private:
    //
    // Non-copyable
    //
                        CEnvironment(const CEnvironment&);
    CEnvironment&       operator= (const CEnvironment&);

private:
    const char*         m_protocol;
    CSipRtcTrace        m_trace;
    HMODULE             m_instance;
    CDatabase           m_db;
};
//--------------------------------------------------------------------------------------------------

extern CEnvironment g_env;
//--------------------------------------------------------------------------------------------------
