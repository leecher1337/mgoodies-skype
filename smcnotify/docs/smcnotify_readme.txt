Status Message Change Notify plugin for Miranda IM
==================================================

This plugin checks if any contact on your list changes his/her status message. Then it can
perform some action:
- notify you about the change via popup (PopUp plugin required),
- store old status message to keep history of all the changes ( accesable through User
  Details dialog),
- if the message window is open show the change in there by adding an event to database,
- log changes to a file.

You can customize the text of every notification using some variables (see below).

Other features:
- a customizable window listing all the contacts that have status message set,
- an item in contact menu 'Go To URL in Status Message' if the status message contains
  a link,
- Unicode support,
- Updater support,
- Translation support,

Variables
=========
You can use those with any customizable template in SMCNotify options.
Note that variables are case sensitive.

 %n	New status message
 %o	Old status message
 %c	Custom nickname
 \n	line break
 \t	tab stop

 %Y	Year
 %M	Month
 %D	Day
 %H	Hour (in 24h format)
 %h	Hour (in 12h format)
 %a	AM/PM
 %m	Minutes
 %s	Seconds

List Contacts with Status Message
=================================
You can change the color of the text and background and choose an image to be dsplayed in
the back of the list. Columns are sortable.
The list doesn't refresh. You have to close it and open again, sorry.
The first column (Protocol) is sortable in 4 ways:
- contacts are sorted by status and then grouped by protocol they use,
- reversed order of the previous one,
- contacts are sorted by status only,
- reversed order of the previous one.

Upgrading from version 0.0.3.5 or below
=======================================
Because the code was rewritten and all the setting names in db were changed you have to
configure the plugin from the begining. If you feel comfortable with using DBEditor plugin
there are some ways to reuse old data:

[Setting/StatusMsgChangeNotify]
This module is not used any more. You can note down some setting (like colors, templates)
and use them while configuring pluging through options page. Than delate this module.

Reuse old per-contact settings
Choose 'Actions->Search and Replace' from the top menu; in the dialogbox do the following:
Serch For/Text: put 'smcn'; check only those options: Case Sensitive, Exact Match,
Setting Name; Replace With/Text: put 'SMCNotify'; hit Replace.

Reuse old per-contact status message history
Choose 'Actions->Search and Replace' from the top menu; in the dialogbox do the following:
Serch For/Text: put 'StatusMsgChangeNotify'; check only those options: Case Sensitive,
Exact Match, Module Name; Replace With/Text: put 'SMCNotify'; hit Replace.

Hidden settings
===============
[Settings/SMCNotify/IgnoreAfterStatusChange]
BYTE set it to 1 if you're using NesStatusMessage plugin with option
'Read status message' checked. This eliminates some double popups for the same contact.
Note that this doesn't always work because the notifications come in random order.

[Settings/SMCNotify/IgnoreTlenAway]
BYTE oroginal Tlen client adds timestamp in the format '[hh:mm DD.MM]' when going Away.
If you set this key to 1 it will ignore those changes.

[<contact>/SMCNotify/HistoryMax]
DWORD set per-contact max number of status messages it should keep in history.

Development
===========
The project was started by daniel who simply changed some code of NickChangeNotify plugin
by noname. The it was carried on by slotwin with a lot of help from pescuma.

Source code
===========
If the source code wasn't published with the plugin you can find it in SVN repository on
mgoodies project page:
http://mgoodies.berlios.de

Copyright and license
=====================
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

History
=======
Version 0.0.3.17 beta

[+] You can use %c with log filename to append nicknames
[+] New option: Log in Ascii

Version 0.0.3.15 beta

[*] Code rewritten almost from scratch
[+] Unicode support
[+] Updater support
[*] Options rearrange

[I've never kept track of the changes and new features I inplement, sorry.]

Version 0.0.1.7

[+] OSD plugin support
[+] contact item menu to quick disable/enable PopUps for this contact
[*] better parsing of URL in status message, it can be surrounded with text
    i.e. "blah blah www.myhomepage.com blah"


Version 0.0.1.6 mod by slotwin

[*] hell lot of changes, here are major ones:
[+] 2 option windows - in 'PopUps' and 'Status' group
[+] main menu item and toptoolbar button to list all contacts with status messages
    in a new window:
    - customizable (text colour, background colour or image)
    - you can sort results by nick or proto/status
    - double-left click to open message window
    - right click for contact menu
    - store size, columns width and last sorting after closing
[+] "Go To URL in Status Message" contact menu item


Version 0.0.1.1 mod by slotwin

[+] empty status message displayed as "<empty>" (it's translatable)
[!] minor options window glitches

Version 0.0.1.0 mod by slotwin

[+] new option: Show PopUps when I connect
[+] new option: Show status message changes in message window (thanks TheLeech)
[+] new variable: \t tab stop
[*] some fixes with formatting log and PopUp text, multiline status messages
[*] new translatable strings


Version 0.0.0.2-0.0.0.4 mod by slotwin

[+] GG (Gadu-Gadu) and Yahoo support
[+] new variable: \n line break
[*] no PopUps when disconnecting
[!] bugfix: wrong data read when contact was clearing his/her status message
[!] bugfix: sound when PopUps disabled


Version 0.0.2.7		2005/05/28

 - First version, completely bases on NickChangeNotify 0.0.2.7
