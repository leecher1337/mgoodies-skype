# Microsoft Developer Studio Project File - Name="scriver" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=scriver - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "scriver.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "scriver.mak" CFG="scriver - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "scriver - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "scriver - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "scriver - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "scriver - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Miranda32/Plugins/scriver", DSEAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "scriver - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /i "../../include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib shlwapi.lib Version.lib /nologo /dll /debug /machine:I386 /out:"../../bin/Release/plugins/scriver.dll"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "scriver - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /i "../../include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib shlwapi.lib Version.lib /nologo /dll /debug /machine:I386 /out:"../../bin/Debug Unicode/plugins/scriver.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "scriver - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "scriver___Win32_Unicode_Release"
# PROP BASE Intermediate_Dir "scriver___Win32_Unicode_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Unicode"
# PROP Intermediate_Dir "Release_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Zi /O1 /I "../../include" /D "UNICODE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_WIN32_IE 0x0500" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /i "../../include" /d "NDEBUG"
# ADD RSC /l 0x809 /i "../../include" /d "NDEBUG" /d "UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib /nologo /dll /pdb:none /machine:I386 /out:"../../bin/release/plugins/scriver.dll"
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib shlwapi.lib Version.lib /nologo /dll /debug /machine:I386 /out:"../../bin/Release Unicode/plugins/scriver.dll"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "scriver - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "scriver___Win32_Unicode_Debug"
# PROP BASE Intermediate_Dir "scriver___Win32_Unicode_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Unicode"
# PROP Intermediate_Dir "Debug_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "UNICODE" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /i "../../include" /d "_DEBUG"
# ADD RSC /l 0x809 /i "../../include" /d "_DEBUG" /d "UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/debug/Plugins/scriver.dll" /pdbtype:sept
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib shlwapi.lib Version.lib /nologo /dll /debug /machine:I386 /out:"../../bin/Debug Unicode/plugins/scriver.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "scriver - Win32 Release"
# Name "scriver - Win32 Debug"
# Name "scriver - Win32 Release Unicode"
# Name "scriver - Win32 Debug Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\cmdlist.c
# End Source File
# Begin Source File

SOURCE=.\globals.c
# End Source File
# Begin Source File

SOURCE=.\msgdialog.c
# End Source File
# Begin Source File

SOURCE=.\msglog.c
# End Source File
# Begin Source File

SOURCE=.\msgoptions.c
# End Source File
# Begin Source File

SOURCE=.\msgs.c
# End Source File
# Begin Source File

SOURCE=.\msgtimedout.c
# End Source File
# Begin Source File

SOURCE=.\msgwindow.c
# End Source File
# Begin Source File

SOURCE=.\srmm.c
# End Source File
# Begin Source File

SOURCE=.\statusicon.c
# End Source File
# Begin Source File

SOURCE=.\utils.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\cmdlist.h
# End Source File
# Begin Source File

SOURCE=.\commonheaders.h
# End Source File
# Begin Source File

SOURCE=.\globals.h
# End Source File
# Begin Source File

SOURCE=.\chat\IcoLib.h
# End Source File
# Begin Source File

SOURCE=.\chat\m_ieview.h
# End Source File
# Begin Source File

SOURCE=.\chat\m_smileyadd.h
# End Source File
# Begin Source File

SOURCE=.\chat\m_uninstaller.h
# End Source File
# Begin Source File

SOURCE=.\msgs.h
# End Source File
# Begin Source File

SOURCE=.\multimon.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\statusicon.h
# End Source File
# Begin Source File

SOURCE=.\utils.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE="..\..\Miranda-IM\res\addcontact.ico"
# End Source File
# Begin Source File

SOURCE=.\res\addcontact.ico
# End Source File
# Begin Source File

SOURCE=..\..\src\res\addcontact.ico
# End Source File
# Begin Source File

SOURCE=.\res\Clock32.ico
# End Source File
# Begin Source File

SOURCE=.\res\Clock8.ico
# End Source File
# Begin Source File

SOURCE=.\res\delete.ico
# End Source File
# Begin Source File

SOURCE=..\..\src\res\delete.ico
# End Source File
# Begin Source File

SOURCE=.\res\Details32.ico
# End Source File
# Begin Source File

SOURCE=.\res\Details8.ico
# End Source File
# Begin Source File

SOURCE="..\..\Miranda-IM\res\downarrow.ico"
# End Source File
# Begin Source File

SOURCE=.\res\Downarrow32.ico
# End Source File
# Begin Source File

SOURCE=.\res\Downarrow8.ico
# End Source File
# Begin Source File

SOURCE=.\res\dragcopy.cur
# End Source File
# Begin Source File

SOURCE=..\..\src\res\dragcopy.cur
# End Source File
# Begin Source File

SOURCE=.\res\emptyblo.ico
# End Source File
# Begin Source File

SOURCE=..\..\src\res\emptyblo.ico
# End Source File
# Begin Source File

SOURCE="..\..\Miranda-IM\res\history.ico"
# End Source File
# Begin Source File

SOURCE=.\res\History32.ico
# End Source File
# Begin Source File

SOURCE=.\res\History8.ico
# End Source File
# Begin Source File

SOURCE="..\..\Miranda-IM\res\hyperlin.cur"
# End Source File
# Begin Source File

SOURCE=.\res\hyperlin.cur
# End Source File
# Begin Source File

SOURCE=..\..\src\res\hyperlin.cur
# End Source File
# Begin Source File

SOURCE=.\res\incoming.ico
# End Source File
# Begin Source File

SOURCE="..\..\Miranda-IM\res\multisend.ico"
# End Source File
# Begin Source File

SOURCE=.\res\Multisend32.ico
# End Source File
# Begin Source File

SOURCE=.\res\Multisend8.ico
# End Source File
# Begin Source File

SOURCE=.\res\notice.ico
# End Source File
# Begin Source File

SOURCE=.\res\outgoing.ico
# End Source File
# Begin Source File

SOURCE=.\res\quote.ico
# End Source File
# Begin Source File

SOURCE="..\..\Miranda-IM\res\rename.ico"
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\res\send8.ico
# End Source File
# Begin Source File

SOURCE=.\res\smiley8.ico
# End Source File
# Begin Source File

SOURCE="..\..\Miranda-IM\res\timestamp.ico"
# End Source File
# Begin Source File

SOURCE="..\..\Miranda-IM\res\typing.ico"
# End Source File
# Begin Source File

SOURCE=.\res\Typing32.ico
# End Source File
# Begin Source File

SOURCE=.\res\Typing8.ico
# End Source File
# Begin Source File

SOURCE=.\res\typingoff.ico
# End Source File
# Begin Source File

SOURCE=.\res\typingon.ico
# End Source File
# Begin Source File

SOURCE=.\res\unicodeoff.ico
# End Source File
# Begin Source File

SOURCE=.\res\unicodeon.ico
# End Source File
# Begin Source File

SOURCE=.\res\unknown.bmp
# End Source File
# Begin Source File

SOURCE="..\..\Miranda-IM\res\viewdetails.ico"
# End Source File
# End Group
# Begin Group "chat"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\chat\chat.h
# End Source File
# Begin Source File

SOURCE=.\chat\chat.rc
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\chat\clist.c
# End Source File
# Begin Source File

SOURCE=.\chat\colorchooser.c
# End Source File
# Begin Source File

SOURCE=.\chat\log.c
# End Source File
# Begin Source File

SOURCE=.\chat\main.c
# End Source File
# Begin Source File

SOURCE=.\chat\manager.c
# End Source File
# Begin Source File

SOURCE=.\chat\message.c
# End Source File
# Begin Source File

SOURCE=.\chat\options.c
# End Source File
# Begin Source File

SOURCE=.\chat\resource.h
# End Source File
# Begin Source File

SOURCE=.\chat\services.c
# End Source File
# Begin Source File

SOURCE=.\chat\tools.c
# End Source File
# Begin Source File

SOURCE=.\chat\window.c
# End Source File
# End Group
# End Target
# End Project
