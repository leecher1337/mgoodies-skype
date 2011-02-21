# Microsoft Developer Studio Project File - Name="listeningto" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=listeningto - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "listeningto.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "listeningto.mak" CFG="listeningto - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "listeningto - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "listeningto - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "listeningto - Win32 Unicode Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "listeningto - Win32 Unicode Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "listeningto - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O1 /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G4 /MT /W3 /GX /Zi /O2 /Ob0 /I "..\..\include" /I "..\..\include_API" /I "sdk" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib shell32.lib wininet.lib gdi32.lib /nologo /base:"0x67100000" /dll /machine:I386 /filealign:0x200
# SUBTRACT BASE LINK32 /pdb:none /map
# ADD LINK32 kernel32.lib user32.lib ole32.lib oleaut32.lib /nologo /base:"0x3EC20000" /dll /map /debug /machine:I386 /out:"..\..\bin\release\Plugins\listeningto.dll" /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /profile /pdb:none

!ELSEIF  "$(CFG)" == "listeningto - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G4 /MT /W3 /GX /O2 /Ob0 /I "../../include" /FR /YX /FD /c
# ADD CPP /nologo /G4 /MTd /W3 /GX /ZI /Od /I "..\..\include" /I "..\..\include_API" /I "sdk" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"..\..bin\release\Plugins\listeningto.dll" /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT BASE LINK32 /profile /pdb:none
# ADD LINK32 kernel32.lib user32.lib ole32.lib oleaut32.lib /nologo /base:"0x3EC20000" /dll /incremental:yes /debug /machine:I386 /out:"..\..\bin\debug\Plugins\listeningto.dll" /pdbtype:sept /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /profile /pdb:none

!ELSEIF  "$(CFG)" == "listeningto - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release_Unicode"
# PROP BASE Intermediate_Dir "Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Unicode"
# PROP Intermediate_Dir "Release_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G4 /MT /W3 /GX /O2 /Ob0 /I "../../include" /I "sdk" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /Fr /YX /FD /c
# ADD CPP /nologo /G4 /MT /W3 /GX /Zi /O2 /Ob0 /I "..\..\include" /I "..\..\include_API" /I "sdk" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib ole32.lib oleaut32.lib /nologo /base:"0x3EC20000" /dll /map /machine:I386 /out:"..\..\bin\release unicode\Plugins\listeningtoW.dll" /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT BASE LINK32 /profile /pdb:none
# ADD LINK32 kernel32.lib user32.lib ole32.lib oleaut32.lib /nologo /base:"0x3EC20000" /dll /map /debug /machine:I386 /out:"..\..\bin\release unicode\Plugins\listeningtoW.dll" /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /profile /pdb:none

!ELSEIF  "$(CFG)" == "listeningto - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Debug_Unicode"
# PROP BASE Intermediate_Dir "Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Debug_Unicode"
# PROP Intermediate_Dir "Debug_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G4 /MTd /W3 /GX /ZI /Od /I "../../include" /I "sdk" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /FR /YX /FD /c
# ADD CPP /nologo /G4 /MTd /W3 /GX /ZI /Od /I "..\..\include" /I "..\..\include_API" /I "sdk" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib ole32.lib oleaut32.lib /nologo /base:"0x3EC20000" /dll /incremental:yes /debug /machine:I386 /out:"..\..\bin\debug unicode\Plugins\listeningtoW.dll" /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT BASE LINK32 /profile /pdb:none
# ADD LINK32 kernel32.lib user32.lib ole32.lib oleaut32.lib /nologo /base:"0x3EC20000" /dll /incremental:yes /debug /machine:I386 /out:"..\..\bin\debug unicode\Plugins\listeningtoW.dll" /pdbtype:sept /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /profile /pdb:none

!ENDIF 

# Begin Target

# Name "listeningto - Win32 Release"
# Name "listeningto - Win32 Debug"
# Name "listeningto - Win32 Release Unicode"
# Name "listeningto - Win32 Debug Unicode"
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\commons.h
# End Source File
# Begin Source File

SOURCE=.\m_listeningto.h
# End Source File
# Begin Source File

SOURCE=..\utils\mir_buffer.h
# End Source File
# Begin Source File

SOURCE=..\utils\mir_icons.h
# End Source File
# Begin Source File

SOURCE=..\utils\mir_memory.h
# End Source File
# Begin Source File

SOURCE=..\utils\mir_options.h
# End Source File
# Begin Source File

SOURCE=.\music.h
# End Source File
# Begin Source File

SOURCE=.\options.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\players\TEventHandler.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\listening_to.ico
# End Source File
# Begin Source File

SOURCE=.\res_vc6.rc
# End Source File
# Begin Source File

SOURCE=.\res\ttb_disabled.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ttb_enabled.bmp
# End Source File
# End Group
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\listeningto.cpp
# End Source File
# Begin Source File

SOURCE=..\utils\mir_icons.cpp
# End Source File
# Begin Source File

SOURCE=..\utils\mir_options.cpp
# End Source File
# Begin Source File

SOURCE=.\music.cpp
# End Source File
# Begin Source File

SOURCE=.\options.cpp
# End Source File
# End Group
# Begin Group "Players"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\players\foobar.cpp
# End Source File
# Begin Source File

SOURCE=.\players\foobar.h
# End Source File
# Begin Source File

SOURCE=.\players\generic.cpp
# End Source File
# Begin Source File

SOURCE=.\players\generic.h
# End Source File
# Begin Source File

SOURCE=.\players\itunes.cpp
# End Source File
# Begin Source File

SOURCE=.\players\itunes.h
# End Source File
# Begin Source File

SOURCE=.\players\mswlm.cpp
# End Source File
# Begin Source File

SOURCE=.\players\mswlm.h
# End Source File
# Begin Source File

SOURCE=.\players\mswmp.cpp
# End Source File
# Begin Source File

SOURCE=.\players\mswmp.h
# End Source File
# Begin Source File

SOURCE=.\players\player.cpp
# End Source File
# Begin Source File

SOURCE=.\players\player.h
# End Source File
# Begin Source File

SOURCE=.\players\watrack.cpp
# End Source File
# Begin Source File

SOURCE=.\players\watrack.h
# End Source File
# Begin Source File

SOURCE=.\players\winamp.cpp
# End Source File
# Begin Source File

SOURCE=.\players\winamp.h
# End Source File
# End Group
# Begin Group "Docs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Docs\langpack_listeningto.txt
# End Source File
# Begin Source File

SOURCE=.\Docs\listeningto_changelog.txt
# End Source File
# Begin Source File

SOURCE=.\Docs\listeningto_readme.txt
# End Source File
# Begin Source File

SOURCE=.\Docs\listeningto_version.txt
# End Source File
# Begin Source File

SOURCE=.\Docs\readme_players.txt
# End Source File
# End Group
# End Target
# End Project
