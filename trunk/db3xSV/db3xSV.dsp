# Microsoft Developer Studio Project File - Name="db3xSV" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=db3xSV - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "db3xSV.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "db3xSV.mak" CFG="db3xSV - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "db3xSV - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "db3xSV - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "db3xSV - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release_Secure"
# PROP BASE Intermediate_Dir "Release_Secure"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Secure"
# PROP Intermediate_Dir "Release_Secure"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O1 /I "../../include/" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "db3xV_EXPORTS" /D "SECUREDB" /FR /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /G4 /Zp4 /MD /W3 /O1 /I "../../include/" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "db3xV_EXPORTS" /D "SECUREDB" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG" /d "SECUREDB"
# ADD RSC /l 0x809 /d "NDEBUG" /d "SECUREDB"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x5130000" /dll /map /machine:I386 /out:"../../bin/release/plugins/dbx_3xSV.dll" /IGNORE:4089
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x5130000" /dll /map /machine:I386 /out:"../../bin/upload/db3xV/dbx_3xSV.dll" /IGNORE:4089 /FILEALIGN:0x200
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=$(COMSPEC) /c "cd ../../bin/upload/ && md5 -s -t -odb3xV/dbx_3xSV.dll.md5 db3xV/dbx_3xSV.dll"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "db3xSV - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug_Secure"
# PROP BASE Intermediate_Dir "Debug_Secure"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Secure"
# PROP Intermediate_Dir "Debug_Secure"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /ZI /Od /I "../../include/" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "db3xV_EXPORTS" /Fr /YX /FD /GZ /c
# ADD CPP /nologo /G4 /Zp4 /MDd /W3 /Gm /ZI /Od /I "../../include/" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "db3xV_EXPORTS" /D "SECUREDB" /Fr /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG" /d "SECUREDB"
# ADD RSC /l 0x809 /d "_DEBUG" /d "SECUREDB"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /dll /map /debug /machine:I386 /out:"../../bin/debug/plugins/dbx_3xSV.dll" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /dll /map /debug /machine:I386 /out:"../../bin/debug/plugins/dbx_3xSV.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "db3xSV - Win32 Release"
# Name "db3xSV - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\commonheaders.c
# End Source File
# Begin Source File

SOURCE=.\database.c
# End Source File
# Begin Source File

SOURCE=.\dbcache.c
# End Source File
# Begin Source File

SOURCE=.\dbcontacts.c
# End Source File
# Begin Source File

SOURCE=.\dbevents.c
# End Source File
# Begin Source File

SOURCE=.\dbheaders.c
# End Source File
# Begin Source File

SOURCE=.\dblists.c
# End Source File
# Begin Source File

SOURCE=.\dbmodulechain.c
# End Source File
# Begin Source File

SOURCE=.\dbsettings.c
# End Source File
# Begin Source File

SOURCE=.\encrypt.c
# End Source File
# Begin Source File

SOURCE=.\icos.c
# End Source File
# Begin Source File

SOURCE=.\init.c
# End Source File
# Begin Source File

SOURCE=.\initmenu.c
# End Source File
# Begin Source File

SOURCE=.\SecureDB.c
# End Source File
# Begin Source File

SOURCE=.\sha256.c
# End Source File
# Begin Source File

SOURCE=.\utf.c
# End Source File
# Begin Source File

SOURCE=.\virtdb.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\commonheaders.h
# End Source File
# Begin Source File

SOURCE=.\database.h
# End Source File
# Begin Source File

SOURCE=.\dblists.h
# End Source File
# Begin Source File

SOURCE=.\initmenu.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\SecureDB.h
# End Source File
# Begin Source File

SOURCE=.\sha256.h
# End Source File
# Begin Source File

SOURCE=.\virtdb.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\dbsec.ico
# End Source File
# Begin Source File

SOURCE=.\delpw.ico
# End Source File
# Begin Source File

SOURCE=.\newpw.ico
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# End Group
# End Target
# End Project
