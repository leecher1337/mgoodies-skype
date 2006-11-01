# Microsoft Developer Studio Generated NMAKE File, Based on jabber.dsp
!IF "$(CFG)" == ""
CFG=jgmail - Win32 Release Unicode
!MESSAGE No configuration specified. Defaulting to jgmail - Win32 Release Unicode.
!ENDIF 

!IF "$(CFG)" != "jgmail - Win32 Release" && "$(CFG)" != "jgmail - Win32 Debug" && "$(CFG)" != "jgmail - Win32 Release Unicode" && "$(CFG)" != "jgmail - Win32 Debug Unicode" && "$(CFG)" != "jgmail - Win32 Static Unicode" && "$(CFG)" != "jgmail - Win32 Static"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "jgmail - Win32 Release"

OUTDIR=.\compile/release/ansi
INTDIR=.\compile/release/ansi

ALL : "..\..\bin\upload\jabber\JGmail.dll"


CLEAN :
	-@erase "$(INTDIR)\gmail.obj"
	-@erase "$(INTDIR)\google_token.obj"
	-@erase "$(INTDIR)\icolib.obj"
	-@erase "$(INTDIR)\jabber.obj"
	-@erase "$(INTDIR)\jabber.pch"
	-@erase "$(INTDIR)\jabber_agent.obj"
	-@erase "$(INTDIR)\jabber_bitmap.obj"
	-@erase "$(INTDIR)\jabber_byte.obj"
	-@erase "$(INTDIR)\jabber_chat.obj"
	-@erase "$(INTDIR)\jabber_deprecated.obj"
	-@erase "$(INTDIR)\jabber_file.obj"
	-@erase "$(INTDIR)\jabber_form.obj"
	-@erase "$(INTDIR)\jabber_ft.obj"
	-@erase "$(INTDIR)\jabber_groupchat.obj"
	-@erase "$(INTDIR)\jabber_icolib.obj"
	-@erase "$(INTDIR)\jabber_iq.obj"
	-@erase "$(INTDIR)\jabber_iqid.obj"
	-@erase "$(INTDIR)\jabber_iqid_muc.obj"
	-@erase "$(INTDIR)\jabber_libstr.obj"
	-@erase "$(INTDIR)\jabber_list.obj"
	-@erase "$(INTDIR)\jabber_menu.obj"
	-@erase "$(INTDIR)\jabber_misc.obj"
	-@erase "$(INTDIR)\jabber_opt.obj"
	-@erase "$(INTDIR)\jabber_password.obj"
	-@erase "$(INTDIR)\jabber_proxy.obj"
	-@erase "$(INTDIR)\jabber_ssl.obj"
	-@erase "$(INTDIR)\jabber_std.obj"
	-@erase "$(INTDIR)\jabber_svc.obj"
	-@erase "$(INTDIR)\jabber_thread.obj"
	-@erase "$(INTDIR)\jabber_userinfo.obj"
	-@erase "$(INTDIR)\jabber_util.obj"
	-@erase "$(INTDIR)\jabber_vcard.obj"
	-@erase "$(INTDIR)\jabber_ws.obj"
	-@erase "$(INTDIR)\jabber_xml.obj"
	-@erase "$(INTDIR)\jabber_xmlns.obj"
	-@erase "$(INTDIR)\msvc6.res"
	-@erase "$(INTDIR)\sha1.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\JGmail.exp"
	-@erase "$(OUTDIR)\JGmail.map"
	-@erase "$(OUTDIR)\JGmail.pdb"
	-@erase "..\..\bin\upload\jabber\JGmail.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\msvc6.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\jabber.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib /nologo /base:"0x32500000" /dll /incremental:no /pdb:"$(OUTDIR)\JGmail.pdb" /map:"$(INTDIR)\JGmail.map" /debug /machine:I386 /out:"../../bin/upload/jabber/JGmail.dll" /implib:"$(OUTDIR)\JGmail.lib" /ALIGN:4096 /ignore:4108 
LINK32_OBJS= \
	"$(INTDIR)\gmail.obj" \
	"$(INTDIR)\google_token.obj" \
	"$(INTDIR)\icolib.obj" \
	"$(INTDIR)\jabber.obj" \
	"$(INTDIR)\jabber_agent.obj" \
	"$(INTDIR)\jabber_bitmap.obj" \
	"$(INTDIR)\jabber_byte.obj" \
	"$(INTDIR)\jabber_chat.obj" \
	"$(INTDIR)\jabber_deprecated.obj" \
	"$(INTDIR)\jabber_file.obj" \
	"$(INTDIR)\jabber_form.obj" \
	"$(INTDIR)\jabber_ft.obj" \
	"$(INTDIR)\jabber_groupchat.obj" \
	"$(INTDIR)\jabber_icolib.obj" \
	"$(INTDIR)\jabber_iq.obj" \
	"$(INTDIR)\jabber_iqid.obj" \
	"$(INTDIR)\jabber_iqid_muc.obj" \
	"$(INTDIR)\jabber_libstr.obj" \
	"$(INTDIR)\jabber_list.obj" \
	"$(INTDIR)\jabber_menu.obj" \
	"$(INTDIR)\jabber_misc.obj" \
	"$(INTDIR)\jabber_opt.obj" \
	"$(INTDIR)\jabber_password.obj" \
	"$(INTDIR)\jabber_proxy.obj" \
	"$(INTDIR)\jabber_ssl.obj" \
	"$(INTDIR)\jabber_std.obj" \
	"$(INTDIR)\jabber_svc.obj" \
	"$(INTDIR)\jabber_thread.obj" \
	"$(INTDIR)\jabber_userinfo.obj" \
	"$(INTDIR)\jabber_util.obj" \
	"$(INTDIR)\jabber_vcard.obj" \
	"$(INTDIR)\jabber_ws.obj" \
	"$(INTDIR)\jabber_xml.obj" \
	"$(INTDIR)\jabber_xmlns.obj" \
	"$(INTDIR)\sha1.obj" \
	"$(INTDIR)\msvc6.res"

"..\..\bin\upload\jabber\JGmail.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

$(DS_POSTBUILD_DEP) : "..\..\bin\upload\jabber\JGmail.dll"
   C:\WINDOWS\system32\cmd.exe /c "cd ../../bin/upload/ && md5 -s -t -ojabber/JGmail.dll.md5 jabber/JGmail.dll"
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

OUTDIR=.\compile/debug/ansi
INTDIR=.\compile/debug/ansi
# Begin Custom Macros
OutDir=.\compile/debug/ansi
# End Custom Macros

ALL : "..\..\bin\debug\plugins\JGmail.dll" "$(OUTDIR)\jabber.bsc"


CLEAN :
	-@erase "$(INTDIR)\gmail.obj"
	-@erase "$(INTDIR)\gmail.sbr"
	-@erase "$(INTDIR)\google_token.obj"
	-@erase "$(INTDIR)\google_token.sbr"
	-@erase "$(INTDIR)\icolib.obj"
	-@erase "$(INTDIR)\icolib.sbr"
	-@erase "$(INTDIR)\jabber.obj"
	-@erase "$(INTDIR)\jabber.pch"
	-@erase "$(INTDIR)\jabber.sbr"
	-@erase "$(INTDIR)\jabber_agent.obj"
	-@erase "$(INTDIR)\jabber_agent.sbr"
	-@erase "$(INTDIR)\jabber_bitmap.obj"
	-@erase "$(INTDIR)\jabber_bitmap.sbr"
	-@erase "$(INTDIR)\jabber_byte.obj"
	-@erase "$(INTDIR)\jabber_byte.sbr"
	-@erase "$(INTDIR)\jabber_chat.obj"
	-@erase "$(INTDIR)\jabber_chat.sbr"
	-@erase "$(INTDIR)\jabber_deprecated.obj"
	-@erase "$(INTDIR)\jabber_deprecated.sbr"
	-@erase "$(INTDIR)\jabber_file.obj"
	-@erase "$(INTDIR)\jabber_file.sbr"
	-@erase "$(INTDIR)\jabber_form.obj"
	-@erase "$(INTDIR)\jabber_form.sbr"
	-@erase "$(INTDIR)\jabber_ft.obj"
	-@erase "$(INTDIR)\jabber_ft.sbr"
	-@erase "$(INTDIR)\jabber_groupchat.obj"
	-@erase "$(INTDIR)\jabber_groupchat.sbr"
	-@erase "$(INTDIR)\jabber_icolib.obj"
	-@erase "$(INTDIR)\jabber_icolib.sbr"
	-@erase "$(INTDIR)\jabber_iq.obj"
	-@erase "$(INTDIR)\jabber_iq.sbr"
	-@erase "$(INTDIR)\jabber_iqid.obj"
	-@erase "$(INTDIR)\jabber_iqid.sbr"
	-@erase "$(INTDIR)\jabber_iqid_muc.obj"
	-@erase "$(INTDIR)\jabber_iqid_muc.sbr"
	-@erase "$(INTDIR)\jabber_libstr.obj"
	-@erase "$(INTDIR)\jabber_libstr.sbr"
	-@erase "$(INTDIR)\jabber_list.obj"
	-@erase "$(INTDIR)\jabber_list.sbr"
	-@erase "$(INTDIR)\jabber_menu.obj"
	-@erase "$(INTDIR)\jabber_menu.sbr"
	-@erase "$(INTDIR)\jabber_misc.obj"
	-@erase "$(INTDIR)\jabber_misc.sbr"
	-@erase "$(INTDIR)\jabber_opt.obj"
	-@erase "$(INTDIR)\jabber_opt.sbr"
	-@erase "$(INTDIR)\jabber_password.obj"
	-@erase "$(INTDIR)\jabber_password.sbr"
	-@erase "$(INTDIR)\jabber_proxy.obj"
	-@erase "$(INTDIR)\jabber_proxy.sbr"
	-@erase "$(INTDIR)\jabber_ssl.obj"
	-@erase "$(INTDIR)\jabber_ssl.sbr"
	-@erase "$(INTDIR)\jabber_std.obj"
	-@erase "$(INTDIR)\jabber_std.sbr"
	-@erase "$(INTDIR)\jabber_svc.obj"
	-@erase "$(INTDIR)\jabber_svc.sbr"
	-@erase "$(INTDIR)\jabber_thread.obj"
	-@erase "$(INTDIR)\jabber_thread.sbr"
	-@erase "$(INTDIR)\jabber_userinfo.obj"
	-@erase "$(INTDIR)\jabber_userinfo.sbr"
	-@erase "$(INTDIR)\jabber_util.obj"
	-@erase "$(INTDIR)\jabber_util.sbr"
	-@erase "$(INTDIR)\jabber_vcard.obj"
	-@erase "$(INTDIR)\jabber_vcard.sbr"
	-@erase "$(INTDIR)\jabber_ws.obj"
	-@erase "$(INTDIR)\jabber_ws.sbr"
	-@erase "$(INTDIR)\jabber_xml.obj"
	-@erase "$(INTDIR)\jabber_xml.sbr"
	-@erase "$(INTDIR)\jabber_xmlns.obj"
	-@erase "$(INTDIR)\jabber_xmlns.sbr"
	-@erase "$(INTDIR)\msvc6.res"
	-@erase "$(INTDIR)\sha1.obj"
	-@erase "$(INTDIR)\sha1.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\jabber.bsc"
	-@erase "$(OUTDIR)\JGmail.exp"
	-@erase "$(OUTDIR)\JGmail.map"
	-@erase "$(OUTDIR)\JGmail.pdb"
	-@erase "..\..\bin\debug\plugins\JGmail.dll"
	-@erase "..\..\bin\debug\plugins\JGmail.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\msvc6.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\jabber.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\gmail.sbr" \
	"$(INTDIR)\google_token.sbr" \
	"$(INTDIR)\icolib.sbr" \
	"$(INTDIR)\jabber.sbr" \
	"$(INTDIR)\jabber_agent.sbr" \
	"$(INTDIR)\jabber_bitmap.sbr" \
	"$(INTDIR)\jabber_byte.sbr" \
	"$(INTDIR)\jabber_chat.sbr" \
	"$(INTDIR)\jabber_deprecated.sbr" \
	"$(INTDIR)\jabber_file.sbr" \
	"$(INTDIR)\jabber_form.sbr" \
	"$(INTDIR)\jabber_ft.sbr" \
	"$(INTDIR)\jabber_groupchat.sbr" \
	"$(INTDIR)\jabber_icolib.sbr" \
	"$(INTDIR)\jabber_iq.sbr" \
	"$(INTDIR)\jabber_iqid.sbr" \
	"$(INTDIR)\jabber_iqid_muc.sbr" \
	"$(INTDIR)\jabber_libstr.sbr" \
	"$(INTDIR)\jabber_list.sbr" \
	"$(INTDIR)\jabber_menu.sbr" \
	"$(INTDIR)\jabber_misc.sbr" \
	"$(INTDIR)\jabber_opt.sbr" \
	"$(INTDIR)\jabber_password.sbr" \
	"$(INTDIR)\jabber_proxy.sbr" \
	"$(INTDIR)\jabber_ssl.sbr" \
	"$(INTDIR)\jabber_std.sbr" \
	"$(INTDIR)\jabber_svc.sbr" \
	"$(INTDIR)\jabber_thread.sbr" \
	"$(INTDIR)\jabber_userinfo.sbr" \
	"$(INTDIR)\jabber_util.sbr" \
	"$(INTDIR)\jabber_vcard.sbr" \
	"$(INTDIR)\jabber_ws.sbr" \
	"$(INTDIR)\jabber_xml.sbr" \
	"$(INTDIR)\jabber_xmlns.sbr" \
	"$(INTDIR)\sha1.sbr"

"$(OUTDIR)\jabber.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib /nologo /base:"0x32500000" /dll /incremental:yes /pdb:"$(OUTDIR)\JGmail.pdb" /map:"$(INTDIR)\JGmail.map" /debug /machine:I386 /out:"../../bin/debug/plugins/JGmail.dll" /implib:"$(OUTDIR)\JGmail.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\gmail.obj" \
	"$(INTDIR)\google_token.obj" \
	"$(INTDIR)\icolib.obj" \
	"$(INTDIR)\jabber.obj" \
	"$(INTDIR)\jabber_agent.obj" \
	"$(INTDIR)\jabber_bitmap.obj" \
	"$(INTDIR)\jabber_byte.obj" \
	"$(INTDIR)\jabber_chat.obj" \
	"$(INTDIR)\jabber_deprecated.obj" \
	"$(INTDIR)\jabber_file.obj" \
	"$(INTDIR)\jabber_form.obj" \
	"$(INTDIR)\jabber_ft.obj" \
	"$(INTDIR)\jabber_groupchat.obj" \
	"$(INTDIR)\jabber_icolib.obj" \
	"$(INTDIR)\jabber_iq.obj" \
	"$(INTDIR)\jabber_iqid.obj" \
	"$(INTDIR)\jabber_iqid_muc.obj" \
	"$(INTDIR)\jabber_libstr.obj" \
	"$(INTDIR)\jabber_list.obj" \
	"$(INTDIR)\jabber_menu.obj" \
	"$(INTDIR)\jabber_misc.obj" \
	"$(INTDIR)\jabber_opt.obj" \
	"$(INTDIR)\jabber_password.obj" \
	"$(INTDIR)\jabber_proxy.obj" \
	"$(INTDIR)\jabber_ssl.obj" \
	"$(INTDIR)\jabber_std.obj" \
	"$(INTDIR)\jabber_svc.obj" \
	"$(INTDIR)\jabber_thread.obj" \
	"$(INTDIR)\jabber_userinfo.obj" \
	"$(INTDIR)\jabber_util.obj" \
	"$(INTDIR)\jabber_vcard.obj" \
	"$(INTDIR)\jabber_ws.obj" \
	"$(INTDIR)\jabber_xml.obj" \
	"$(INTDIR)\jabber_xmlns.obj" \
	"$(INTDIR)\sha1.obj" \
	"$(INTDIR)\msvc6.res"

"..\..\bin\debug\plugins\JGmail.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

OUTDIR=.\compile/release/unicode
INTDIR=.\compile/release/unicode

ALL : "..\..\bin\upload\jabber\u\JGmail.dll"


CLEAN :
	-@erase "$(INTDIR)\gmail.obj"
	-@erase "$(INTDIR)\google_token.obj"
	-@erase "$(INTDIR)\icolib.obj"
	-@erase "$(INTDIR)\jabber.obj"
	-@erase "$(INTDIR)\jabber.pch"
	-@erase "$(INTDIR)\jabber_agent.obj"
	-@erase "$(INTDIR)\jabber_bitmap.obj"
	-@erase "$(INTDIR)\jabber_byte.obj"
	-@erase "$(INTDIR)\jabber_chat.obj"
	-@erase "$(INTDIR)\jabber_deprecated.obj"
	-@erase "$(INTDIR)\jabber_file.obj"
	-@erase "$(INTDIR)\jabber_form.obj"
	-@erase "$(INTDIR)\jabber_ft.obj"
	-@erase "$(INTDIR)\jabber_groupchat.obj"
	-@erase "$(INTDIR)\jabber_icolib.obj"
	-@erase "$(INTDIR)\jabber_iq.obj"
	-@erase "$(INTDIR)\jabber_iqid.obj"
	-@erase "$(INTDIR)\jabber_iqid_muc.obj"
	-@erase "$(INTDIR)\jabber_libstr.obj"
	-@erase "$(INTDIR)\jabber_list.obj"
	-@erase "$(INTDIR)\jabber_menu.obj"
	-@erase "$(INTDIR)\jabber_misc.obj"
	-@erase "$(INTDIR)\jabber_opt.obj"
	-@erase "$(INTDIR)\jabber_password.obj"
	-@erase "$(INTDIR)\jabber_proxy.obj"
	-@erase "$(INTDIR)\jabber_ssl.obj"
	-@erase "$(INTDIR)\jabber_std.obj"
	-@erase "$(INTDIR)\jabber_svc.obj"
	-@erase "$(INTDIR)\jabber_thread.obj"
	-@erase "$(INTDIR)\jabber_userinfo.obj"
	-@erase "$(INTDIR)\jabber_util.obj"
	-@erase "$(INTDIR)\jabber_vcard.obj"
	-@erase "$(INTDIR)\jabber_ws.obj"
	-@erase "$(INTDIR)\jabber_xml.obj"
	-@erase "$(INTDIR)\jabber_xmlns.obj"
	-@erase "$(INTDIR)\msvc6.res"
	-@erase "$(INTDIR)\sha1.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\JGmail.exp"
	-@erase "$(OUTDIR)\JGmail.map"
	-@erase "$(OUTDIR)\JGmail.pdb"
	-@erase "..\..\bin\upload\jabber\u\JGmail.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\msvc6.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\jabber.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib /nologo /base:"0x32500000" /dll /incremental:no /pdb:"$(OUTDIR)\JGmail.pdb" /map:"$(INTDIR)\JGmail.map" /debug /machine:I386 /out:"../../bin/upload/jabber/u/JGmail.dll" /implib:"$(OUTDIR)\JGmail.lib" /ALIGN:4096 /ignore:4108 
LINK32_OBJS= \
	"$(INTDIR)\gmail.obj" \
	"$(INTDIR)\google_token.obj" \
	"$(INTDIR)\icolib.obj" \
	"$(INTDIR)\jabber.obj" \
	"$(INTDIR)\jabber_agent.obj" \
	"$(INTDIR)\jabber_bitmap.obj" \
	"$(INTDIR)\jabber_byte.obj" \
	"$(INTDIR)\jabber_chat.obj" \
	"$(INTDIR)\jabber_deprecated.obj" \
	"$(INTDIR)\jabber_file.obj" \
	"$(INTDIR)\jabber_form.obj" \
	"$(INTDIR)\jabber_ft.obj" \
	"$(INTDIR)\jabber_groupchat.obj" \
	"$(INTDIR)\jabber_icolib.obj" \
	"$(INTDIR)\jabber_iq.obj" \
	"$(INTDIR)\jabber_iqid.obj" \
	"$(INTDIR)\jabber_iqid_muc.obj" \
	"$(INTDIR)\jabber_libstr.obj" \
	"$(INTDIR)\jabber_list.obj" \
	"$(INTDIR)\jabber_menu.obj" \
	"$(INTDIR)\jabber_misc.obj" \
	"$(INTDIR)\jabber_opt.obj" \
	"$(INTDIR)\jabber_password.obj" \
	"$(INTDIR)\jabber_proxy.obj" \
	"$(INTDIR)\jabber_ssl.obj" \
	"$(INTDIR)\jabber_std.obj" \
	"$(INTDIR)\jabber_svc.obj" \
	"$(INTDIR)\jabber_thread.obj" \
	"$(INTDIR)\jabber_userinfo.obj" \
	"$(INTDIR)\jabber_util.obj" \
	"$(INTDIR)\jabber_vcard.obj" \
	"$(INTDIR)\jabber_ws.obj" \
	"$(INTDIR)\jabber_xml.obj" \
	"$(INTDIR)\jabber_xmlns.obj" \
	"$(INTDIR)\sha1.obj" \
	"$(INTDIR)\msvc6.res"

"..\..\bin\upload\jabber\u\JGmail.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

$(DS_POSTBUILD_DEP) : "..\..\bin\upload\jabber\u\JGmail.dll"
   C:\WINDOWS\system32\cmd.exe /c "cd ../../bin/upload/ && md5 -s -t -ojabber/u/JGmail.dll.md5 jabber/u/JGmail.dll"
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

OUTDIR=.\compile/debug/unicode
INTDIR=.\compile/debug/unicode
# Begin Custom Macros
OutDir=.\compile/debug/unicode
# End Custom Macros

ALL : "..\..\bin\Debug Unicode\plugins\JGmail.dll" "$(OUTDIR)\jabber.bsc"


CLEAN :
	-@erase "$(INTDIR)\gmail.obj"
	-@erase "$(INTDIR)\gmail.sbr"
	-@erase "$(INTDIR)\google_token.obj"
	-@erase "$(INTDIR)\google_token.sbr"
	-@erase "$(INTDIR)\icolib.obj"
	-@erase "$(INTDIR)\icolib.sbr"
	-@erase "$(INTDIR)\jabber.obj"
	-@erase "$(INTDIR)\jabber.pch"
	-@erase "$(INTDIR)\jabber.sbr"
	-@erase "$(INTDIR)\jabber_agent.obj"
	-@erase "$(INTDIR)\jabber_agent.sbr"
	-@erase "$(INTDIR)\jabber_bitmap.obj"
	-@erase "$(INTDIR)\jabber_bitmap.sbr"
	-@erase "$(INTDIR)\jabber_byte.obj"
	-@erase "$(INTDIR)\jabber_byte.sbr"
	-@erase "$(INTDIR)\jabber_chat.obj"
	-@erase "$(INTDIR)\jabber_chat.sbr"
	-@erase "$(INTDIR)\jabber_deprecated.obj"
	-@erase "$(INTDIR)\jabber_deprecated.sbr"
	-@erase "$(INTDIR)\jabber_file.obj"
	-@erase "$(INTDIR)\jabber_file.sbr"
	-@erase "$(INTDIR)\jabber_form.obj"
	-@erase "$(INTDIR)\jabber_form.sbr"
	-@erase "$(INTDIR)\jabber_ft.obj"
	-@erase "$(INTDIR)\jabber_ft.sbr"
	-@erase "$(INTDIR)\jabber_groupchat.obj"
	-@erase "$(INTDIR)\jabber_groupchat.sbr"
	-@erase "$(INTDIR)\jabber_icolib.obj"
	-@erase "$(INTDIR)\jabber_icolib.sbr"
	-@erase "$(INTDIR)\jabber_iq.obj"
	-@erase "$(INTDIR)\jabber_iq.sbr"
	-@erase "$(INTDIR)\jabber_iqid.obj"
	-@erase "$(INTDIR)\jabber_iqid.sbr"
	-@erase "$(INTDIR)\jabber_iqid_muc.obj"
	-@erase "$(INTDIR)\jabber_iqid_muc.sbr"
	-@erase "$(INTDIR)\jabber_libstr.obj"
	-@erase "$(INTDIR)\jabber_libstr.sbr"
	-@erase "$(INTDIR)\jabber_list.obj"
	-@erase "$(INTDIR)\jabber_list.sbr"
	-@erase "$(INTDIR)\jabber_menu.obj"
	-@erase "$(INTDIR)\jabber_menu.sbr"
	-@erase "$(INTDIR)\jabber_misc.obj"
	-@erase "$(INTDIR)\jabber_misc.sbr"
	-@erase "$(INTDIR)\jabber_opt.obj"
	-@erase "$(INTDIR)\jabber_opt.sbr"
	-@erase "$(INTDIR)\jabber_password.obj"
	-@erase "$(INTDIR)\jabber_password.sbr"
	-@erase "$(INTDIR)\jabber_proxy.obj"
	-@erase "$(INTDIR)\jabber_proxy.sbr"
	-@erase "$(INTDIR)\jabber_ssl.obj"
	-@erase "$(INTDIR)\jabber_ssl.sbr"
	-@erase "$(INTDIR)\jabber_std.obj"
	-@erase "$(INTDIR)\jabber_std.sbr"
	-@erase "$(INTDIR)\jabber_svc.obj"
	-@erase "$(INTDIR)\jabber_svc.sbr"
	-@erase "$(INTDIR)\jabber_thread.obj"
	-@erase "$(INTDIR)\jabber_thread.sbr"
	-@erase "$(INTDIR)\jabber_userinfo.obj"
	-@erase "$(INTDIR)\jabber_userinfo.sbr"
	-@erase "$(INTDIR)\jabber_util.obj"
	-@erase "$(INTDIR)\jabber_util.sbr"
	-@erase "$(INTDIR)\jabber_vcard.obj"
	-@erase "$(INTDIR)\jabber_vcard.sbr"
	-@erase "$(INTDIR)\jabber_ws.obj"
	-@erase "$(INTDIR)\jabber_ws.sbr"
	-@erase "$(INTDIR)\jabber_xml.obj"
	-@erase "$(INTDIR)\jabber_xml.sbr"
	-@erase "$(INTDIR)\jabber_xmlns.obj"
	-@erase "$(INTDIR)\jabber_xmlns.sbr"
	-@erase "$(INTDIR)\msvc6.res"
	-@erase "$(INTDIR)\sha1.obj"
	-@erase "$(INTDIR)\sha1.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\jabber.bsc"
	-@erase "$(OUTDIR)\JGmail.exp"
	-@erase "$(OUTDIR)\JGmail.map"
	-@erase "$(OUTDIR)\JGmail.pdb"
	-@erase "..\..\bin\Debug Unicode\plugins\JGmail.dll"
	-@erase "..\..\bin\Debug Unicode\plugins\JGmail.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\msvc6.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\jabber.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\gmail.sbr" \
	"$(INTDIR)\google_token.sbr" \
	"$(INTDIR)\icolib.sbr" \
	"$(INTDIR)\jabber.sbr" \
	"$(INTDIR)\jabber_agent.sbr" \
	"$(INTDIR)\jabber_bitmap.sbr" \
	"$(INTDIR)\jabber_byte.sbr" \
	"$(INTDIR)\jabber_chat.sbr" \
	"$(INTDIR)\jabber_deprecated.sbr" \
	"$(INTDIR)\jabber_file.sbr" \
	"$(INTDIR)\jabber_form.sbr" \
	"$(INTDIR)\jabber_ft.sbr" \
	"$(INTDIR)\jabber_groupchat.sbr" \
	"$(INTDIR)\jabber_icolib.sbr" \
	"$(INTDIR)\jabber_iq.sbr" \
	"$(INTDIR)\jabber_iqid.sbr" \
	"$(INTDIR)\jabber_iqid_muc.sbr" \
	"$(INTDIR)\jabber_libstr.sbr" \
	"$(INTDIR)\jabber_list.sbr" \
	"$(INTDIR)\jabber_menu.sbr" \
	"$(INTDIR)\jabber_misc.sbr" \
	"$(INTDIR)\jabber_opt.sbr" \
	"$(INTDIR)\jabber_password.sbr" \
	"$(INTDIR)\jabber_proxy.sbr" \
	"$(INTDIR)\jabber_ssl.sbr" \
	"$(INTDIR)\jabber_std.sbr" \
	"$(INTDIR)\jabber_svc.sbr" \
	"$(INTDIR)\jabber_thread.sbr" \
	"$(INTDIR)\jabber_userinfo.sbr" \
	"$(INTDIR)\jabber_util.sbr" \
	"$(INTDIR)\jabber_vcard.sbr" \
	"$(INTDIR)\jabber_ws.sbr" \
	"$(INTDIR)\jabber_xml.sbr" \
	"$(INTDIR)\jabber_xmlns.sbr" \
	"$(INTDIR)\sha1.sbr"

"$(OUTDIR)\jabber.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib /nologo /base:"0x32500000" /dll /incremental:yes /pdb:"$(OUTDIR)\JGmail.pdb" /map:"$(INTDIR)\JGmail.map" /debug /machine:I386 /out:"../../bin/Debug Unicode/plugins/JGmail.dll" /implib:"$(OUTDIR)\JGmail.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\gmail.obj" \
	"$(INTDIR)\google_token.obj" \
	"$(INTDIR)\icolib.obj" \
	"$(INTDIR)\jabber.obj" \
	"$(INTDIR)\jabber_agent.obj" \
	"$(INTDIR)\jabber_bitmap.obj" \
	"$(INTDIR)\jabber_byte.obj" \
	"$(INTDIR)\jabber_chat.obj" \
	"$(INTDIR)\jabber_deprecated.obj" \
	"$(INTDIR)\jabber_file.obj" \
	"$(INTDIR)\jabber_form.obj" \
	"$(INTDIR)\jabber_ft.obj" \
	"$(INTDIR)\jabber_groupchat.obj" \
	"$(INTDIR)\jabber_icolib.obj" \
	"$(INTDIR)\jabber_iq.obj" \
	"$(INTDIR)\jabber_iqid.obj" \
	"$(INTDIR)\jabber_iqid_muc.obj" \
	"$(INTDIR)\jabber_libstr.obj" \
	"$(INTDIR)\jabber_list.obj" \
	"$(INTDIR)\jabber_menu.obj" \
	"$(INTDIR)\jabber_misc.obj" \
	"$(INTDIR)\jabber_opt.obj" \
	"$(INTDIR)\jabber_password.obj" \
	"$(INTDIR)\jabber_proxy.obj" \
	"$(INTDIR)\jabber_ssl.obj" \
	"$(INTDIR)\jabber_std.obj" \
	"$(INTDIR)\jabber_svc.obj" \
	"$(INTDIR)\jabber_thread.obj" \
	"$(INTDIR)\jabber_userinfo.obj" \
	"$(INTDIR)\jabber_util.obj" \
	"$(INTDIR)\jabber_vcard.obj" \
	"$(INTDIR)\jabber_ws.obj" \
	"$(INTDIR)\jabber_xml.obj" \
	"$(INTDIR)\jabber_xmlns.obj" \
	"$(INTDIR)\sha1.obj" \
	"$(INTDIR)\msvc6.res"

"..\..\bin\Debug Unicode\plugins\JGmail.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

OUTDIR=.\compile/staticssl/unicode
INTDIR=.\compile/staticssl/unicode

ALL : "..\..\bin\upload\jabber\staticssl\u\JGmail.dll"


CLEAN :
	-@erase "$(INTDIR)\gmail.obj"
	-@erase "$(INTDIR)\google_token.obj"
	-@erase "$(INTDIR)\icolib.obj"
	-@erase "$(INTDIR)\jabber.obj"
	-@erase "$(INTDIR)\jabber.pch"
	-@erase "$(INTDIR)\jabber_agent.obj"
	-@erase "$(INTDIR)\jabber_bitmap.obj"
	-@erase "$(INTDIR)\jabber_byte.obj"
	-@erase "$(INTDIR)\jabber_chat.obj"
	-@erase "$(INTDIR)\jabber_deprecated.obj"
	-@erase "$(INTDIR)\jabber_file.obj"
	-@erase "$(INTDIR)\jabber_form.obj"
	-@erase "$(INTDIR)\jabber_ft.obj"
	-@erase "$(INTDIR)\jabber_groupchat.obj"
	-@erase "$(INTDIR)\jabber_icolib.obj"
	-@erase "$(INTDIR)\jabber_iq.obj"
	-@erase "$(INTDIR)\jabber_iqid.obj"
	-@erase "$(INTDIR)\jabber_iqid_muc.obj"
	-@erase "$(INTDIR)\jabber_libstr.obj"
	-@erase "$(INTDIR)\jabber_list.obj"
	-@erase "$(INTDIR)\jabber_menu.obj"
	-@erase "$(INTDIR)\jabber_misc.obj"
	-@erase "$(INTDIR)\jabber_opt.obj"
	-@erase "$(INTDIR)\jabber_password.obj"
	-@erase "$(INTDIR)\jabber_proxy.obj"
	-@erase "$(INTDIR)\jabber_ssl.obj"
	-@erase "$(INTDIR)\jabber_std.obj"
	-@erase "$(INTDIR)\jabber_svc.obj"
	-@erase "$(INTDIR)\jabber_thread.obj"
	-@erase "$(INTDIR)\jabber_userinfo.obj"
	-@erase "$(INTDIR)\jabber_util.obj"
	-@erase "$(INTDIR)\jabber_vcard.obj"
	-@erase "$(INTDIR)\jabber_ws.obj"
	-@erase "$(INTDIR)\jabber_xml.obj"
	-@erase "$(INTDIR)\jabber_xmlns.obj"
	-@erase "$(INTDIR)\msvc6.res"
	-@erase "$(INTDIR)\sha1.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\JGmail.exp"
	-@erase "$(OUTDIR)\JGmail.map"
	-@erase "$(OUTDIR)\JGmail.pdb"
	-@erase "..\..\bin\upload\jabber\staticssl\u\JGmail.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\msvc6.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\jabber.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib zlib.lib ssleay32.lib libeay32.lib /nologo /base:"0x32500000" /dll /incremental:no /pdb:"$(OUTDIR)\JGmail.pdb" /map:"$(INTDIR)\JGmail.map" /debug /machine:I386 /out:"../../bin/upload/jabber/staticssl/u/JGmail.dll" /implib:"$(OUTDIR)\JGmail.lib" /libpath:"z:\temp\openssl\zlib-1.2.3\projects\visualc6\Win32_LIB_ASM_Release\\" /libpath:"Z:\temp\openssl\openssl-0.9.8d\out32\\" /ALIGN:4096 /ignore:4108 
LINK32_OBJS= \
	"$(INTDIR)\gmail.obj" \
	"$(INTDIR)\google_token.obj" \
	"$(INTDIR)\icolib.obj" \
	"$(INTDIR)\jabber.obj" \
	"$(INTDIR)\jabber_agent.obj" \
	"$(INTDIR)\jabber_bitmap.obj" \
	"$(INTDIR)\jabber_byte.obj" \
	"$(INTDIR)\jabber_chat.obj" \
	"$(INTDIR)\jabber_deprecated.obj" \
	"$(INTDIR)\jabber_file.obj" \
	"$(INTDIR)\jabber_form.obj" \
	"$(INTDIR)\jabber_ft.obj" \
	"$(INTDIR)\jabber_groupchat.obj" \
	"$(INTDIR)\jabber_icolib.obj" \
	"$(INTDIR)\jabber_iq.obj" \
	"$(INTDIR)\jabber_iqid.obj" \
	"$(INTDIR)\jabber_iqid_muc.obj" \
	"$(INTDIR)\jabber_libstr.obj" \
	"$(INTDIR)\jabber_list.obj" \
	"$(INTDIR)\jabber_menu.obj" \
	"$(INTDIR)\jabber_misc.obj" \
	"$(INTDIR)\jabber_opt.obj" \
	"$(INTDIR)\jabber_password.obj" \
	"$(INTDIR)\jabber_proxy.obj" \
	"$(INTDIR)\jabber_ssl.obj" \
	"$(INTDIR)\jabber_std.obj" \
	"$(INTDIR)\jabber_svc.obj" \
	"$(INTDIR)\jabber_thread.obj" \
	"$(INTDIR)\jabber_userinfo.obj" \
	"$(INTDIR)\jabber_util.obj" \
	"$(INTDIR)\jabber_vcard.obj" \
	"$(INTDIR)\jabber_ws.obj" \
	"$(INTDIR)\jabber_xml.obj" \
	"$(INTDIR)\jabber_xmlns.obj" \
	"$(INTDIR)\sha1.obj" \
	"$(INTDIR)\msvc6.res"

"..\..\bin\upload\jabber\staticssl\u\JGmail.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

$(DS_POSTBUILD_DEP) : "..\..\bin\upload\jabber\staticssl\u\JGmail.dll"
   C:\WINDOWS\system32\cmd.exe /c "cd ../../bin/upload/ && upx --best --force "jabber/staticssl/u/JGmail.dll" && md5 -s -t -ojabber/staticssl/u/JGmail.dll.md5 jabber/staticssl/u/JGmail.dll"
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

OUTDIR=.\compile/staticssl/ansi
INTDIR=.\compile/staticssl/ansi

ALL : "..\..\bin\upload\jabber\staticssl\JGmail.dll"


CLEAN :
	-@erase "$(INTDIR)\gmail.obj"
	-@erase "$(INTDIR)\google_token.obj"
	-@erase "$(INTDIR)\icolib.obj"
	-@erase "$(INTDIR)\jabber.obj"
	-@erase "$(INTDIR)\jabber.pch"
	-@erase "$(INTDIR)\jabber_agent.obj"
	-@erase "$(INTDIR)\jabber_bitmap.obj"
	-@erase "$(INTDIR)\jabber_byte.obj"
	-@erase "$(INTDIR)\jabber_chat.obj"
	-@erase "$(INTDIR)\jabber_deprecated.obj"
	-@erase "$(INTDIR)\jabber_file.obj"
	-@erase "$(INTDIR)\jabber_form.obj"
	-@erase "$(INTDIR)\jabber_ft.obj"
	-@erase "$(INTDIR)\jabber_groupchat.obj"
	-@erase "$(INTDIR)\jabber_icolib.obj"
	-@erase "$(INTDIR)\jabber_iq.obj"
	-@erase "$(INTDIR)\jabber_iqid.obj"
	-@erase "$(INTDIR)\jabber_iqid_muc.obj"
	-@erase "$(INTDIR)\jabber_libstr.obj"
	-@erase "$(INTDIR)\jabber_list.obj"
	-@erase "$(INTDIR)\jabber_menu.obj"
	-@erase "$(INTDIR)\jabber_misc.obj"
	-@erase "$(INTDIR)\jabber_opt.obj"
	-@erase "$(INTDIR)\jabber_password.obj"
	-@erase "$(INTDIR)\jabber_proxy.obj"
	-@erase "$(INTDIR)\jabber_ssl.obj"
	-@erase "$(INTDIR)\jabber_std.obj"
	-@erase "$(INTDIR)\jabber_svc.obj"
	-@erase "$(INTDIR)\jabber_thread.obj"
	-@erase "$(INTDIR)\jabber_userinfo.obj"
	-@erase "$(INTDIR)\jabber_util.obj"
	-@erase "$(INTDIR)\jabber_vcard.obj"
	-@erase "$(INTDIR)\jabber_ws.obj"
	-@erase "$(INTDIR)\jabber_xml.obj"
	-@erase "$(INTDIR)\jabber_xmlns.obj"
	-@erase "$(INTDIR)\msvc6.res"
	-@erase "$(INTDIR)\sha1.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\JGmail.exp"
	-@erase "$(OUTDIR)\JGmail.map"
	-@erase "$(OUTDIR)\JGmail.pdb"
	-@erase "..\..\bin\upload\jabber\staticssl\JGmail.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\msvc6.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\jabber.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib zlib.lib ssleay32.lib libeay32.lib /nologo /base:"0x32500000" /dll /incremental:no /pdb:"$(OUTDIR)\JGmail.pdb" /map:"$(INTDIR)\JGmail.map" /debug /machine:I386 /out:"../../bin/upload/jabber/staticssl/JGmail.dll" /implib:"$(OUTDIR)\JGmail.lib" /libpath:"z:\temp\openssl\zlib-1.2.3\projects\visualc6\Win32_LIB_ASM_Release\\" /libpath:"Z:\temp\openssl\openssl-0.9.8d\out32\\" /ALIGN:4096 /ignore:4108 
LINK32_OBJS= \
	"$(INTDIR)\gmail.obj" \
	"$(INTDIR)\google_token.obj" \
	"$(INTDIR)\icolib.obj" \
	"$(INTDIR)\jabber.obj" \
	"$(INTDIR)\jabber_agent.obj" \
	"$(INTDIR)\jabber_bitmap.obj" \
	"$(INTDIR)\jabber_byte.obj" \
	"$(INTDIR)\jabber_chat.obj" \
	"$(INTDIR)\jabber_deprecated.obj" \
	"$(INTDIR)\jabber_file.obj" \
	"$(INTDIR)\jabber_form.obj" \
	"$(INTDIR)\jabber_ft.obj" \
	"$(INTDIR)\jabber_groupchat.obj" \
	"$(INTDIR)\jabber_icolib.obj" \
	"$(INTDIR)\jabber_iq.obj" \
	"$(INTDIR)\jabber_iqid.obj" \
	"$(INTDIR)\jabber_iqid_muc.obj" \
	"$(INTDIR)\jabber_libstr.obj" \
	"$(INTDIR)\jabber_list.obj" \
	"$(INTDIR)\jabber_menu.obj" \
	"$(INTDIR)\jabber_misc.obj" \
	"$(INTDIR)\jabber_opt.obj" \
	"$(INTDIR)\jabber_password.obj" \
	"$(INTDIR)\jabber_proxy.obj" \
	"$(INTDIR)\jabber_ssl.obj" \
	"$(INTDIR)\jabber_std.obj" \
	"$(INTDIR)\jabber_svc.obj" \
	"$(INTDIR)\jabber_thread.obj" \
	"$(INTDIR)\jabber_userinfo.obj" \
	"$(INTDIR)\jabber_util.obj" \
	"$(INTDIR)\jabber_vcard.obj" \
	"$(INTDIR)\jabber_ws.obj" \
	"$(INTDIR)\jabber_xml.obj" \
	"$(INTDIR)\jabber_xmlns.obj" \
	"$(INTDIR)\sha1.obj" \
	"$(INTDIR)\msvc6.res"

"..\..\bin\upload\jabber\staticssl\JGmail.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

$(DS_POSTBUILD_DEP) : "..\..\bin\upload\jabber\staticssl\JGmail.dll"
   C:\WINDOWS\system32\cmd.exe /c "cd ../../bin/upload/ && upx --best --force jabber/staticssl/JGmail.dll && md5 -s -t -ojabber/staticssl/JGmail.dll.md5 jabber/staticssl/JGmail.dll"
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("jabber.dep")
!INCLUDE "jabber.dep"
!ELSE 
!MESSAGE Warning: cannot find "jabber.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "jgmail - Win32 Release" || "$(CFG)" == "jgmail - Win32 Debug" || "$(CFG)" == "jgmail - Win32 Release Unicode" || "$(CFG)" == "jgmail - Win32 Debug Unicode" || "$(CFG)" == "jgmail - Win32 Static Unicode" || "$(CFG)" == "jgmail - Win32 Static"
SOURCE=.\gmail.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"


"$(INTDIR)\gmail.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"


"$(INTDIR)\gmail.obj"	"$(INTDIR)\gmail.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"


"$(INTDIR)\gmail.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"


"$(INTDIR)\gmail.obj"	"$(INTDIR)\gmail.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"


"$(INTDIR)\gmail.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"


"$(INTDIR)\gmail.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF 

SOURCE=.\google_token.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"


"$(INTDIR)\google_token.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"


"$(INTDIR)\google_token.obj"	"$(INTDIR)\google_token.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"


"$(INTDIR)\google_token.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"


"$(INTDIR)\google_token.obj"	"$(INTDIR)\google_token.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"


"$(INTDIR)\google_token.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"


"$(INTDIR)\google_token.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF 

SOURCE=.\icolib.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"


"$(INTDIR)\icolib.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"


"$(INTDIR)\icolib.obj"	"$(INTDIR)\icolib.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"


"$(INTDIR)\icolib.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"


"$(INTDIR)\icolib.obj"	"$(INTDIR)\icolib.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"


"$(INTDIR)\icolib.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"


"$(INTDIR)\icolib.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF 

SOURCE=.\jabber.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yc"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber.obj"	"$(INTDIR)\jabber.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yc"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber.obj"	"$(INTDIR)\jabber.sbr"	"$(INTDIR)\jabber.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yc"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber.obj"	"$(INTDIR)\jabber.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yc"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber.obj"	"$(INTDIR)\jabber.sbr"	"$(INTDIR)\jabber.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yc"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber.obj"	"$(INTDIR)\jabber.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yc"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber.obj"	"$(INTDIR)\jabber.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_agent.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_agent.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_agent.obj"	"$(INTDIR)\jabber_agent.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_agent.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_agent.obj"	"$(INTDIR)\jabber_agent.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_agent.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_agent.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_bitmap.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"


"$(INTDIR)\jabber_bitmap.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"


"$(INTDIR)\jabber_bitmap.obj"	"$(INTDIR)\jabber_bitmap.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"


"$(INTDIR)\jabber_bitmap.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"


"$(INTDIR)\jabber_bitmap.obj"	"$(INTDIR)\jabber_bitmap.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"


"$(INTDIR)\jabber_bitmap.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"


"$(INTDIR)\jabber_bitmap.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF 

SOURCE=.\jabber_byte.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_byte.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_byte.obj"	"$(INTDIR)\jabber_byte.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_byte.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_byte.obj"	"$(INTDIR)\jabber_byte.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_byte.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_byte.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_chat.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"


"$(INTDIR)\jabber_chat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"


"$(INTDIR)\jabber_chat.obj"	"$(INTDIR)\jabber_chat.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"


"$(INTDIR)\jabber_chat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"


"$(INTDIR)\jabber_chat.obj"	"$(INTDIR)\jabber_chat.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"


"$(INTDIR)\jabber_chat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"


"$(INTDIR)\jabber_chat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF 

SOURCE=.\jabber_deprecated.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"


"$(INTDIR)\jabber_deprecated.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"


"$(INTDIR)\jabber_deprecated.obj"	"$(INTDIR)\jabber_deprecated.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"


"$(INTDIR)\jabber_deprecated.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"


"$(INTDIR)\jabber_deprecated.obj"	"$(INTDIR)\jabber_deprecated.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"


"$(INTDIR)\jabber_deprecated.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"


"$(INTDIR)\jabber_deprecated.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF 

SOURCE=.\jabber_file.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_file.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_file.obj"	"$(INTDIR)\jabber_file.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_file.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_file.obj"	"$(INTDIR)\jabber_file.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_file.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_file.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_form.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_form.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_form.obj"	"$(INTDIR)\jabber_form.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_form.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_form.obj"	"$(INTDIR)\jabber_form.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_form.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_form.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_ft.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_ft.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_ft.obj"	"$(INTDIR)\jabber_ft.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_ft.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_ft.obj"	"$(INTDIR)\jabber_ft.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_ft.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_ft.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_groupchat.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_groupchat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_groupchat.obj"	"$(INTDIR)\jabber_groupchat.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_groupchat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_groupchat.obj"	"$(INTDIR)\jabber_groupchat.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_groupchat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_groupchat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_icolib.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"


"$(INTDIR)\jabber_icolib.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"


"$(INTDIR)\jabber_icolib.obj"	"$(INTDIR)\jabber_icolib.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"


"$(INTDIR)\jabber_icolib.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"


"$(INTDIR)\jabber_icolib.obj"	"$(INTDIR)\jabber_icolib.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"


"$(INTDIR)\jabber_icolib.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"


"$(INTDIR)\jabber_icolib.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF 

SOURCE=.\jabber_iq.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_iq.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_iq.obj"	"$(INTDIR)\jabber_iq.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_iq.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_iq.obj"	"$(INTDIR)\jabber_iq.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_iq.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_iq.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_iqid.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_iqid.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_iqid.obj"	"$(INTDIR)\jabber_iqid.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_iqid.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_iqid.obj"	"$(INTDIR)\jabber_iqid.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_iqid.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_iqid.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_iqid_muc.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_iqid_muc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_iqid_muc.obj"	"$(INTDIR)\jabber_iqid_muc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_iqid_muc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_iqid_muc.obj"	"$(INTDIR)\jabber_iqid_muc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_iqid_muc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_iqid_muc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_libstr.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"


"$(INTDIR)\jabber_libstr.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"


"$(INTDIR)\jabber_libstr.obj"	"$(INTDIR)\jabber_libstr.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"


"$(INTDIR)\jabber_libstr.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"


"$(INTDIR)\jabber_libstr.obj"	"$(INTDIR)\jabber_libstr.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"


"$(INTDIR)\jabber_libstr.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"


"$(INTDIR)\jabber_libstr.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF 

SOURCE=.\jabber_list.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_list.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_list.obj"	"$(INTDIR)\jabber_list.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_list.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_list.obj"	"$(INTDIR)\jabber_list.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_list.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_list.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_menu.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_menu.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_menu.obj"	"$(INTDIR)\jabber_menu.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_menu.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_menu.obj"	"$(INTDIR)\jabber_menu.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_menu.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_menu.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_misc.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_misc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_misc.obj"	"$(INTDIR)\jabber_misc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_misc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_misc.obj"	"$(INTDIR)\jabber_misc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_misc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_misc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_opt.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_opt.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_opt.obj"	"$(INTDIR)\jabber_opt.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_opt.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_opt.obj"	"$(INTDIR)\jabber_opt.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_opt.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_opt.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_password.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_password.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_password.obj"	"$(INTDIR)\jabber_password.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_password.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_password.obj"	"$(INTDIR)\jabber_password.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_password.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_password.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_proxy.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_proxy.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_proxy.obj"	"$(INTDIR)\jabber_proxy.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_proxy.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_proxy.obj"	"$(INTDIR)\jabber_proxy.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_proxy.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_proxy.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_ssl.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_ssl.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_ssl.obj"	"$(INTDIR)\jabber_ssl.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_ssl.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_ssl.obj"	"$(INTDIR)\jabber_ssl.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_ssl.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_ssl.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_std.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"


"$(INTDIR)\jabber_std.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"


"$(INTDIR)\jabber_std.obj"	"$(INTDIR)\jabber_std.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"


"$(INTDIR)\jabber_std.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"


"$(INTDIR)\jabber_std.obj"	"$(INTDIR)\jabber_std.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"


"$(INTDIR)\jabber_std.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"


"$(INTDIR)\jabber_std.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF 

SOURCE=.\jabber_svc.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_svc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_svc.obj"	"$(INTDIR)\jabber_svc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_svc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_svc.obj"	"$(INTDIR)\jabber_svc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_svc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_svc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_thread.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_thread.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_thread.obj"	"$(INTDIR)\jabber_thread.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_thread.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_thread.obj"	"$(INTDIR)\jabber_thread.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_thread.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_thread.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_userinfo.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_userinfo.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_userinfo.obj"	"$(INTDIR)\jabber_userinfo.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_userinfo.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_userinfo.obj"	"$(INTDIR)\jabber_userinfo.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_userinfo.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_userinfo.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_util.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_util.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_util.obj"	"$(INTDIR)\jabber_util.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_util.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_util.obj"	"$(INTDIR)\jabber_util.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_util.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_util.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_vcard.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_vcard.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_vcard.obj"	"$(INTDIR)\jabber_vcard.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_vcard.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_vcard.obj"	"$(INTDIR)\jabber_vcard.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_vcard.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_vcard.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_ws.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_ws.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_ws.obj"	"$(INTDIR)\jabber_ws.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_ws.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_ws.obj"	"$(INTDIR)\jabber_ws.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_ws.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_ws.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_xml.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_xml.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_xml.obj"	"$(INTDIR)\jabber_xml.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_xml.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_xml.obj"	"$(INTDIR)\jabber_xml.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_xml.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_xml.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\jabber_xmlns.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_xmlns.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_xmlns.obj"	"$(INTDIR)\jabber_xmlns.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_xmlns.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\jabber_xmlns.obj"	"$(INTDIR)\jabber_xmlns.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_xmlns.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\jabber_xmlns.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\sha1.cpp

!IF  "$(CFG)" == "jgmail - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\sha1.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\sha1.obj"	"$(INTDIR)\sha1.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\sha1.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\sha1.obj"	"$(INTDIR)\sha1.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\sha1.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jgmail - Win32 Static"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /D "STATICSSL" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\sha1.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\msvc6.rc

"$(INTDIR)\msvc6.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

