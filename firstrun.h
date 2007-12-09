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
//--------------------------------------------------------------------------------------------------

class CFirstRun
{
public:
                        CFirstRun(void);

private:
    void                InstallIconPack(void);
};
//--------------------------------------------------------------------------------------------------

inline CFirstRun::CFirstRun(void)
{
    if(g_env.DB().GetMySettingBool("FirstRun", true))
    {
        g_env.DB().WriteMySettingBool("FirstRun", false);

        InstallIconPack();
    }
}
//--------------------------------------------------------------------------------------------------

inline void CFirstRun::InstallIconPack(void)
{
    const char* iconsFileName = "proto_siprtc.dll";

    bool iconsFileFound = false;

    char fileName[MAX_PATH] = { 0 };
    const unsigned fileNameSize = sizeof(fileName) / sizeof(fileName[0]);
    if(GetModuleFileNameA(g_env.Instance(), fileName, fileNameSize) > 0)
    {
        char* slash = strrchr(fileName, '\\');
        if(slash)
        {
            MTLVERIFY(S_OK == StringCchCopyA(slash + 1, fileNameSize - (slash - fileName) - 1,
                iconsFileName));
            iconsFileFound = INVALID_FILE_ATTRIBUTES != GetFileAttributesA(fileName);
        }
    }

    if(!iconsFileFound && GetModuleFileNameA(0, fileName, fileNameSize) > 0)
    {
        char* slash = strrchr(fileName, '\\');
        if(slash)
        {
            MTLVERIFY(S_OK == StringCchPrintfA(slash + 1, fileNameSize - (slash - fileName) - 1,
                "Icons\\%s", iconsFileName));
            iconsFileFound = INVALID_FILE_ATTRIBUTES != GetFileAttributesA(fileName);
        }
    }

    if(iconsFileFound)
    {
        struct StatusResId
        {
            int status;
            int resId;
        };

        StatusResId map[] = {
            { ID_STATUS_OFFLINE,    105 },
            { ID_STATUS_ONLINE,     104 },
            { ID_STATUS_AWAY,       128 },
            { ID_STATUS_NA,         131 },
            { ID_STATUS_OCCUPIED,   159 },
            { ID_STATUS_INVISIBLE,  130 },
            { ID_STATUS_ONTHEPHONE, 1002 },
            { ID_STATUS_OUTTOLUNCH, 1003 }
        };

        for(unsigned i = 0; i < sizeof(map) / sizeof(map[0]); ++i)
        {
            char param[32];
            MTLVERIFY(S_OK == StringCbPrintfA(param, sizeof(param), "%s%d",
                g_env.ProtocolName(), map[i].status));

            char value[2 * MAX_PATH];
            MTLVERIFY(S_OK == StringCbPrintfA(value, sizeof(value), "%s,-%d",
                fileName, map[i].resId));

            DBWriteContactSettingString(0, "Icons", param, value);
        }
    }
}
//--------------------------------------------------------------------------------------------------
