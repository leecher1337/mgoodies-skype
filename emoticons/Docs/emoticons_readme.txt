Emoticons plugin
----------------

CAUTION: THIS IS AN ALPHA STAGE PLUGIN. IT CAN DO VERY BAD THINGS. USE AT YOUR OWN RISK.

This is a plugin to replace emoticons in message windows. It support replacement in the log and in message entry area. Animated gifs are supported in both areas.

It is based in two concepts:

1. The emoticons configuration is per protocol, and comes bundled with the plugin. They reference emoticons by a set of pre-configured names (they can't create new names) and set the shortcut and names for the emoticons the protocol support.
They are *.emo files and must be inside <Miranda>\Plugins\Emoticons
Currently I created a pack for MSN and ICQ. I intend to do yahoo and google talk ones too. If someone knows where I can find a list of default emoticons for other protocols, please tell me.

2. Emoticon packs, containing the emoticons to show, only contain the images and a .mep file describing the creator. 
Each one must be a folder inside Customize\Emoticons, with the images beeing the pre-configured name of the smiley. Currently gif (included animated ones), png and jpeg images are supported.
The name of the pack is the name of the folder, and a file <Pack name>.mep can exist inside the folder, containing this entries (all of then are optional):
Name: <Full name of the pack>
Creator: <Artist name>
Updater URL: <URL for updating this pack - not yet implemented>

A default pack, containing Tango Emoticons (made by Hylke) is packed and can be used as an example.

Currently there is a lot more work to be done on this plugin, but I wanted to show an initial version, so I could get some feedback.

Thanks to FYR for the AniSmiley plugin (which this plugin originally used) and to Hylke for the nice emoticons.

To report bugs/make suggestions, go to the forum thread: http://forums.miranda-im.org/showthread.php?t=17210


TODO:
- Per protocol pack
- Support for custom smileys
- Updater support for Emoticon Packs


