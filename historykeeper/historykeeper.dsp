# Microsoft Developer Studio Project File - Name="historykeeper" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=historykeeper - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "historykeeper.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "historykeeper.mak" CFG="historykeeper - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "historykeeper - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "historykeeper - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "historykeeper - Win32 Unicode Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "historykeeper - Win32 Unicode Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "historykeeper - Win32 Release"

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
# ADD CPP /nologo /G4 /MT /W3 /GX /O2 /Ob0 /I "../../include" /I "sdk" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fr /YX /FD /c
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
# ADD LINK32 kernel32.lib user32.lib /nologo /base:"0x3EC20000" /dll /map /machine:I386 /out:"..\..\bin\release\Plugins\historykeeper.dll" /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /profile /pdb:none

!ELSEIF  "$(CFG)" == "historykeeper - Win32 Debug"

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
# ADD CPP /nologo /G4 /MTd /W3 /GX /ZI /Od /I "../../include" /I "sdk" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"..\..bin\release\Plugins\historykeeper.dll" /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT BASE LINK32 /profile /pdb:none
# ADD LINK32 kernel32.lib user32.lib /nologo /base:"0x3EC20000" /dll /incremental:yes /debug /machine:I386 /out:"..\..\bin\debug\Plugins\historykeeper.dll" /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /profile /pdb:none

!ELSEIF  "$(CFG)" == "historykeeper - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Unicode_Debug"
# PROP BASE Intermediate_Dir "Unicode_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Unicode_Debug"
# PROP Intermediate_Dir "Unicode_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G4 /MTd /W3 /GX /ZI /Od /I "../../include" /FR /YX /FD /c
# ADD CPP /nologo /G4 /MTd /W3 /GX /ZI /Od /I "../../include" /I "sdk" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x32100000" /dll /incremental:yes /debug /machine:I386 /out:"..\..\bin\debug\Plugins\historykeeper.dll" /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT BASE LINK32 /profile /pdb:none
# ADD LINK32 kernel32.lib user32.lib /nologo /base:"0x3EC20000" /dll /incremental:yes /debug /machine:I386 /out:"..\..\bin\debug unicode\Plugins\historykeeperW.dll" /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /profile /pdb:none

!ELSEIF  "$(CFG)" == "historykeeper - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Unicode_Release"
# PROP BASE Intermediate_Dir "Unicode_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Unicode_Release"
# PROP Intermediate_Dir "Unicode_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G4 /MT /W3 /GX /O2 /Ob0 /I "../../include" /Fr /YX /FD /c
# ADD CPP /nologo /G4 /MT /W3 /GX /O2 /Ob0 /I "../../include" /I "sdk" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x32100000" /dll /map /machine:I386 /out:"..\..\bin\release\Plugins\historykeeper.dll" /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT BASE LINK32 /profile /pdb:none
# ADD LINK32 kernel32.lib user32.lib /nologo /base:"0x3EC20000" /dll /map /machine:I386 /out:"..\..\bin\release unicode\Plugins\historykeeperW.dll" /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /profile /pdb:none

!ENDIF 

# Begin Target

# Name "historykeeper - Win32 Release"
# Name "historykeeper - Win32 Debug"
# Name "historykeeper - Win32 Unicode Debug"
# Name "historykeeper - Win32 Unicode Release"
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\commons.h
# End Source File
# Begin Source File

SOURCE=..\utils\ContactAsyncQueue.h
# End Source File
# Begin Source File

SOURCE=.\m_historykeeper.h
# End Source File
# Begin Source File

SOURCE=..\utils\mir_dblists.h
# End Source File
# Begin Source File

SOURCE=..\utils\mir_memory.h
# End Source File
# Begin Source File

SOURCE=..\utils\mir_options.h
# End Source File
# Begin Source File

SOURCE=.\options.h
# End Source File
# Begin Source File

SOURCE=.\popup.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\nick_change.ico
# End Source File
# Begin Source File

SOURCE=.\res\nickname_change.ico
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\res\sm_change.ico
# End Source File
# Begin Source File

SOURCE=.\res\status_change.ico
# End Source File
# End Group
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\config.cpp
# End Source File
# Begin Source File

SOURCE=..\utils\ContactAsyncQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\historykeeper.cpp
# End Source File
# Begin Source File

SOURCE=..\utils\mir_dblists.cpp
# End Source File
# Begin Source File

SOURCE=..\utils\mir_memory.cpp
# End Source File
# Begin Source File

SOURCE=..\utils\mir_options.cpp
# End Source File
# Begin Source File

SOURCE=.\options.cpp
# End Source File
# Begin Source File

SOURCE=.\popup.cpp
# End Source File
# End Group
# Begin Group "Docs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Docs\historykeeper_changelog.txt
# End Source File
# Begin Source File

SOURCE=.\Docs\historykeeper_readme.txt
# End Source File
# Begin Source File

SOURCE=.\Docs\historykeeper_version.txt
# End Source File
# Begin Source File

SOURCE=.\Docs\langpack_historykeeper.txt
# End Source File
# End Group
# End Target
# End Project
