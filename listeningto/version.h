/* 
ListeningTo plugin for Miranda IM
==========================================================================
Copyright	(C) 2005-2011 Ricardo Pescuma Domenecci
			(C) 2010-2011 Merlin_de

PRE-CONDITION to use this code under the GNU General Public License:
 1. you do not build another Miranda IM plugin with the code without written permission
    of the autor (peace for the project).
 2. you do not publish copies of the code in other Miranda IM-related code repositories.
    This project is already hosted in a SVN and you are welcome to become a contributing member.
 3. you do not create listeningTo-derivatives based on this code for the Miranda IM project.
    (feel free to do this for another project e.g. foobar)
 4. you do not distribute any kind of self-compiled binary of this plugin (we want continuity
    for the plugin users, who should know that they use the original) you can compile this plugin
    for your own needs, friends, but not for a whole branch of people (e.g. miranda plugin pack).
 5. This isn't free beer. If your jurisdiction (country) does not accept
    GNU General Public License, as a whole, you have no rights to the software
    until you sign a private contract with its author. !!!
 6. you always put these notes and copyright notice at the beginning of your code.
==========================================================================

in case you accept the pre-condition,
this is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef __M_VERSION_H__
#define __M_VERSION_H__

#define __MAJOR_VERSION   0
#define __MINOR_VERSION   3
#define __RELEASE_NUM     1	// due to beta builders
#define __BUILD_NUM       0	// due to beta builders

#define __STRINGIFY_(x) #x
#define __STRINGIFY(x)  __STRINGIFY_(x)

#define __FILEVERSION_STRING      __MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_STRING_DOTS __MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __VERSION_STRING  __STRINGIFY(__FILEVERSION_STRING_DOTS)
#define __VERSION_DWORD   PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM)

#define __SHORT_DESC  "Handle listening information to/for contacts"
#define __DESC        ""
#define __AUTHOR      "Ricardo Pescuma Domenecci, Merlin_de"
#define __AUTHOREMAIL ""
#define __COPYRIGHT   "© 2006-2010 Ricardo Pescuma Domenecci"
#define __AUTHORWEB   "http://pescuma.org/miranda/listeningto"
#ifndef MIID_LISTENINGTO
#define MIID_LISTENINGTO { 0x1fc1efa, 0xaa9f, 0x461b, { 0x92, 0x69, 0xaf, 0x66, 0x6b, 0x89, 0x31, 0xee } }
#endif


#ifdef _WIN64	//TODO get file listing details for x64	(and update #define)
 #ifdef _UNICODE
  #define __UPDATER_DOWNLOAD_ID	????
  #define __PLUGIN_DISPLAY_NAME	"ListeningTo (Unicode x64)" //ensure plugin shortName matches file listing! <title>ListeningTo (Unicode x64)</title>
  #define __PLUGIN_FILENAME		"listeningtoW.dll"
  #define __UPDATER_BETA_URL	"http://pescuma.org/miranda/listeningto64.zip"
//#define __UPDATER_UID			{ 0x614e00b5, 0x2571, 0x46b5, { 0xb2, 0x1d, 0xed, 0xaf, 0xa4, 0xa9, 0xa2, 0x4b } } // {614E00B5-2571-46b5-B21D-EDAFA4A9A24B}
#endif
#else
 #ifdef _UNICODE
  #define __UPDATER_DOWNLOAD_ID	3693
  #define __PLUGIN_DISPLAY_NAME	"ListeningTo (Unicode)"	//ensure plugin shortName matches file listing! <title>ListeningTo (Unicode)</title>
  #define __PLUGIN_FILENAME		"listeningtoW.dll"
  #define __UPDATER_BETA_URL	"http://pescuma.org/miranda/listeningtoW.zip"
//#define __UPDATER_UID			{ 0xf981f3f5, 0x35a, 0x444f, { 0x98, 0x92, 0xca, 0x72, 0x2c, 0x19, 0x5a, 0xda } } // {F981F3F5-035A-444f-9892-CA722C195ADA}
 #else
  #define __UPDATER_DOWNLOAD_ID	3692
  #define __PLUGIN_DISPLAY_NAME	"ListeningTo (Ansi)"	//ensure plugin shortName matches file listing! <title>ListeningTo (Ansi)</title>
  #define __PLUGIN_FILENAME		"listeningto.dll"
  #define __UPDATER_BETA_URL	"http://pescuma.org/miranda/listeningto.zip"
//#define __UPDATER_UID			{ 0xa4a8ff7a, 0xc48a, 0x4d2a, { 0xb5, 0xa9, 0x46, 0x46, 0x84, 0x43, 0x26, 0x3d } } // {A4A8FF7A-C48A-4d2a-B5A9-46468443263D}
 #endif
#endif
#define   __UPDATER_BETA_VERPRE	"ListeningTo "
#define   __UPDATER_BETA_VERURL	"http://pescuma.org/miranda/listeningto_version.txt"
#define   __UPDATER_BETA_CHLOG	"http://pescuma.org/miranda/listeningto#Changelog"

#endif // __M_VERSION_H__
