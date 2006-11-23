# Microsoft Developer Studio Project File - Name="jgmail" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=jgmail - Win32 Release Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "jabber.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "jabber.mak" CFG="jgmail - Win32 Release Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "jgmail - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "jgmail - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "jgmail - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "jgmail - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "jgmail - Win32 Static Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "jgmail - Win32 Static" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Miranda/miranda/protocols/jgmail", NEOAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "jgmail - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ""
# PROP BASE Intermediate_Dir ""
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "compile/release/ansi"
# PROP Intermediate_Dir "compile/release/ansi"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Yu"jabber.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/upload/jabber/JGmail.dll" /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /pdb:none /incremental:yes
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=$(COMSPEC) /c "cd ../../bin/upload/ && md5 -s -t -ojabber/JGmail.dll.md5 jabber/JGmail.dll"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ""
# PROP BASE Intermediate_Dir ""
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "compile/debug/ansi"
# PROP Intermediate_Dir "compile/debug/ansi"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /FR /Yu"jabber.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/debug/plugins/JGmail.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ""
# PROP BASE Intermediate_Dir ""
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "compile/release/unicode"
# PROP Intermediate_Dir "compile/release/unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Yu"jabber.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /FR /Yu"jabber.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/upload/jabber/u/JGmail.dll" /ALIGN:4096 /ignore:4108
# SUBTRACT BASE LINK32 /pdb:none /incremental:yes
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/upload/jabber/u/JGmail.dll" /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /pdb:none /incremental:yes
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=$(COMSPEC) /c "cd ../../bin/upload/ && md5 -s -t -ojabber/u/JGmail.dll.md5 jabber/u/JGmail.dll"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ""
# PROP BASE Intermediate_Dir ""
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "compile/debug/unicode"
# PROP Intermediate_Dir "compile/debug/unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /FR /Yu"jabber.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /FR /Yu"jabber.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib /nologo /base:"0x32500000" /dll /incremental:no /map /debug /machine:I386 /out:"../../bin/Debug Unicode/plugins/JGmail.dll" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/Debug Unicode/plugins/JGmail.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ""
# PROP BASE Intermediate_Dir ""
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "compile/staticssl/unicode"
# PROP Intermediate_Dir "compile/staticssl/unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /Yu"jabber.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Yu"jabber.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib zlib.lib ssleay32.lib libeay32.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/upload/jabber/staticssl/u/JGmail.dll" /libpath:"z:\temp\openssl\zlib-1.2.3\projects\visualc6\Win32_LIB_ASM_Release\\" /libpath:"Z:\temp\openssl\openssl-0.9.8d\out32\\" /ALIGN:4096 /ignore:4108
# SUBTRACT BASE LINK32 /pdb:none /incremental:yes
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib zlib.lib ssleay32.lib libeay32.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/upload/jabber/staticssl/u/JGmail.dll" /libpath:"z:\temp\openssl\zlib-1.2.3\projects\visualc6\Win32_LIB_ASM_Release\\" /libpath:"Z:\temp\openssl\openssl-0.9.8d\out32\\" /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /pdb:none /incremental:yes
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=$(COMSPEC) /c "cd ../../bin/upload/ && upx --best --force "jabber/staticssl/u/JGmail.dll" && md5 -s -t -ojabber/staticssl/u/JGmail.dll.md5 jabber/staticssl/u/JGmail.dll"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ""
# PROP BASE Intermediate_Dir ""
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "compile/staticssl/ansi"
# PROP Intermediate_Dir "compile/staticssl/ansi"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /Yu"jabber.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Yu"jabber.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/upload/jabber/staticssl/JGmail.dll" /ALIGN:4096 /ignore:4108
# SUBTRACT BASE LINK32 /pdb:none /incremental:yes
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib zlib.lib ssleay32.lib libeay32.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/upload/jabber/staticssl/JGmail.dll" /libpath:"z:\temp\openssl\zlib-1.2.3\projects\visualc6\Win32_LIB_ASM_Release\\" /libpath:"Z:\temp\openssl\openssl-0.9.8d\out32\\" /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /pdb:none /incremental:yes
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=$(COMSPEC) /c "cd ../../bin/upload/ && upx --best --force jabber/staticssl/JGmail.dll && md5 -s -t -ojabber/staticssl/JGmail.dll.md5 jabber/staticssl/JGmail.dll"
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "jgmail - Win32 Release"
# Name "jgmail - Win32 Debug"
# Name "jgmail - Win32 Release Unicode"
# Name "jgmail - Win32 Debug Unicode"
# Name "jgmail - Win32 Static Unicode"
# Name "jgmail - Win32 Static"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\gmail.cpp
# End Source File
# Begin Source File

SOURCE=.\google_token.cpp
# End Source File
# Begin Source File

SOURCE=.\icolib.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber.cpp
# ADD CPP /Yc"jabber.h"
# End Source File
# Begin Source File

SOURCE=.\jabber_agent.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_bitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_byte.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_chat.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_deprecated.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_file.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_form.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_ft.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_groupchat.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_icolib.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_iq.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_iqid.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_iqid_muc.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_libstr.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_list.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_menu.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_misc.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_opt.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_password.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_proxy.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_secur.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_ssl.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_std.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_svc.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_thread.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_userinfo.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_util.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_vcard.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_ws.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_xml.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\jabber_xmlns.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\md5c.cpp
# End Source File
# Begin Source File

SOURCE=.\sha1.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\jabber.h
# End Source File
# Begin Source File

SOURCE=.\jabber_byte.h
# End Source File
# Begin Source File

SOURCE=.\jabber_iq.h
# End Source File
# Begin Source File

SOURCE=.\jabber_list.h
# End Source File
# Begin Source File

SOURCE=.\jabber_proxy.h
# End Source File
# Begin Source File

SOURCE=.\jabber_secur.h
# End Source File
# Begin Source File

SOURCE=.\jabber_ssl.h
# End Source File
# Begin Source File

SOURCE=.\jabber_xml.h
# End Source File
# Begin Source File

SOURCE=.\jabber_xmlns.h
# End Source File
# Begin Source File

SOURCE=.\md5.h
# End Source File
# Begin Source File

SOURCE=.\sha1.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icos\16x16.bmp
# End Source File
# Begin Source File

SOURCE=.\icos\jabber.ico
# End Source File
# Begin Source File

SOURCE=.\msvc6.rc
# End Source File
# End Group
# End Target
# End Project
