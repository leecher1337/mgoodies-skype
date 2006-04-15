/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2004 Miranda ICQ/IM project, 
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

Avatar service 

- load and maintain a cache of contact avatars.
- draw avatars to a given target device context
- maintain per protocol fallback images

The avatar service builds on top of Mirandas core bitmap loading service (MS_UTILS_LOADBITMAP).
However, if imgdecoder.dll is installed in mirandas main or Plugins directory, it can be used
to support PNG images. The avatar service loads 32bit PNG images and peforms alpha channel
premultiplication so that these images can be rendered by using the Win32 AlphaBlend() API.

The cache grows on demand only, that is, no avatars are PREloaded. An avatar is only loaded
if a plugin requests this by using the MS_AV_GETAVATAR service. Since avatars may update
asynchronously, the avatar iamge may not be ready when a plugin calls the service. In that
case, an event (ME_AV_AVATARCHANGED) is fired when a contacts avatar changes. This event
is also fired, when a contact avatar changes automatically.

The service takes care about protocol capabilites (does not actively fetch avatars for 
protocols which do not report avatar capabilities via PF4_AVATARS or for protocols which
have been disabled in the option dialog). It also does not actively fetch avatars for
protocols which are in invisible status mode (may cause privacy issues and some protocols
like MSN don't allow any outbound client communication when in invisible status mode).

- TODO
- maintain recent avatars (store the last hashes to avoid re-fetching)
- cache expiration, based on least recently used algorithm.

(c) 2005 by Nightwish, silvercircle@gmail.com

*/

#ifndef _M_AVATARS_H
#define _M_AVATARS_H

#define AVS_BITMAP_VALID 1
#define AVS_BITMAP_EXPIRED 2        // the bitmap has been expired from the cache. (unused, currently.
#define AVS_HIDEONCLIST 4
#define AVS_PREMULTIPLIED 8         // set in the dwFlags member of the struct avatarCacheEntry for 32 bit transparent
                                    // images when loaded with imgdecoder. These images can be rendered transparently
                                    // using the AlphaBlend() API with AC_SRC_ALPHA
#define AVS_PROTOPIC 16             // picture is a protocol picture
#define AVS_CUSTOMTRANSPBKG 32      // Bitmap was changed to set the background color transparent
#define AVS_HASTRANSPARENCY 64      // Bitmap has at least one pixel transparent
#define AVS_OWNAVATAR 128			// is own avatar entry

struct avatarCacheEntry {
    DWORD cbSize;                   // set to sizeof(struct)
    HANDLE hContact;                // contacts handle, 0, if it is a protocol avatar
    HBITMAP hbmPic;                 // bitmap handle of the picutre itself
    DWORD dwFlags;                  // see above for flag values
    LONG bmHeight, bmWidth;         // bitmap dimensions
    DWORD t_lastAccess;            // last access time (currently unused, but plugins should still
                                    // use it whenever they access the avatar. may be used in the future
                                    // to implement cache expiration
    LPVOID lpDIBSection;
    char szFilename[MAX_PATH];      // filename of the avatar (absolute path)
};

typedef struct avatarCacheEntry AVATARCACHEENTRY;

#define AVDRQ_FALLBACKPROTO 1              // use the protocol picture as fallback (currently not used)
#define AVDRQ_FAILIFNOTCACHED 2            // don't create a cache entry if it doesn't already exist. (currently not working)
#define AVDRQ_ROUNDEDCORNER 4              // draw with rounded corners
#define AVDRQ_DRAWBORDER 8                 // draw a border around the picture
#define AVDRQ_PROTOPICT  16                // draw a protocol picture (if available).
#define AVDRQ_HIDEBORDERONTRANSPARENCY 32  // hide border if bitmap has transparency
#define AVDRQ_OWNPIC	64				   // draw own avatar (szProto is valid)

// request to draw a contacts picture. See MS_AV_DRAWAVATAR service description

typedef struct _avatarDrawRequest {
    DWORD  cbSize;                  // set this to sizeof(AVATARDRAWREQUEST) - mandatory, service will return failure code if 
                                    // cbSize is wrong
    HANDLE hContact;                // the contact for which the avatar should be drawn. set it to 0 to draw a protocol picture
    HDC    hTargetDC;               // target device context
    RECT   rcDraw;                  // target rectangle. The avatar will be centered within the rectangle and scaled to fit.
    DWORD  dwFlags;                 // flags (see above for valid bitflags)
    DWORD  dwReserved;              // for future use
    DWORD  dwInternal;              // don't use it
    COLORREF clrBorder;             // color for the border  (used with AVDRQ_DRAWBORDER)
    UCHAR  radius;                  // radius (used with AVDRQ_ROUNDEDCORNER)
    UCHAR  alpha;                   // alpha value for semi-transparent avatars (valid values form 1 to 255, if it is set to 0
                                    // the avatar won't be transparent.
    char   *szProto;                // only used when AVDRQ_PROTOPICT or AVDRQ_OWNPIC is set
} AVATARDRAWREQUEST;

#define INITIAL_AVATARCACHESIZE 300
#define CACHE_GROWSTEP 50

#define AVS_MODULE "AVS_Settings"          // db settings module path
#define PPICT_MODULE "AVS_ProtoPics"   // protocol pictures are saved here

// obtain the bitmap handle of the avatar for the given contact
// wParam = (HANDLE)hContact
// lParam = 0;
// returns: pointer to a struct avatarCacheEntry *, NULL on failure
// if it returns a failure, the avatar may be ready later and the caller may receive
// a notification via ME_AV_AVATARCHANGED
// DONT modify the contents of the returned data structure

#define MS_AV_GETAVATARBITMAP "SV_Avatars/GetAvatar"

// obtain a avatar cache entry for one of my own avatars
// wParam = 0
// lParam = (char *)szProto  (protocol for which we need to obtain the own avatar information)
// returns: pointer to a struct avatarCacheEntry *, NULL on failure
// DONT modify the contents of the returned data structure

#define MS_AV_GETMYAVATAR "SV_Avatars/GetMyAvatar"

// protect the current contact picture from being overwritten by automatic
// avatar updates. Actually, it only backups the contact picture filename
// and will used the backuped version until the contact picture gets unlocked
// again. So this service does not disable avatar updates, but it "fakes"
// a locked contact picture to the users of the GetAvatar service.
// 
// wParam = (HANDLE)hContact
// lParam = 1 -> lock the avatar, lParam = 0 -> unlock

#define MS_AV_PROTECTAVATAR "SV_Avatars/ProtectAvatar"

// set (and optionally protect) a local contact picture for the given hContact
// 
// wParam = (HANDLE)hContact
// lParam = either a full picture filename or NULL. If lParam == NULL, the service
// will open a file selection dialog.

#define MS_AV_SETAVATAR "SV_Avatars/SetAvatar"

// Call avatar option dialog for contact
// 
// wParam = (HANDLE)hContact

#define MS_AV_CONTACTOPTIONS "SV_Avatars/ContactOptions"

// draw an avatar picture
// 
// wParam = 0 (not used)
// lParam = AVATARDRAWREQUEST *avdr
// draw a contact picture to a destination device context. see description of
// the AVATARDRAWREQUEST structure for more information on how to use this
// service.
// return value: 0 -> failure, avatar probably not available, or not ready. The drawing
// service DOES schedule an avatar update so your plugin will be notified by the ME_AV_AVATARCHANGED
// event when the requested avatar is ready for use.

#define MS_AV_DRAWAVATAR "SV_Avatars/Draw"

// fired when the contacts avatar changes
// wParam = hContact
// lParam = struct avatarCacheEntry *cacheEntry
// the event CAN pass a NULL pointer in lParam which means that the avatar has changed,
// but is no longer valid (happens, when a contact removes his avatar, for example).
// DONT DESTROY the bitmap handle passed in the struct avatarCacheEntry *
// 
// It is also possible that this event passes 0 as wParam (hContact), in which case,
// a protocol picture (pseudo - avatar) has been changed. 
 
#define ME_AV_AVATARCHANGED "SV_Avatars/AvatarChanged"

// fired when one of our own avatars was changed
// wParam = (char *)szProto (protocol for which a new avatar was set)
// lParam = AVATARCACHEENTRY *ace (new cache entry, NULL if the new avatar is not valid)
 
#define ME_AV_MYAVATARCHANGED "SV_Avatars/MyAvatarChanged"

#endif
