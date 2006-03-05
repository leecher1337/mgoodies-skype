StatusMessageChangeNotify 0.0.1.6
=================================

This plugin shows a popup whenever a contact changes his/her status message.
Also, it keeps a history of all status message changes.

I completely admit it, this plugin is a complete rip-off of the NickChangeNotify (NCN)
plugin by noname. Nothing is original in this code. NCN is released under the GPL, so
this plugin is too. SMCN is based on NCN 0.0.2.7 and contains all the features and fixes
(and bugs) from this version.

This is a beta version of this plugin. I didn't test is a lot. It seems to be working,
it could contain bugs, make Miranda crash or do weird things to the database. I don't know
because I didn't test it. So beware... (this shouldn't be a case anymore as my [slotwin]
mod was heavily tested on polish miranda forum: 
http://miranda.kom.pl/viewtopic.php?t=3644&postdays=0&postorder=asc&start=0 )


Some notes:
 - This plugin requires Miranda IM v0.3.2.0 or higher.
 - This plugin requires PopUp plugins. Be sure to set multi-line and dynamic resize
   popup are ON.
 - To prevent internal logging occurs for all contacts, set the maximun entries to
   0 in the plugin options.
 - Leave the 'log to' edit box empty to disable external logging for all contact.
 - Settings for individual contact can be found in the toolbar of status message history
   window.

 - When connecting, PopUps show only if contact's status message is different then was
   before disconnection. This can be disabled with "Show PopUps when I connect" option.
 - On 'contacts with status messages list':
   - click first column header to group contacts by protocol and sort them by status
     (free for chat, online, occupied, on the phone, away etc.)
   - click it second time to change the order
   - click 3rd time to sort contacts by status (without protocol grouping)
   - click 4th time to change the order

Variables
---------
Note that these variables are case sensitive.
 %D	Day
 %H	Hour (in 24h format)
 %M	Month
 %Y	Year
 %a	AM/PM
 %c	Custom nickname
 %h	Hour (in 12h format)
 %m	Minute
 %n	New status message
 %o	Old status message

 \n	line break
 \t	tab stop

History
-------

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
[*] new translatable strings:

;options window
[Message Window]
[Show status message changes in message window]
[Message cleared]
[Message changed]
;default notifications for message window
[cleared his/her status message]
[changed his/her status message]

Version 0.0.0.2-0.0.0.4 mod by slotwin

[+] GG (Gadu-Gadu) and Yahoo support
[+] new variable: \n line break
[*] no PopUps when disconnecting
[!] bugfix: wrong data read when contact was clearing his/her status message
[!] bugfix: sound when PopUps disabled


Version 0.0.2.7		2005/05/28
 - First version, completely bases on NickChangeNotify 0.0.2.7

Translation
-----------

[PopUp text]
[Contact]
[Old status message]
[New status message
[Status message Changed]
[Enable/Disable internal logging for this contact]
[Enable/Disable external logging]
[Enable/Disable popups for this contact]
[View status message history]

; the following can be found some popup plugins
[Left Click Actions]  (neweventnotify)
[Right Click Actions]  (neweventnotify)
[Dismiss PopUp]  (weather proto)
[Open message window]  (whoisreadingmyawaymsg notify)
[Show user detail]  (weather proto)
[Show user menu]  (weather proto)
[Background colours]  (weather proto)
[Text colours]  (weather proto)
[Use Windows colours]  (newstatusnotify)
[Logging]
[Log to]
[History format]
[Log format]

; == ORIGINAL STRINGS ==
; texts in option pages
[PopUp display time]
[Maximun history entry]

; menu items
[Disable &status message change notification]
[Enable &status message notification]
[View Status Message History]
[Status Message History]

; popup messages
[changes his/her status message to:]

===========================

; the following are the same for most notification plugins (ie. neweventnotify, newstatusnotify)
[General Options]
[Temporarily disable Popups]
[seconds]
[Preview]
[PopUp]

; the following are the same as some text in SRMM plugin
[User Menu]
[View User's Details]
[View User's History]

; this string can also be found in last seen plugin
[Send Instant Message]
