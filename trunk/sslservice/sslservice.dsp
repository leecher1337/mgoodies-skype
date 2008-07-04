# Microsoft Developer Studio Project File - Name="sslservice" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=sslservice - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sslservice.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sslservice.mak" CFG="sslservice - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sslservice - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "sslservice - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sslservice - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "sslservice_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G4 /Zp4 /MD /W3 /GX /O1 /I "cyassl/include" /I "cyassl/ctaocrypt/include" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "sslservice_EXPORTS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib wsock32.lib Advapi32.lib /nologo /dll /machine:I386 /out:"../../bin/release/plugins/sslservice.dll" /FILEALIGN:0x200
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "sslservice - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "sslservice_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "cyassl/include" /I "cyassl/ctaocrypt/include" /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "sslservice_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/debug/plugins/sslservice.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "sslservice - Win32 Release"
# Name "sslservice - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\sslservice.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\m_sslservice.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "cyassl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cyassl\ctaocrypt\src\aes.c
# End Source File
# Begin Source File

SOURCE=.\cyassl\ctaocrypt\src\arc4.c
# End Source File
# Begin Source File

SOURCE=.\cyassl\ctaocrypt\src\asn.c
# End Source File
# Begin Source File

SOURCE=.\cyassl\ctaocrypt\src\coding.c
# End Source File
# Begin Source File

SOURCE=.\cyassl\src\cyassl_int.c

!IF  "$(CFG)" == "sslservice - Win32 Release"

!ELSEIF  "$(CFG)" == "sslservice - Win32 Debug"

# SUBTRACT CPP /I "../../include"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cyassl\ctaocrypt\src\des3.c
# End Source File
# Begin Source File

SOURCE=.\cyassl\ctaocrypt\src\dh.c
# End Source File
# Begin Source File

SOURCE=.\cyassl\ctaocrypt\src\dsa.c
# End Source File
# Begin Source File

SOURCE=.\cyassl\ctaocrypt\src\hmac.c
# End Source File
# Begin Source File

SOURCE=.\cyassl\ctaocrypt\src\integer.c
# End Source File
# Begin Source File

SOURCE=.\cyassl\src\keys.c
# End Source File
# Begin Source File

SOURCE=.\cyassl\ctaocrypt\src\md5.c
# End Source File
# Begin Source File

SOURCE=.\cyassl\ctaocrypt\src\misc.c
# End Source File
# Begin Source File

SOURCE=.\cyassl\ctaocrypt\src\random.c
# End Source File
# Begin Source File

SOURCE=.\cyassl\ctaocrypt\src\rsa.c
# End Source File
# Begin Source File

SOURCE=.\cyassl\ctaocrypt\src\sha.c
# End Source File
# Begin Source File

SOURCE=.\cyassl\ctaocrypt\src\sha256.c
# End Source File
# Begin Source File

SOURCE=.\cyassl\src\ssl.c

!IF  "$(CFG)" == "sslservice - Win32 Release"

# ADD CPP /I "cyassl/include"

!ELSEIF  "$(CFG)" == "sslservice - Win32 Debug"

# SUBTRACT CPP /I "../../include"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cyassl\src\tls.c
# End Source File
# End Group
# End Target
# End Project
