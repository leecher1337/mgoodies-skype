History Events plugin
---------------------

CAUTION: THIS IS BETA QUALITY SOFTWARE. IT CAN ERASE ALL YOUR HISTORY. USE AT YOUR OWN RISK.

This is a service plugin that allows adding/reading new event types to history. The main idea is a service that plugins use to add events to history and that the *srmm plugins can use to show then.

It also provides options to remove events from history in some pre-configured intervals. This part is not well tested and can cause big problems. To erase the old events it will, after 1 min of miranda start, go through all events and check which to remove. This is done slowly and in other thread (to avoid too much CPU usage), but this means that the event may take longer than the selected value to be removed.

It now support Variables plugin. BUT you need to use this mod:
- http://pescuma.mirandaim.ru/miranda/variables.zip
- http://pescuma.mirandaim.ru/miranda/variablesW.zip
it adds an option to use temporary variables. The patch is here (I already sent it to UnregistereD):
- http://pescuma.mirandaim.ru/miranda/variables.patch

This plugin requires at least Miranda 0.7

To report bugs/make suggestions, go to the forum thread: http://forums.miranda-im.org/showthread.php?t=15467

To do:
 - Change some plugins to use this service
 - Add support in *srmm
 - Add support for HTML
 - Add support for color/font formats
