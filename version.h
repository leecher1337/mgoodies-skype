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

#define SIPRTC_VERSION_V1   0
#define SIPRTC_VERSION_V2   0
#define SIPRTC_VERSION_V3   0
#define SIPRTC_VERSION_V4   11
//--------------------------------------------------------------------------------------------------

#if !defined(_CRT_STRINGIZE)
#define _CRT_STRINGIZE2(x) #x
#define _CRT_STRINGIZE(x) _CRT_STRINGIZE2(x)
#endif
//--------------------------------------------------------------------------------------------------

#define SIPRTC_VERSION_TEXT _CRT_STRINGIZE(SIPRTC_VERSION_V1) ", " \
                            _CRT_STRINGIZE(SIPRTC_VERSION_V2) ", " \
                            _CRT_STRINGIZE(SIPRTC_VERSION_V3) ", " \
                            _CRT_STRINGIZE(SIPRTC_VERSION_V4)
//--------------------------------------------------------------------------------------------------
