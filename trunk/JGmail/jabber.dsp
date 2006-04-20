# Microsoft Developer Studio Project File - Name="jabberg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=jabberg - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "jabber.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "jabber.mak" CFG="jabberg - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "jabberg - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "jabberg - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "jabberg - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "jabberg - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "jabberg - Win32 Static Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "jabberg - Win32 Static" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Miranda/miranda/protocols/JabberG", NEOAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "jabberg - Win32 Release"

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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/release/plugins/JGmail.dll" /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /pdb:none /incremental:yes
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=upx --best --force ../../bin/release/plugins/JGmail.dll
# End Special Build Tool

!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"

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

!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "jabberg___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "jabberg___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release_Unicode"
# PROP Intermediate_Dir ".\Release_Unicode"
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
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/release/plugins/JGmail.dll" /ALIGN:4096 /ignore:4108
# SUBTRACT BASE LINK32 /pdb:none /incremental:yes
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/Release Unicode/plugins/JGmail.dll" /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /pdb:none /incremental:yes
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=upx --best --force "../../bin/Release Unicode/plugins/JGmail.dll"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "jabberg___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "jabberg___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "./Debug_Unicode"
# PROP Intermediate_Dir "./Debug_Unicode"
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

!ELSEIF  "$(CFG)" == "jabberg - Win32 Static Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "jabberg___Win32_Static_Unicode"
# PROP BASE Intermediate_Dir "jabberg___Win32_Static_Unicode"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Static_Unicode"
# PROP Intermediate_Dir "Release_Static_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "Z:\temp\openssl\openssl-0.9.8-stable-SNAP-20060314\inc32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Yu"jabber.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "Z:\temp\openssl\openssl-0.9.8-stable-SNAP-20060314\inc32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Yu"jabber.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib zlib.lib ssleay32.lib libeay32.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/Release Unicode/plugins/staticssl/JGmail.dll" /libpath:"z:\temp\openssl\zlib-1.2.3\projects\visualc6\Win32_LIB_ASM_Release\\" /libpath:"z:\temp\openssl\openssl-0.9.8-stable-SNAP-20060314\out32\\" /ALIGN:4096 /ignore:4108
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib zlib.lib ssleay32.lib libeay32.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/Release Unicode/plugins/staticssl/JGmail.dll" /libpath:"z:\temp\openssl\zlib-1.2.3\projects\visualc6\Win32_LIB_ASM_Release\\" /libpath:"z:\temp\openssl\openssl-0.9.8-stable-SNAP-20060314\out32\\" /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=upx --best --force "../../bin/Release Unicode/plugins/staticssl/JGmail.dll"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "jabberg - Win32 Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "jabberg___Win32_Static"
# PROP BASE Intermediate_Dir "jabberg___Win32_Static"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Static"
# PROP Intermediate_Dir "Release_Static"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Yu"jabber.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "Z:\temp\openssl\openssl-0.9.8-stable-SNAP-20060314\inc32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Yu"jabber.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/release/plugins/JGmail.dll" /ALIGN:4096 /ignore:4108
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib zlib.lib ssleay32.lib libeay32.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/release/plugins/staticssl/JGmail.dll" /libpath:"z:\temp\openssl\zlib-1.2.3\projects\visualc6\Win32_LIB_ASM_Release\\" /libpath:"z:\temp\openssl\openssl-0.9.8-stable-SNAP-20060314\out32\\" /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=upx --best --force ../../bin/release/plugins/staticssl/JGmail.dll
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "jabberg - Win32 Release"
# Name "jabberg - Win32 Debug"
# Name "jabberg - Win32 Release Unicode"
# Name "jabberg - Win32 Debug Unicode"
# Name "jabberg - Win32 Static Unicode"
# Name "jabberg - Win32 Static"
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

SOURCE=.\jabber_ssl.h
# End Source File
# Begin Source File

SOURCE=.\jabber_xml.h
# End Source File
# Begin Source File

SOURCE=.\jabber_xmlns.h
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

SOURCE=.\icos\addcontact.ico
# End Source File
# Begin Source File

SOURCE=.\icos\block.ico
# End Source File
# Begin Source File

SOURCE=.\icos\delete.ico
# End Source File
# Begin Source File

SOURCE=.\icos\gcadmin.ico
# End Source File
# Begin Source File

SOURCE=.\icos\gcmodera.ico
# End Source File
# Begin Source File

SOURCE=.\icos\gcowner.ico
# End Source File
# Begin Source File

SOURCE=.\icos\gcvoice.ico
# End Source File
# Begin Source File

SOURCE=.\icos\grant.ico
# End Source File
# Begin Source File

SOURCE=.\icos\group.ico
# End Source File
# Begin Source File

SOURCE=.\icos\jabber.ico
# End Source File
# Begin Source File

SOURCE=.\icos\key.ico
# End Source File
# Begin Source File

SOURCE=".\icos\mail-clock.ico"
# End Source File
# Begin Source File

SOURCE=".\icos\mail-info.ico"
# End Source File
# Begin Source File

SOURCE=".\icos\mail-new.ico"
# End Source File
# Begin Source File

SOURCE=".\icos\mail-stop.ico"
# End Source File
# Begin Source File

SOURCE=.\msvc6.rc
# End Source File
# Begin Source File

SOURCE=.\icos\open.ico
# End Source File
# Begin Source File

SOURCE=.\icos\pages.ico
# End Source File
# Begin Source File

SOURCE=.\icos\rename.ico
# End Source File
# Begin Source File

SOURCE=.\icos\request.ico
# End Source File
# Begin Source File

SOURCE=.\icos\save.ico
# End Source File
# Begin Source File

SOURCE=.\icos\write.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\docs\changelog_jabber.txt
# End Source File
# Begin Source File

SOURCE=.\docs\readme_jabber.txt
# End Source File
# Begin Source File

SOURCE=.\docs\translation_jabber.txt
# End Source File
# End Target
# End Project
