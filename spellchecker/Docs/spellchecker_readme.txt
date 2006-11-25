Spell Checker plugin
--------------------

CAUTION: THIS IS AN ALPHA STAGE PLUGIN. IT CAN DO VERY BAD THINGS. USE AT YOUR OWN RISK.

This is a spell checker plugin that uses Hunspell to do the dirty work. Hunspell is the spell checker used by OpenOffice, so it should have a good range of dictionaries.

For message window plugins (let's call them all *srmm) it works transparently if those plugins implement the new notification API and have a richedit in the input area. Latest version (from build #0.7) of tabSRMM and scriver already do so. SRMM plugin don't have a rich edit, so you need this moded version:
Ansi: http://pescuma.mirandaim.ru/miranda/srmm.zip
Unicode: http://pescuma.mirandaim.ru/miranda/srmmW.zip
Patch: http://pescuma.mirandaim.ru/miranda/srmm.spellchecker.diff

For other plugins, it works throught providing 3 serices for them to interact:
- one to add handling of a rich edit control
- one to remove it
- one to append options needed to a popup menu and show it
These plugins have to call this services in other for the magic to happen. So, things like when the popup menu is show are handled by the calling plugin.

The dictionaries: it uses hunspell dictionaries. Each is a couple of files with the name beeing the language and the extensions .dic and .aff. Both need to be inside the dir <Miranda Path>\Dictionaries (of a custom folder if folders plugin is installed). You can download then at: http://wiki.services.openoffice.org/wiki/Dictionaries.
PS: This path is read only at startup, so changing it needs a restart of miranda.

It has an options page to set the default dictionary and some other options. It is at Message Sessions/Spell Checker

Also, new versions support showing flags to represent the dicts. An inicial set of flags are at http://pescuma.mirandaim.ru/miranda/flags.zip. These must be copied to <miranda path>\Icons\Flags in order to spell checker to find then. Suppport for it will be extended in next vers. Please note that they need a lot of renaming, so if you find a wrong name, please post it here. The flag icon has to have the same name of the dict with .ico as extension. For example: pt_BR.ico

Many thanks to the Hunspell team and to Vladimir Vainer that made an initial version of the plugin. And thanks to the famfamfam.com site for the icons I'm using for the flags.

To report bugs/make suggestions, go to the forum thread: http://forums.miranda-im.org/showthread.php?t=11555