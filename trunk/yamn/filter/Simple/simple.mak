# Microsoft Developer Studio Generated NMAKE File, Based on simple.dsp
!IF "$(CFG)" == ""
CFG=simple - Win32 Debug
!MESSAGE No configuration specified. Defaulting to simple - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "simple - Win32 Release" && "$(CFG)" != "simple - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "simple.mak" CFG="simple - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "simple - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "simple - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "simple - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\..\..\bin\release\plugins\YAMN\simple.dll"


CLEAN :
	-@erase "$(INTDIR)\maindll.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\simple.exp"
	-@erase "$(OUTDIR)\simple.lib"
	-@erase "..\..\..\..\bin\release\plugins\YAMN\simple.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "../../../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\simple.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\simple.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\simple.pdb" /machine:I386 /out:"../../../../bin/release/plugins//YAMN/simple.dll" /implib:"$(OUTDIR)\simple.lib" 
LINK32_OBJS= \
	"$(INTDIR)\maindll.obj"

"..\..\..\..\bin\release\plugins\YAMN\simple.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "simple - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\simple.dll"


CLEAN :
	-@erase "$(INTDIR)\maindll.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\simple.dll"
	-@erase "$(OUTDIR)\simple.exp"
	-@erase "$(OUTDIR)\simple.ilk"
	-@erase "$(OUTDIR)\simple.lib"
	-@erase "$(OUTDIR)\simple.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Gm /GX /ZI /Od /I "../../../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\simple.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\simple.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\simple.pdb" /debug /machine:I386 /out:"$(OUTDIR)\simple.dll" /implib:"$(OUTDIR)\simple.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\maindll.obj"

"$(OUTDIR)\simple.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("simple.dep")
!INCLUDE "simple.dep"
!ELSE 
!MESSAGE Warning: cannot find "simple.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "simple - Win32 Release" || "$(CFG)" == "simple - Win32 Debug"
SOURCE=.\maindll.cpp

"$(INTDIR)\maindll.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

