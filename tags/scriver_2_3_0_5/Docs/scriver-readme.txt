Scriver user's manual & FAQ v1.0
---------------------------------

1. Overview
-----------

Scriver is meant to be a replacement for SRMM plugin. It offers more features, including
tabs, and seamless integration with IEView.

2. Installation
---------------

To install Scriver please copy scriver.dll or scriver_unicode.dll (Unicode version requires Win2k, XP or newer)
into your Miranda's plugins directory.

3. User's manual
----------------

Keyboard shortcuts:
-------------------
ALT+D - user's details
ALT+H - view user's history
ALT+Q - quote
ALT+U - user menu
ALT+S - send
CTRL+TAB, ALT+RIGHT - next tab
CTRL+SHIFT+TAB, ALT+LEFT - previous tab
CTRL+SHIFT+M - switch title bar on/off
CTRL+SHIFT+R - switch RTL mode on/off
CTRL+SHIFT+S - switch status bar on/off
CTRL+SHIFT+T - switch tool bar on/off
CTRL+A - select all
CTRL+L - clear logs
CTRL+W - close tab/window
CTRL+ALT+ENTER - send to all open tabs
SHIFT+ESC - minimize window
ESC - close tab/window

4. FAQ
------
Q: Where can I find new versions of Scriver, report bugs or request new features ?
A: http://developer.berlios.de/projects/mgoodies.

Q: Does Scriver support Unicode ?
A: Yes, it does.

Q: Does Scriver support RTL ?
A: Yes, it does.

Q: What version of Miranda is required to run Scriver ?
A: Scriver requires Miranda IM 0.4.

Q: How to hide avatar on the right hand side without disabling avatar support ?
A: Just set max. avatar height to 0.

Q: Is it possible to send the same message to all contacts in open tabs ?
A: Yes, there is a special shortcut - ctrl+alt+enter.

Q: How can I disable Unicode or change code page ?
A: There is a special Unicode icon in the status bar (U),
   left click the icon to enable/disable Unicode and right click to open code page selection menu.

5. Known Issues
---------------


6. Change Log
-------------
2.3.0.5
* bugfix: crash on exit (patch by borkra)
2.3.0.4
+ "Scriver" is now returned by MS_MSG_GETWINDOWCLASS
* bugfixes: bug #6350
* bugfix: some focus issues
* bugfix: avatar reloading
2.3.0.3
* bugfixes
2.3.0.2
* bugfixes
2.3.0.1
* bugfix: bug #006324
2.3.0.0
+ improved support for Avatar Service plugin
* bugfixes
2.2.9.10 RC1
+ basic support for Avatar Service plugin
* avatar-related code cleanup and improvements
2.2.9.9
+ Always On Top option in system menu
* bug #006247: 'Unread msg' when the window is minimized
2.2.9.8
+ window snapping
* bug #006208: double click does not activate tab
* Send To All shortcut changed to CTRL+SHIFT+ENTER
2.2.9.7
* bug #006176: movable mazimized windows
* bug #006195: disabled text selection
2.2.9.6
+ improved (more bash-like :) history of sent messages
* feature #001675: flashing minimized window
* bug #006163: focus stealing when minimized
2.2.9.5
+ option to switch IEview on and off
* bugfix: bug #006144 autoselection of text
* bugfix: bug #006139 maximized container resizing issue
2.2.9.4
* bugfixes (focus stealing, Unicode nick names)
2.2.9.3
* bugfix
2.2.9.2
+ auto popup option is now set per status
+ option to save drafts
+ vertical maximize (ctrl+click maximize button)
+ support for ME_SMILEYADD_OPTIONSCHANGED and ME_IEVIEW_OPTIONSCHANGED
+ %statusmsg% macro
+ improved message grouping
* log formatting improvements
* bugfixes
2.2.9.1
* bugfixes
2.2.9.0
+ support for Unicode nick names
+ new, experimental RichEdit renderer
+ translation file contributed by Raq
* improved switching to active tab (FR001089)
* fixed problem with MS_MSG_GETWINDOWDATA service (005026)
2.2.2.10
* improved drag&drop tab sorting
* fixed Updater compatibility
2.2.2.9
+ midnight-split message grouping
+ ctrl+F4 closes tabs
+ configurable window title - "hidden" (editable with DBEditor only) WindowTitle setting is used to store the template, %name% and %status% variables are available)
+ "smarter" toolbar auto-sizing
* bugfixes: creating new tabs, showing history in RichEdit mode
* other small changes and improvements
2.2.2.8
+ showing file and URL events in logs
* bugfixes: status changes logging, compatibility with newer RichEdit control, background colours
2.2.2.7
+ option to draw horizontal line between messages
+ FontService support
+ individual background colours in font selection list
* bugfixes
2.2.2.6
* better smiley selection/replacement for metacontact (real protocols' emoticons are used)
* improved tabs dragging
* bugfix: stay minimized and do not bring new tabs to front
* bugfix: message grouping
* bugfix: status bar redrawing
* bugfix: option page
2.2.2.5
+ new option: Do not bring new tabs to front
* bugfixes: SmileAdd support, typos, RichEdit control scrollbar
2.2.2.4
+ pop up and stay minimized
* bugfix: error dialog when messaging window is minimized
* bugfix: message grouping
* option page improvements
2.2.2.3
+ separate background colours for incoming and outgoing messages
* avatar support fixes
2.2.2.2
* a bunch of bugfixes and improvements:
  - simple drag&drop tab sorting
  - easier easy drag :)
  - improved drag&drop file transfer
2.2.2.1
* bugfixes
2.2.2.0 Stable
+ drag&drop file transfer support
+ new shortcut: shift+ESC (minimize window)
* bugfixes: window flashing on receiving new message, multiline tabs, timeout warnings
2.2.1.11 RC3
* bugfixes
2.2.1.10 RC2
+ send message to all open tabs (ctrl+alt+enter)
* code cleanup & gcc compatibility
* bugfixes
2.2.1.9 RC1
+ code page selection (right click Unicode switch)
+ quote button
+ new option: Hide tab bar if there is only one tab
+ new option: Start message text on a new line
* bugfixes
2.2.1.8
+ support for window transparency
+ new shortcuts: ctrl+tab, ctrl+shift+tab
* bugfixes
2.2.1.7
+ input area colors settings (font and background colors)
* errors handling rewritten again
* bugfix: new line in Unicode version
* bugfix: text pasting
2.2.1.6
+ avatar height limits
+ easy drag feature
2.2.1.5
+ context menu in message edit box
+ "sending in progress" indicator (status bar)
+ limit names on tabs (currently the limit is fixed - 20 characters)
+ SmileyAdd support
+ show hide title bar (options page and ctrl+shift+m shortcut)
* bugfixes: error handling
2.2.1.4
+ IcoLib support
+ saving window position and size in tabbed mode
+ improved Unicode support
+ saving per-contact RTL settings (use CTRL+SHIFT+R to switch between LTR and RTL)
+ saving per-contact Unicode settings (use tool bar icon to enable/disable Unicode)
+ option to save per-contact splitter position
+ new shortcut: CTRL+SHIFT+T (switch tool bar on/off)
* bugfixes
2.2.1.3
+ re-written errors handling
+ RTL support (use CTRL+SHIFT+R to switch between LTR and RTL)
+ new shortcuts: CTRL+L (clear log), CTRL+SHIFT+S (turn status bar on/off)
+ avatar is also saved as ContactPhoto/File
+ some IcoLib support
* many bugfixes: send on enter, send on dbl enter, minimized window size etc.
2.2.1.2
+ RichEdit control used as message edit box
+ saving per-contact splitter position
* avatar placement changed
* ALT+S fixed
* window flashing and icons blinking improved
2.2.1.1
+ experimental avatar support
+ ALT+LEFT, ALT+RIGHT shortcuts
* bugfixes
2.2.1.0
* initial version with tabbed mode and some look&feel changes.
