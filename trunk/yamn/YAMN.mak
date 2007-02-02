# Microsoft Developer Studio Generated NMAKE File, Based on YAMN.dsp
!IF "$(CFG)" == ""
CFG=YAMN - Win32 Release Win2in1
!MESSAGE No configuration specified. Defaulting to YAMN - Win32 Release Win2in1.
!ENDIF 

!IF "$(CFG)" != "YAMN - Win32 Release Win2in1" && "$(CFG)" != "YAMN - Win32 Debug Win2in1" && "$(CFG)" != "YAMN - Win32 Release" && "$(CFG)" != "YAMN - Win32 Debug" && "$(CFG)" != "YAMN - Win32 Release Win9x" && "$(CFG)" != "YAMN - Win32 Debug Win9x"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "YAMN.mak" CFG="YAMN - Win32 Release Win2in1"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "YAMN - Win32 Release Win2in1" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "YAMN - Win32 Debug Win2in1" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "YAMN - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "YAMN - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "YAMN - Win32 Release Win9x" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "YAMN - Win32 Debug Win9x" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"

OUTDIR=.\Release\Win2in1
INTDIR=.\Release\Win2in1
# Begin Custom Macros
OutDir=.\Release\Win2in1
# End Custom Macros

ALL : "..\..\bin\release\plugins\YAMN.dll" "$(OUTDIR)\YAMN.bsc"


CLEAN :
	-@erase "$(INTDIR)\account.obj"
	-@erase "$(INTDIR)\badconnect.obj"
	-@erase "$(INTDIR)\badconnect.sbr"
	-@erase "$(INTDIR)\debug.obj"
	-@erase "$(INTDIR)\debug.sbr"
	-@erase "$(INTDIR)\decode.obj"
	-@erase "$(INTDIR)\decode.sbr"
	-@erase "$(INTDIR)\filterplugin.obj"
	-@erase "$(INTDIR)\filterplugin.sbr"
	-@erase "$(INTDIR)\mailbrowser.obj"
	-@erase "$(INTDIR)\mailbrowser.sbr"
	-@erase "$(INTDIR)\mails.obj"
	-@erase "$(INTDIR)\mails.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\mime.obj"
	-@erase "$(INTDIR)\mime.sbr"
	-@erase "$(INTDIR)\netlib.obj"
	-@erase "$(INTDIR)\netlib.sbr"
	-@erase "$(INTDIR)\pop3.obj"
	-@erase "$(INTDIR)\pop3.sbr"
	-@erase "$(INTDIR)\pop3comm.obj"
	-@erase "$(INTDIR)\pop3comm.sbr"
	-@erase "$(INTDIR)\pop3opt.obj"
	-@erase "$(INTDIR)\pop3opt.sbr"
	-@erase "$(INTDIR)\protoplugin.obj"
	-@erase "$(INTDIR)\protoplugin.sbr"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\services.sbr"
	-@erase "$(INTDIR)\ssl.obj"
	-@erase "$(INTDIR)\ssl.sbr"
	-@erase "$(INTDIR)\synchro.obj"
	-@erase "$(INTDIR)\synchro.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\yamn.obj"
	-@erase "$(INTDIR)\YAMN.res"
	-@erase "$(INTDIR)\yamn.sbr"
	-@erase "$(OUTDIR)\YAMN.bsc"
	-@erase "$(OUTDIR)\YAMN.exp"
	-@erase "$(OUTDIR)\YAMN.lib"
	-@erase "..\..\bin\release\plugins\YAMN.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Zp4 /MD /W3 /GX /Zi /O1 /Ob0 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WIN2IN1" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\YAMN.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC_PROJ=/l 0x417 /fo"$(INTDIR)\YAMN.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\YAMN.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\badconnect.sbr" \
	"$(INTDIR)\mailbrowser.sbr" \
	"$(INTDIR)\decode.sbr" \
	"$(INTDIR)\mails.sbr" \
	"$(INTDIR)\mime.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\netlib.sbr" \
	"$(INTDIR)\pop3.sbr" \
	"$(INTDIR)\pop3comm.sbr" \
	"$(INTDIR)\pop3opt.sbr" \
	"$(INTDIR)\ssl.sbr" \
	"$(INTDIR)\debug.sbr" \
	"$(INTDIR)\filterplugin.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\protoplugin.sbr" \
	"$(INTDIR)\services.sbr" \
	"$(INTDIR)\synchro.sbr" \
	"$(INTDIR)\yamn.sbr"

"$(OUTDIR)\YAMN.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=libs/unicows.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /pdb:none /machine:I386 /out:"../../bin/release/plugins/YAMN.dll" /implib:"$(OUTDIR)\YAMN.lib" /filealign:512 
LINK32_OBJS= \
	"$(INTDIR)\badconnect.obj" \
	"$(INTDIR)\mailbrowser.obj" \
	"$(INTDIR)\decode.obj" \
	"$(INTDIR)\mails.obj" \
	"$(INTDIR)\mime.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\netlib.obj" \
	"$(INTDIR)\pop3.obj" \
	"$(INTDIR)\pop3comm.obj" \
	"$(INTDIR)\pop3opt.obj" \
	"$(INTDIR)\ssl.obj" \
	"$(INTDIR)\account.obj" \
	"$(INTDIR)\debug.obj" \
	"$(INTDIR)\filterplugin.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\protoplugin.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\synchro.obj" \
	"$(INTDIR)\yamn.obj" \
	"$(INTDIR)\YAMN.res"

"..\..\bin\release\plugins\YAMN.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"

OUTDIR=.\Debug\Win2in1
INTDIR=.\Debug\Win2in1
# Begin Custom Macros
OutDir=.\Debug\Win2in1
# End Custom Macros

ALL : "..\..\bin\Debug\plugins\YAMN.dll" "$(OUTDIR)\YAMN.bsc"


CLEAN :
	-@erase "$(INTDIR)\account.obj"
	-@erase "$(INTDIR)\badconnect.obj"
	-@erase "$(INTDIR)\badconnect.sbr"
	-@erase "$(INTDIR)\debug.obj"
	-@erase "$(INTDIR)\debug.sbr"
	-@erase "$(INTDIR)\decode.obj"
	-@erase "$(INTDIR)\decode.sbr"
	-@erase "$(INTDIR)\filterplugin.obj"
	-@erase "$(INTDIR)\filterplugin.sbr"
	-@erase "$(INTDIR)\mailbrowser.obj"
	-@erase "$(INTDIR)\mailbrowser.sbr"
	-@erase "$(INTDIR)\mails.obj"
	-@erase "$(INTDIR)\mails.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\mime.obj"
	-@erase "$(INTDIR)\mime.sbr"
	-@erase "$(INTDIR)\netlib.obj"
	-@erase "$(INTDIR)\netlib.sbr"
	-@erase "$(INTDIR)\pop3.obj"
	-@erase "$(INTDIR)\pop3.sbr"
	-@erase "$(INTDIR)\pop3comm.obj"
	-@erase "$(INTDIR)\pop3comm.sbr"
	-@erase "$(INTDIR)\pop3opt.obj"
	-@erase "$(INTDIR)\pop3opt.sbr"
	-@erase "$(INTDIR)\protoplugin.obj"
	-@erase "$(INTDIR)\protoplugin.sbr"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\services.sbr"
	-@erase "$(INTDIR)\ssl.obj"
	-@erase "$(INTDIR)\ssl.sbr"
	-@erase "$(INTDIR)\synchro.obj"
	-@erase "$(INTDIR)\synchro.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\yamn.obj"
	-@erase "$(INTDIR)\YAMN.res"
	-@erase "$(INTDIR)\yamn.sbr"
	-@erase "$(OUTDIR)\YAMN.bsc"
	-@erase "$(OUTDIR)\YAMN.exp"
	-@erase "$(OUTDIR)\YAMN.lib"
	-@erase "$(OUTDIR)\YAMN.pdb"
	-@erase "..\..\bin\Debug\plugins\YAMN.dll"
	-@erase "..\..\bin\Debug\plugins\YAMN.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Zp4 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../include" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_DEBUG" /D "WIN2IN1" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\YAMN.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC_PROJ=/l 0x417 /fo"$(INTDIR)\YAMN.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\YAMN.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\badconnect.sbr" \
	"$(INTDIR)\mailbrowser.sbr" \
	"$(INTDIR)\decode.sbr" \
	"$(INTDIR)\mails.sbr" \
	"$(INTDIR)\mime.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\netlib.sbr" \
	"$(INTDIR)\pop3.sbr" \
	"$(INTDIR)\pop3comm.sbr" \
	"$(INTDIR)\pop3opt.sbr" \
	"$(INTDIR)\ssl.sbr" \
	"$(INTDIR)\debug.sbr" \
	"$(INTDIR)\filterplugin.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\protoplugin.sbr" \
	"$(INTDIR)\services.sbr" \
	"$(INTDIR)\synchro.sbr" \
	"$(INTDIR)\yamn.sbr"

"$(OUTDIR)\YAMN.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=libs/unicows.lib winspool.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /dll /incremental:yes /pdb:"$(OUTDIR)\YAMN.pdb" /debug /machine:I386 /out:"../../bin/Debug/plugins/YAMN.dll" /implib:"$(OUTDIR)\YAMN.lib" 
LINK32_OBJS= \
	"$(INTDIR)\badconnect.obj" \
	"$(INTDIR)\mailbrowser.obj" \
	"$(INTDIR)\decode.obj" \
	"$(INTDIR)\mails.obj" \
	"$(INTDIR)\mime.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\netlib.obj" \
	"$(INTDIR)\pop3.obj" \
	"$(INTDIR)\pop3comm.obj" \
	"$(INTDIR)\pop3opt.obj" \
	"$(INTDIR)\ssl.obj" \
	"$(INTDIR)\account.obj" \
	"$(INTDIR)\debug.obj" \
	"$(INTDIR)\filterplugin.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\protoplugin.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\synchro.obj" \
	"$(INTDIR)\yamn.obj" \
	"$(INTDIR)\YAMN.res"

"..\..\bin\Debug\plugins\YAMN.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"

OUTDIR=.\Release/WinNT
INTDIR=.\Release/WinNT
# Begin Custom Macros
OutDir=.\Release/WinNT
# End Custom Macros

ALL : "..\..\bin\release\plugins\YAMN-NT\YAMN.dll" "$(OUTDIR)\YAMN.bsc"


CLEAN :
	-@erase "$(INTDIR)\account.obj"
	-@erase "$(INTDIR)\badconnect.obj"
	-@erase "$(INTDIR)\badconnect.sbr"
	-@erase "$(INTDIR)\debug.obj"
	-@erase "$(INTDIR)\debug.sbr"
	-@erase "$(INTDIR)\decode.obj"
	-@erase "$(INTDIR)\decode.sbr"
	-@erase "$(INTDIR)\filterplugin.obj"
	-@erase "$(INTDIR)\filterplugin.sbr"
	-@erase "$(INTDIR)\mailbrowser.obj"
	-@erase "$(INTDIR)\mailbrowser.sbr"
	-@erase "$(INTDIR)\mails.obj"
	-@erase "$(INTDIR)\mails.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\mime.obj"
	-@erase "$(INTDIR)\mime.sbr"
	-@erase "$(INTDIR)\netlib.obj"
	-@erase "$(INTDIR)\netlib.sbr"
	-@erase "$(INTDIR)\pop3.obj"
	-@erase "$(INTDIR)\pop3.sbr"
	-@erase "$(INTDIR)\pop3comm.obj"
	-@erase "$(INTDIR)\pop3comm.sbr"
	-@erase "$(INTDIR)\pop3opt.obj"
	-@erase "$(INTDIR)\pop3opt.sbr"
	-@erase "$(INTDIR)\protoplugin.obj"
	-@erase "$(INTDIR)\protoplugin.sbr"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\services.sbr"
	-@erase "$(INTDIR)\ssl.obj"
	-@erase "$(INTDIR)\ssl.sbr"
	-@erase "$(INTDIR)\synchro.obj"
	-@erase "$(INTDIR)\synchro.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\yamn.obj"
	-@erase "$(INTDIR)\YAMN.res"
	-@erase "$(INTDIR)\yamn.sbr"
	-@erase "$(OUTDIR)\YAMN.bsc"
	-@erase "$(OUTDIR)\YAMN.exp"
	-@erase "$(OUTDIR)\YAMN.lib"
	-@erase "..\..\bin\release\plugins\YAMN-NT\YAMN.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Zp4 /MD /W3 /GX /O1 /Ob0 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\YAMN.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC_PROJ=/l 0x417 /fo"$(INTDIR)\YAMN.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\YAMN.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\badconnect.sbr" \
	"$(INTDIR)\mailbrowser.sbr" \
	"$(INTDIR)\decode.sbr" \
	"$(INTDIR)\mails.sbr" \
	"$(INTDIR)\mime.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\netlib.sbr" \
	"$(INTDIR)\pop3.sbr" \
	"$(INTDIR)\pop3comm.sbr" \
	"$(INTDIR)\pop3opt.sbr" \
	"$(INTDIR)\ssl.sbr" \
	"$(INTDIR)\debug.sbr" \
	"$(INTDIR)\filterplugin.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\protoplugin.sbr" \
	"$(INTDIR)\services.sbr" \
	"$(INTDIR)\synchro.sbr" \
	"$(INTDIR)\yamn.sbr"

"$(OUTDIR)\YAMN.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /pdb:none /machine:I386 /out:"../../bin/release/plugins/YAMN-NT/YAMN.dll" /implib:"$(OUTDIR)\YAMN.lib" 
LINK32_OBJS= \
	"$(INTDIR)\badconnect.obj" \
	"$(INTDIR)\mailbrowser.obj" \
	"$(INTDIR)\decode.obj" \
	"$(INTDIR)\mails.obj" \
	"$(INTDIR)\mime.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\netlib.obj" \
	"$(INTDIR)\pop3.obj" \
	"$(INTDIR)\pop3comm.obj" \
	"$(INTDIR)\pop3opt.obj" \
	"$(INTDIR)\ssl.obj" \
	"$(INTDIR)\account.obj" \
	"$(INTDIR)\debug.obj" \
	"$(INTDIR)\filterplugin.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\protoplugin.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\synchro.obj" \
	"$(INTDIR)\yamn.obj" \
	"$(INTDIR)\YAMN.res"

"..\..\bin\release\plugins\YAMN-NT\YAMN.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"

OUTDIR=.\Debug/WinNT
INTDIR=.\Debug/WinNT
# Begin Custom Macros
OutDir=.\Debug/WinNT
# End Custom Macros

ALL : "..\..\bin\Debug\plugins\YAMN-NT\YAMN.dll" "$(OUTDIR)\YAMN.bsc"


CLEAN :
	-@erase "$(INTDIR)\account.obj"
	-@erase "$(INTDIR)\badconnect.obj"
	-@erase "$(INTDIR)\badconnect.sbr"
	-@erase "$(INTDIR)\debug.obj"
	-@erase "$(INTDIR)\debug.sbr"
	-@erase "$(INTDIR)\decode.obj"
	-@erase "$(INTDIR)\decode.sbr"
	-@erase "$(INTDIR)\filterplugin.obj"
	-@erase "$(INTDIR)\filterplugin.sbr"
	-@erase "$(INTDIR)\mailbrowser.obj"
	-@erase "$(INTDIR)\mailbrowser.sbr"
	-@erase "$(INTDIR)\mails.obj"
	-@erase "$(INTDIR)\mails.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\mime.obj"
	-@erase "$(INTDIR)\mime.sbr"
	-@erase "$(INTDIR)\netlib.obj"
	-@erase "$(INTDIR)\netlib.sbr"
	-@erase "$(INTDIR)\pop3.obj"
	-@erase "$(INTDIR)\pop3.sbr"
	-@erase "$(INTDIR)\pop3comm.obj"
	-@erase "$(INTDIR)\pop3comm.sbr"
	-@erase "$(INTDIR)\pop3opt.obj"
	-@erase "$(INTDIR)\pop3opt.sbr"
	-@erase "$(INTDIR)\protoplugin.obj"
	-@erase "$(INTDIR)\protoplugin.sbr"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\services.sbr"
	-@erase "$(INTDIR)\ssl.obj"
	-@erase "$(INTDIR)\ssl.sbr"
	-@erase "$(INTDIR)\synchro.obj"
	-@erase "$(INTDIR)\synchro.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\yamn.obj"
	-@erase "$(INTDIR)\YAMN.res"
	-@erase "$(INTDIR)\yamn.sbr"
	-@erase "$(OUTDIR)\YAMN.bsc"
	-@erase "$(OUTDIR)\YAMN.exp"
	-@erase "$(OUTDIR)\YAMN.pdb"
	-@erase "..\..\bin\Debug\plugins\YAMN-NT\YAMN.dll"
	-@erase "..\..\bin\Debug\plugins\YAMN-NT\YAMN.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Zp4 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../include" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_DEBUG" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\YAMN.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC_PROJ=/l 0x417 /fo"$(INTDIR)\YAMN.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\YAMN.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\badconnect.sbr" \
	"$(INTDIR)\mailbrowser.sbr" \
	"$(INTDIR)\decode.sbr" \
	"$(INTDIR)\mails.sbr" \
	"$(INTDIR)\mime.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\netlib.sbr" \
	"$(INTDIR)\pop3.sbr" \
	"$(INTDIR)\pop3comm.sbr" \
	"$(INTDIR)\pop3opt.sbr" \
	"$(INTDIR)\ssl.sbr" \
	"$(INTDIR)\debug.sbr" \
	"$(INTDIR)\filterplugin.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\protoplugin.sbr" \
	"$(INTDIR)\services.sbr" \
	"$(INTDIR)\synchro.sbr" \
	"$(INTDIR)\yamn.sbr"

"$(OUTDIR)\YAMN.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=winspool.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /dll /incremental:yes /pdb:"$(OUTDIR)\YAMN.pdb" /debug /machine:I386 /out:"../../bin/Debug/plugins/YAMN-NT/YAMN.dll" /implib:"$(OUTDIR)\YAMN.lib" 
LINK32_OBJS= \
	"$(INTDIR)\badconnect.obj" \
	"$(INTDIR)\mailbrowser.obj" \
	"$(INTDIR)\decode.obj" \
	"$(INTDIR)\mails.obj" \
	"$(INTDIR)\mime.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\netlib.obj" \
	"$(INTDIR)\pop3.obj" \
	"$(INTDIR)\pop3comm.obj" \
	"$(INTDIR)\pop3opt.obj" \
	"$(INTDIR)\ssl.obj" \
	"$(INTDIR)\account.obj" \
	"$(INTDIR)\debug.obj" \
	"$(INTDIR)\filterplugin.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\protoplugin.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\synchro.obj" \
	"$(INTDIR)\yamn.obj" \
	"$(INTDIR)\YAMN.res"

"..\..\bin\Debug\plugins\YAMN-NT\YAMN.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"

OUTDIR=.\Release\Win9x
INTDIR=.\Release\Win9x
# Begin Custom Macros
OutDir=.\Release\Win9x
# End Custom Macros

ALL : "..\..\bin\Release\plugins\YAMN-9x\YAMN.dll" "$(OUTDIR)\YAMN.bsc"


CLEAN :
	-@erase "$(INTDIR)\account.obj"
	-@erase "$(INTDIR)\badconnect.obj"
	-@erase "$(INTDIR)\badconnect.sbr"
	-@erase "$(INTDIR)\debug.obj"
	-@erase "$(INTDIR)\debug.sbr"
	-@erase "$(INTDIR)\decode.obj"
	-@erase "$(INTDIR)\decode.sbr"
	-@erase "$(INTDIR)\filterplugin.obj"
	-@erase "$(INTDIR)\filterplugin.sbr"
	-@erase "$(INTDIR)\mailbrowser.obj"
	-@erase "$(INTDIR)\mailbrowser.sbr"
	-@erase "$(INTDIR)\mails.obj"
	-@erase "$(INTDIR)\mails.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\mime.obj"
	-@erase "$(INTDIR)\mime.sbr"
	-@erase "$(INTDIR)\netlib.obj"
	-@erase "$(INTDIR)\netlib.sbr"
	-@erase "$(INTDIR)\pop3.obj"
	-@erase "$(INTDIR)\pop3.sbr"
	-@erase "$(INTDIR)\pop3comm.obj"
	-@erase "$(INTDIR)\pop3comm.sbr"
	-@erase "$(INTDIR)\pop3opt.obj"
	-@erase "$(INTDIR)\pop3opt.sbr"
	-@erase "$(INTDIR)\protoplugin.obj"
	-@erase "$(INTDIR)\protoplugin.sbr"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\services.sbr"
	-@erase "$(INTDIR)\ssl.obj"
	-@erase "$(INTDIR)\ssl.sbr"
	-@erase "$(INTDIR)\synchro.obj"
	-@erase "$(INTDIR)\synchro.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\yamn.obj"
	-@erase "$(INTDIR)\YAMN.res"
	-@erase "$(INTDIR)\yamn.sbr"
	-@erase "$(OUTDIR)\YAMN.bsc"
	-@erase "$(OUTDIR)\YAMN.exp"
	-@erase "$(OUTDIR)\YAMN.lib"
	-@erase "..\..\bin\Release\plugins\YAMN-9x\YAMN.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Zp4 /MD /W3 /GX /O1 /Ob0 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WIN9X" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\YAMN.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC_PROJ=/l 0x417 /fo"$(INTDIR)\YAMN.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\YAMN.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\badconnect.sbr" \
	"$(INTDIR)\mailbrowser.sbr" \
	"$(INTDIR)\decode.sbr" \
	"$(INTDIR)\mails.sbr" \
	"$(INTDIR)\mime.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\netlib.sbr" \
	"$(INTDIR)\pop3.sbr" \
	"$(INTDIR)\pop3comm.sbr" \
	"$(INTDIR)\pop3opt.sbr" \
	"$(INTDIR)\ssl.sbr" \
	"$(INTDIR)\debug.sbr" \
	"$(INTDIR)\filterplugin.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\protoplugin.sbr" \
	"$(INTDIR)\services.sbr" \
	"$(INTDIR)\synchro.sbr" \
	"$(INTDIR)\yamn.sbr"

"$(OUTDIR)\YAMN.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=libs/unicows.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /pdb:none /machine:I386 /out:"../../bin/Release/plugins/YAMN-9x/YAMN.dll" /implib:"$(OUTDIR)\YAMN.lib" /filealign:512 
LINK32_OBJS= \
	"$(INTDIR)\badconnect.obj" \
	"$(INTDIR)\mailbrowser.obj" \
	"$(INTDIR)\decode.obj" \
	"$(INTDIR)\mails.obj" \
	"$(INTDIR)\mime.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\netlib.obj" \
	"$(INTDIR)\pop3.obj" \
	"$(INTDIR)\pop3comm.obj" \
	"$(INTDIR)\pop3opt.obj" \
	"$(INTDIR)\ssl.obj" \
	"$(INTDIR)\account.obj" \
	"$(INTDIR)\debug.obj" \
	"$(INTDIR)\filterplugin.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\protoplugin.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\synchro.obj" \
	"$(INTDIR)\yamn.obj" \
	"$(INTDIR)\YAMN.res"

"..\..\bin\Release\plugins\YAMN-9x\YAMN.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"

OUTDIR=.\Debug\Win9x
INTDIR=.\Debug\Win9x
# Begin Custom Macros
OutDir=.\Debug\Win9x
# End Custom Macros

ALL : "..\..\bin\Debug\plugins\YAMN-9x\YAMN.dll" "$(OUTDIR)\YAMN.bsc"


CLEAN :
	-@erase "$(INTDIR)\account.obj"
	-@erase "$(INTDIR)\badconnect.obj"
	-@erase "$(INTDIR)\badconnect.sbr"
	-@erase "$(INTDIR)\debug.obj"
	-@erase "$(INTDIR)\debug.sbr"
	-@erase "$(INTDIR)\decode.obj"
	-@erase "$(INTDIR)\decode.sbr"
	-@erase "$(INTDIR)\filterplugin.obj"
	-@erase "$(INTDIR)\filterplugin.sbr"
	-@erase "$(INTDIR)\mailbrowser.obj"
	-@erase "$(INTDIR)\mailbrowser.sbr"
	-@erase "$(INTDIR)\mails.obj"
	-@erase "$(INTDIR)\mails.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\mime.obj"
	-@erase "$(INTDIR)\mime.sbr"
	-@erase "$(INTDIR)\netlib.obj"
	-@erase "$(INTDIR)\netlib.sbr"
	-@erase "$(INTDIR)\pop3.obj"
	-@erase "$(INTDIR)\pop3.sbr"
	-@erase "$(INTDIR)\pop3comm.obj"
	-@erase "$(INTDIR)\pop3comm.sbr"
	-@erase "$(INTDIR)\pop3opt.obj"
	-@erase "$(INTDIR)\pop3opt.sbr"
	-@erase "$(INTDIR)\protoplugin.obj"
	-@erase "$(INTDIR)\protoplugin.sbr"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\services.sbr"
	-@erase "$(INTDIR)\ssl.obj"
	-@erase "$(INTDIR)\ssl.sbr"
	-@erase "$(INTDIR)\synchro.obj"
	-@erase "$(INTDIR)\synchro.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\yamn.obj"
	-@erase "$(INTDIR)\YAMN.res"
	-@erase "$(INTDIR)\yamn.sbr"
	-@erase "$(OUTDIR)\YAMN.bsc"
	-@erase "$(OUTDIR)\YAMN.exp"
	-@erase "$(OUTDIR)\YAMN.pdb"
	-@erase "..\..\bin\Debug\plugins\YAMN-9x\YAMN.dll"
	-@erase "..\..\bin\Debug\plugins\YAMN-9x\YAMN.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Zp4 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WIN9X" /D "YAMN_VER_BETA" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\YAMN.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC_PROJ=/l 0x417 /fo"$(INTDIR)\YAMN.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\YAMN.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\badconnect.sbr" \
	"$(INTDIR)\mailbrowser.sbr" \
	"$(INTDIR)\decode.sbr" \
	"$(INTDIR)\mails.sbr" \
	"$(INTDIR)\mime.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\netlib.sbr" \
	"$(INTDIR)\pop3.sbr" \
	"$(INTDIR)\pop3comm.sbr" \
	"$(INTDIR)\pop3opt.sbr" \
	"$(INTDIR)\ssl.sbr" \
	"$(INTDIR)\debug.sbr" \
	"$(INTDIR)\filterplugin.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\protoplugin.sbr" \
	"$(INTDIR)\services.sbr" \
	"$(INTDIR)\synchro.sbr" \
	"$(INTDIR)\yamn.sbr"

"$(OUTDIR)\YAMN.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=libs/unicows.lib winspool.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /dll /incremental:yes /pdb:"$(OUTDIR)\YAMN.pdb" /debug /machine:I386 /out:"../../bin/Debug/plugins/YAMN-9x/YAMN.dll" /implib:"$(OUTDIR)\YAMN.lib" 
LINK32_OBJS= \
	"$(INTDIR)\badconnect.obj" \
	"$(INTDIR)\mailbrowser.obj" \
	"$(INTDIR)\decode.obj" \
	"$(INTDIR)\mails.obj" \
	"$(INTDIR)\mime.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\netlib.obj" \
	"$(INTDIR)\pop3.obj" \
	"$(INTDIR)\pop3comm.obj" \
	"$(INTDIR)\pop3opt.obj" \
	"$(INTDIR)\ssl.obj" \
	"$(INTDIR)\account.obj" \
	"$(INTDIR)\debug.obj" \
	"$(INTDIR)\filterplugin.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\protoplugin.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\synchro.obj" \
	"$(INTDIR)\yamn.obj" \
	"$(INTDIR)\YAMN.res"

"..\..\bin\Debug\plugins\YAMN-9x\YAMN.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("YAMN.dep")
!INCLUDE "YAMN.dep"
!ELSE 
!MESSAGE Warning: cannot find "YAMN.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "YAMN - Win32 Release Win2in1" || "$(CFG)" == "YAMN - Win32 Debug Win2in1" || "$(CFG)" == "YAMN - Win32 Release" || "$(CFG)" == "YAMN - Win32 Debug" || "$(CFG)" == "YAMN - Win32 Release Win9x" || "$(CFG)" == "YAMN - Win32 Debug Win9x"
SOURCE=.\browser\badconnect.cpp

"$(INTDIR)\badconnect.obj"	"$(INTDIR)\badconnect.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\browser\mailbrowser.cpp

"$(INTDIR)\mailbrowser.obj"	"$(INTDIR)\mailbrowser.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mails\decode.cpp

"$(INTDIR)\decode.obj"	"$(INTDIR)\decode.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mails\mails.cpp

"$(INTDIR)\mails.obj"	"$(INTDIR)\mails.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mails\mime.cpp

"$(INTDIR)\mime.obj"	"$(INTDIR)\mime.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\proto\md5.c

"$(INTDIR)\md5.obj"	"$(INTDIR)\md5.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\proto\netlib.cpp

"$(INTDIR)\netlib.obj"	"$(INTDIR)\netlib.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\proto\pop3\pop3.cpp

"$(INTDIR)\pop3.obj"	"$(INTDIR)\pop3.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\proto\pop3\pop3comm.cpp

"$(INTDIR)\pop3comm.obj"	"$(INTDIR)\pop3comm.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\proto\pop3\pop3opt.cpp

"$(INTDIR)\pop3opt.obj"	"$(INTDIR)\pop3opt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\proto\ssl.cpp

"$(INTDIR)\ssl.obj"	"$(INTDIR)\ssl.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\account.cpp

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"

CPP_SWITCHES=/nologo /Zp4 /MD /W3 /GX /Zi /O1 /Ob0 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WIN2IN1" /Fp"$(INTDIR)\YAMN.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\account.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"

CPP_SWITCHES=/nologo /Zp4 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../include" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_DEBUG" /D "WIN2IN1" /Fp"$(INTDIR)\YAMN.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\account.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"

CPP_SWITCHES=/nologo /Zp4 /MD /W3 /GX /O1 /Ob0 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\YAMN.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\account.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"

CPP_SWITCHES=/nologo /Zp4 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../include" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_DEBUG" /Fp"$(INTDIR)\YAMN.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\account.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"

CPP_SWITCHES=/nologo /Zp4 /MD /W3 /GX /O1 /Ob0 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WIN9X" /Fp"$(INTDIR)\YAMN.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\account.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"

CPP_SWITCHES=/nologo /Zp4 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WIN9X" /D "YAMN_VER_BETA" /Fp"$(INTDIR)\YAMN.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\account.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\debug.cpp

"$(INTDIR)\debug.obj"	"$(INTDIR)\debug.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\filterplugin.cpp

"$(INTDIR)\filterplugin.obj"	"$(INTDIR)\filterplugin.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\main.cpp

"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\protoplugin.cpp

"$(INTDIR)\protoplugin.obj"	"$(INTDIR)\protoplugin.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\services.cpp

"$(INTDIR)\services.obj"	"$(INTDIR)\services.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\synchro.cpp

"$(INTDIR)\synchro.obj"	"$(INTDIR)\synchro.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\yamn.cpp

"$(INTDIR)\yamn.obj"	"$(INTDIR)\yamn.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\resources\YAMN.rc

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\YAMN.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x417 /fo"$(INTDIR)\YAMN.res" /i "resources" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\YAMN.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x417 /fo"$(INTDIR)\YAMN.res" /i "resources" /d "_DEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\YAMN.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x417 /fo"$(INTDIR)\YAMN.res" /i "resources" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\YAMN.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x417 /fo"$(INTDIR)\YAMN.res" /i "resources" /d "_DEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\YAMN.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x417 /fo"$(INTDIR)\YAMN.res" /i "resources" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\YAMN.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x417 /fo"$(INTDIR)\YAMN.res" /i "resources" /d "_DEBUG" $(SOURCE)


!ENDIF 


!ENDIF 

