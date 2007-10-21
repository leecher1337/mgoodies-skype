Jingle - Jabber plugin
---------------------

CAUTION: THIS IS AN ALPHA STAGE PLUGIN. IT CAN DO VERY BAD THINGS. USE AT YOUR OWN RISK.

This is a plugin for the jabber protocol, to allow it to make voice calls using libjingle. This is a very alpha version, that was tested very little and probabilly contains lots of bugs and TODOs.

It can talk to Google Talk client and other jabber clients that implements the jingle spec.

To make this work you need this mod of jabber that support plugins:
Ansi: http://pescuma.mirandaim.ru/miranda/jabber_mp.zip
Unicode: http://pescuma.mirandaim.ru/miranda/jabber_mpW.zip
With some lucky it will be incorporated in the SVN soon.

This plugin depends on Voice Service, that can be downloaded here:
http://pescuma.mirandaim.ru/miranda/voiceservice
Ansi: http://pescuma.mirandaim.ru/miranda/voiceservice.zip
Unicode: http://pescuma.mirandaim.ru/miranda/voiceserviceW.zip

Also, you need to copy the contents of the following zip to the same folder of miranda.exe:
http://pescuma.mirandaim.ru/miranda/mediastreamer2.zip

This plugin needs Miranda 0.7 to work.

This plugin uses libjingle, code from linphone and a lot of other codecs. Thanks a lot to all the developers of these softwares.

Todo:
- Everything
- A little bit more

Known problems:
- When making a call, takes a few seconds until both parts can hear each other
- It does not take very well going offline while talking (aka crashes)

To report bugs/make suggestions, go to the forum thread: http://forums.miranda-im.org/showthread.php?p=113763
