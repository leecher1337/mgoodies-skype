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

#include "env.h"
//--------------------------------------------------------------------------------------------------

CEnvironment g_env;
//--------------------------------------------------------------------------------------------------

CEnvironment::CEnvironment(void) :
    m_protocol(0),
    m_instance(0)
{
    // Nothing
}
//--------------------------------------------------------------------------------------------------

void CEnvironment::SetProtocolName(const char* protocol)
{
    MTLASSERT(protocol);
    m_protocol = protocol;
}
//--------------------------------------------------------------------------------------------------

void CEnvironment::SetInstance(HMODULE instance)
{
    MTLASSERT(instance);
    m_instance = instance;
}
//--------------------------------------------------------------------------------------------------

const char* CEnvironment::ProtocolName(void) const
{
    MTLASSERT(m_protocol);
    return m_protocol;
}
//--------------------------------------------------------------------------------------------------

const CSipRtcTrace& CEnvironment::Trace(void) const
{
    return m_trace;
}
//--------------------------------------------------------------------------------------------------

HMODULE CEnvironment::Instance(void) const
{
    MTLASSERT(m_instance);
    return m_instance;
}
//--------------------------------------------------------------------------------------------------

CDatabase& CEnvironment::DB(void)
{
    return m_db;
}
//--------------------------------------------------------------------------------------------------
