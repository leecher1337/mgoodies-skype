

   SIP RTC protocol for Miranda IM

   Version 0.0.0.9

   Copyright (c) 2007 Paul Shmakov



   Many thanks go to Alexey Potapochkin for hosting!


PLEASE NOTE #1

     This plugin is in an early alpha state now. A lot of things are not
     implemented. And, for sure, there're bugs.

     Please feel free to report bugs and request features.
     Any feedback is appreciated!

     Here you can find a discussion:
     http://forums.miranda-im.org/showthread.php?t=9282

     You can contact me directly - paul.shmakov@gmail.com, ICQ 141028981

PLEASE NOTE #2

    There're currently 2 SIP protocol plugins based on Microsoft RTC API
    for now - this one, and another one by TOXIC.

    Please, don't use them together in the same instance of Miranda IM.


INSTALLATION REQUIREMENTS

     - Latest UNICODE build of Miranda IM
     - Windows 2000, Windows XP, or Windows 2003
     - Microsoft RTC (Real-time Communications) 1.3
         If you have Windows Messenger 5.1 or Office Communicator installed,
         than you most likely already have it.
         Otherwise, you can download it from here:
           http://www.potapochkin.ru/files/RtcApiSetup.msi

INSTALLATION

    Just copy the files to Plugins folder and restart the Miranda IM.


CHANGE LOG

    0.0.0.9
        - Miranda IM 0.7/0.8 support
        - No more errors when the plugin is disabled
        - Typing notifications are sent only after the first message in session

    0.0.0.8
        - IM fixes
        - Search by SIP Address (URI) always finds buddies
          This was made especially for SIP servers that don't support
          searching (The same behavior was available by holding Ctrl during search)
        - Popups are used to annoy people with connection errors
        - Fingerprint support
        - Connecting icon support

    0.0.0.7
        - Fix: sometimes messages from Windows Messenger or Office Communicator
          were not received
        - Requires RTC 1.3
        - Fix: "The Authentication type requested is not supported" error with
          Gizmo network.
        - Fix: Transport wasn't saved in the Options page.
        - Fix: the siprtc_XXX.xml and SipRtc.log files' location changes randomly
        - Fix: Updater copies the files into the correct dirs
        - Source code released
        - More logging

    0.0.0.6
        - Miranda and SIP plugin versions are displayed on the Notes page of
          the User Details dialog
        - Bug fixes
        - Updater support

    0.0.0.5
        - Bug fixes
        - Application sharing & Whiteboard sessions
        - Icons

    0.0.0.4
        - Bug fixes

    0.0.0.3
        - Bug fixes

    0.0.0.2
        - Accepts Application sharing & Whiteboard sessions
        - Authentication support
        - Visibility
        - Options page

    0.0.0.1
        - Initial release

KNOWN ISSUES

    1. Sometimes messages to a Office Communicator 2005 timeout when Communicator's
       status is Away or NA.
       Don't know how to fix it. Moreover, Windows Messenger also fails to  send  a
       message. That means, I guess, that the problem is somewhere  inside  of  RTC
       API or in the Office Communicator 2005.

    2. Sometimes Whiteboard launches instead of Application Sharing

    3. Application Sharing and Whiteboard session are accepted automatically

    4. Password stored in clear text

    5. CommunigatePro problems (some users appear offline, messages timeout)

    6. Problem with clist_mw: when SIP is the only protocol loaded in Miranda,
       statuses are "stuck" in the status menu.
