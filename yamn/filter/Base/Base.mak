# Microsoft Developer Studio Generated NMAKE File, Based on Base.dsp
!IF "$(CFG)" == ""
CFG=Base - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Base - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Base - Win32 Release" && "$(CFG)" != "Base - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Base.mak" CFG="Base - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Base - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Base - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "Base - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\..\..\bin\release\plugins\YAMN-filter\Base.dll"


CLEAN :
	-@erase "$(INTDIR)\debug.obj"
	-@erase "$(INTDIR)\maindll.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\Base.exp"
	-@erase "$(OUTDIR)\Base.lib"
	-@erase "..\..\..\..\bin\release\plugins\YAMN-filter\Base.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "../../../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\Base.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Base.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\Base.pdb" /machine:I386 /out:"../../../../bin/release/plugins/YAMN-filter/Base.dll" /implib:"$(OUTDIR)\Base.lib" 
LINK32_OBJS= \
	"$(INTDIR)\debug.obj" \
	"$(INTDIR)\maindll.obj"

"..\..\..\..\bin\release\plugins\YAMN-filter\Base.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Base - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\Base.dll"


CLEAN :
	-@erase "$(INTDIR)\debug.obj"
	-@erase "$(INTDIR)\maindll.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\Base.dll"
	-@erase "$(OUTDIR)\Base.exp"
	-@erase "$(OUTDIR)\Base.ilk"
	-@erase "$(OUTDIR)\Base.lib"
	-@erase "$(OUTDIR)\Base.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Gm /GX /ZI /Od /I "../../../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DEBUG_FILTER" /Fp"$(INTDIR)\Base.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Base.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\Base.pdb" /debug /machine:I386 /out:"$(OUTDIR)\Base.dll" /implib:"$(OUTDIR)\Base.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\debug.obj" \
	"$(INTDIR)\maindll.obj"

"$(OUTDIR)\Base.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("Base.dep")
!INCLUDE "Base.dep"
!ELSE 
!MESSAGE Warning: cannot find "Base.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Base - Win32 Release" || "$(CFG)" == "Base - Win32 Debug"
SOURCE=.\debug.cpp

"$(INTDIR)\debug.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\maindll.cpp

"$(INTDIR)\maindll.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

