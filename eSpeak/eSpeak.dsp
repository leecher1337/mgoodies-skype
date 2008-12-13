# Microsoft Developer Studio Project File - Name="eSpeak" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=eSpeak - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "eSpeak.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "eSpeak.mak" CFG="eSpeak - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "eSpeak - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "eSpeak - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "eSpeak - Win32 Unicode Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "eSpeak - Win32 Unicode Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "eSpeak - Win32 Release"

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
# ADD CPP /nologo /G4 /MT /W3 /GX /O2 /Ob0 /I "../../include" /I "sdk" /D "WIN32" /D "W32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fr /YX /FD /c
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winmm.lib advapi32.lib PAStaticWMME.lib /nologo /base:"0x3EC20000" /dll /map /debug /debugtype:both /machine:I386 /nodefaultlib:"LIBC" /out:"..\..\bin\release\Plugins\meSpeak.dll" /pdbtype:sept /libpath:"lib" /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /profile /pdb:none

!ELSEIF  "$(CFG)" == "eSpeak - Win32 Debug"

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
# ADD CPP /nologo /G4 /MTd /W3 /GX /ZI /Od /I "../../include" /I "sdk" /D "WIN32" /D "W32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"..\..bin\release\Plugins\eSpeak.dll" /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT BASE LINK32 /profile /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winmm.lib advapi32.lib PAStaticWMME.lib /nologo /base:"0x3EC20000" /dll /incremental:yes /debug /machine:I386 /nodefaultlib:"LIBC" /out:"..\..\bin\debug\Plugins\meSpeak.dll" /libpath:"lib" /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /profile /pdb:none

!ELSEIF  "$(CFG)" == "eSpeak - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "eSpeak___Win32_Unicode_Debug"
# PROP BASE Intermediate_Dir "eSpeak___Win32_Unicode_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Unicode_Debug"
# PROP Intermediate_Dir "Unicode_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G4 /MTd /W3 /GX /ZI /Od /I "../../include" /FR /YX /FD /c
# ADD CPP /nologo /G4 /MTd /W3 /GX /ZI /Od /I "../../include" /I "sdk" /D "WIN32" /D "W32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x32100000" /dll /incremental:yes /debug /machine:I386 /out:"..\..\bin\debug\Plugins\eSpeak.dll" /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT BASE LINK32 /profile /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winmm.lib advapi32.lib PAStaticWMME.lib /nologo /base:"0x3EC20000" /dll /incremental:yes /debug /machine:I386 /nodefaultlib:"LIBC" /out:"..\..\bin\debug unicode\Plugins\meSpeakW.dll" /libpath:"lib" /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /profile /pdb:none

!ELSEIF  "$(CFG)" == "eSpeak - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "eSpeak___Win32_Unicode_Release"
# PROP BASE Intermediate_Dir "eSpeak___Win32_Unicode_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Unicode_Release"
# PROP Intermediate_Dir "Unicode_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G4 /MT /W3 /GX /O2 /Ob0 /I "../../include" /Fr /YX /FD /c
# ADD CPP /nologo /G4 /MT /W3 /GX /O2 /Ob0 /I "../../include" /I "sdk" /D "WIN32" /D "W32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x32100000" /dll /map /machine:I386 /out:"..\..\bin\release\Plugins\eSpeak.dll" /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT BASE LINK32 /profile /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winmm.lib advapi32.lib PAStaticWMME.lib /nologo /base:"0x3EC20000" /dll /map /debug /debugtype:both /machine:I386 /nodefaultlib:"LIBC" /out:"..\..\bin\release\Plugins\meSpeakW.dll" /pdbtype:sept /libpath:"lib" /filealign:0x200 /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /profile /pdb:none

!ENDIF 

# Begin Target

# Name "eSpeak - Win32 Release"
# Name "eSpeak - Win32 Debug"
# Name "eSpeak - Win32 Unicode Debug"
# Name "eSpeak - Win32 Unicode Release"
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\commons.h
# End Source File
# Begin Source File

SOURCE=..\utils\ContactAsyncQueue.h
# End Source File
# Begin Source File

SOURCE=.\m_speak.h
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

SOURCE=.\options.h
# End Source File
# Begin Source File

SOURCE=.\types.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\res\unknown.ico
# End Source File
# End Group
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\utils\ContactAsyncQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\eSpeak.cpp
# End Source File
# Begin Source File

SOURCE=..\utils\mir_icons.cpp
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

SOURCE=.\types.cpp
# End Source File
# End Group
# Begin Group "Docs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Docs\langpack_meSpeak.txt
# End Source File
# Begin Source File

SOURCE=.\Docs\meSpeak_changelog.txt
# End Source File
# Begin Source File

SOURCE=.\Docs\meSpeak_readme.txt
# End Source File
# Begin Source File

SOURCE=.\Docs\meSpeak_version.txt
# End Source File
# End Group
# Begin Group "eSpeak"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\eSpeak\compiledict.cpp
# End Source File
# Begin Source File

SOURCE=.\eSpeak\dictionary.cpp
# End Source File
# Begin Source File

SOURCE=.\eSpeak\intonation.cpp
# End Source File
# Begin Source File

SOURCE=.\eSpeak\klatt.cpp
# End Source File
# Begin Source File

SOURCE=.\eSpeak\klatt.h
# End Source File
# Begin Source File

SOURCE=.\eSpeak\numbers.cpp
# End Source File
# Begin Source File

SOURCE=.\eSpeak\phoneme.h
# End Source File
# Begin Source File

SOURCE=.\eSpeak\phonemelist.cpp
# End Source File
# Begin Source File

SOURCE=.\eSpeak\readclause.cpp
# End Source File
# Begin Source File

SOURCE=.\eSpeak\setlengths.cpp
# End Source File
# Begin Source File

SOURCE=.\eSpeak\sintab.h
# End Source File
# Begin Source File

SOURCE=.\eSpeak\speak_lib.cpp
# End Source File
# Begin Source File

SOURCE=.\eSpeak\speak_lib.h
# End Source File
# Begin Source File

SOURCE=.\eSpeak\speech.h
# End Source File
# Begin Source File

SOURCE=.\eSpeak\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\eSpeak\synth_mbrola.cpp
# End Source File
# Begin Source File

SOURCE=.\eSpeak\synthdata.cpp
# End Source File
# Begin Source File

SOURCE=.\eSpeak\synthesize.cpp
# End Source File
# Begin Source File

SOURCE=.\eSpeak\synthesize.h
# End Source File
# Begin Source File

SOURCE=.\eSpeak\tr_english.cpp
# End Source File
# Begin Source File

SOURCE=.\eSpeak\tr_languages.cpp
# End Source File
# Begin Source File

SOURCE=.\eSpeak\tr_languages.h
# End Source File
# Begin Source File

SOURCE=.\eSpeak\translate.cpp
# End Source File
# Begin Source File

SOURCE=.\eSpeak\translate.h
# End Source File
# Begin Source File

SOURCE=.\eSpeak\voice.h
# End Source File
# Begin Source File

SOURCE=.\eSpeak\voices.cpp
# End Source File
# Begin Source File

SOURCE=.\eSpeak\wavegen.cpp
# End Source File
# End Group
# End Target
# End Project
