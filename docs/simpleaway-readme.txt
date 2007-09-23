		-================================-
		 SimpleAway plugin for Miranda-IM
		-================================-


Description
-----------
This plugin for Miranda-IM replaces the build-in SRAway module. It provides many usable functions. Some of them are: ability to remember up to 25 recent status messages which you can choose through a list box displayed in the dialog window, new variabls %winampsong%, %rand(arg1,arg2)% and support for "Variables" plugin which allows many more variables, possibility to change the status from within a dialog window, ability to update your status message with title of current song played by winamp if the message contains %winampsong% variable, static status messages for given protocols, predefined status messages and more.


Installation
------------
Unzip simpleaway.dll to the Miranda IM plugin folder or install via Miranda Installer.


Options
-------
Options page Options->Status->Status Messages.


===================
Available Variables:

%winampsong% - replaced by title of the song currently played by winamp (see Tips for more info).
%date% - actual date.
%time% - actual time.
%rand(arg1,arg2)% - gets random number from range arg1 to arg2. arg2 must be greater than arg1 and both args must be integers.
%fortunemsg% - fortune message (FortuneAwayMsg plugin required).
%protofortunemsg% - fortune message for given protocol (FortuneAwayMsg plugin required).
%statusfortunemsg% - fortune message for given status (FortuneAwayMsg plugin required).
===================


Tips
----
Winamp song changes can be checked by SimpleAway. By default SimpleAway checks for new song every 15 secs (you can change the time interval) but only if your status message contains %winampsong% variable. If the song has not been changed your status message stays untouched.

You can use %winampsong% variable to show song titles played in foobar2000 by using Winamp API Emulator which you can find here: http://www.r1ch.net/stuff/foobar/

If you wish to use many other variables, install the "variables" plugin which can be found here: http://www.cs.vu.nl/~pboon/variables.zip

For %*fortunemsg% variables use FortuneAwayMsg plugin: http://www.miranda-im.org/download/details.php?action=viewfile&id=1933 

You can change default SimpleAway icons by downloading and installing IcoLib plugin. Get it here: http://www.miranda-im.org/download/details.php?action=viewfile&id=1895

SimpleAway adds TopToolBar button which allows user to change his global status and message. Grab TopToolBar plugin from here: http://www.miranda-im.org/download/details.php?action=viewfile&id=466


Thanks
------
- Std
- TioDuke
- iENO
- the_leech
- bankrut
- raq
- DJ Lotos
- karaguy
- Nazir (+v on #miranda.kom.pl ;P)
- all other miranda.kom.pl forums and SimpleAway users :)


========================
Base Address: 0x3ab00000
========================


License
-------
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


=====================================================================
Copyright (C) 2004-2005 Mateusz Kwasniewski aka Harven
mailto: harven@users.berlios.de 
http://developer.berlios.de/projects/mgoodies/