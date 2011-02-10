# Microsoft Developer Studio Generated NMAKE File, Based on listeningto.dsp
!IF "$(CFG)" == ""
CFG=listeningto - Win32 Release
!MESSAGE Keine Konfiguration angegeben. listeningto - Win32 Release wird als Standard verwendet.
!ENDIF 

!IF "$(CFG)" != "listeningto - Win32 Release" && "$(CFG)" != "listeningto - Win32 Debug" && "$(CFG)" != "listeningto - Win32 Release Unicode" && "$(CFG)" != "listeningto - Win32 Debug Unicode"
!MESSAGE UngÅltige Konfiguration "$(CFG)" angegeben.
!MESSAGE Sie kînnen beim AusfÅhren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "listeningto.mak" CFG="listeningto - Win32 Release"
!MESSAGE 
!MESSAGE FÅr die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "listeningto - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "listeningto - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "listeningto - Win32 Release Unicode" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "listeningto - Win32 Debug Unicode" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR Eine ungÅltige Konfiguration wurde angegeben.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "listeningto - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "..\..\bin\release\Plugins\listeningto.dll" "$(OUTDIR)\listeningto.bsc"


CLEAN :
	-@erase "$(INTDIR)\foobar.obj"
	-@erase "$(INTDIR)\foobar.sbr"
	-@erase "$(INTDIR)\generic.obj"
	-@erase "$(INTDIR)\generic.sbr"
	-@erase "$(INTDIR)\itunes.obj"
	-@erase "$(INTDIR)\itunes.sbr"
	-@erase "$(INTDIR)\listeningto.obj"
	-@erase "$(INTDIR)\listeningto.sbr"
	-@erase "$(INTDIR)\mir_icons.obj"
	-@erase "$(INTDIR)\mir_icons.sbr"
	-@erase "$(INTDIR)\mir_options.obj"
	-@erase "$(INTDIR)\mir_options.sbr"
	-@erase "$(INTDIR)\music.obj"
	-@erase "$(INTDIR)\music.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\player.obj"
	-@erase "$(INTDIR)\player.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\watrack.obj"
	-@erase "$(INTDIR)\watrack.sbr"
	-@erase "$(INTDIR)\winamp.obj"
	-@erase "$(INTDIR)\winamp.sbr"
	-@erase "$(INTDIR)\wmp.obj"
	-@erase "$(INTDIR)\wmp.sbr"
	-@erase "$(OUTDIR)\listeningto.bsc"
	-@erase "$(OUTDIR)\listeningto.exp"
	-@erase "$(OUTDIR)\listeningto.lib"
	-@erase "$(OUTDIR)\listeningto.map"
	-@erase "$(OUTDIR)\listeningto.pdb"
	-@erase "..\..\bin\release\Plugins\listeningto.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G4 /MT /W3 /GX /Zi /O2 /Ob0 /I "..\..\include" /I "..\..\include_API" /I "sdk" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\listeningto.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\listeningto.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\listeningto.sbr" \
	"$(INTDIR)\mir_icons.sbr" \
	"$(INTDIR)\mir_options.sbr" \
	"$(INTDIR)\music.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\foobar.sbr" \
	"$(INTDIR)\generic.sbr" \
	"$(INTDIR)\itunes.sbr" \
	"$(INTDIR)\player.sbr" \
	"$(INTDIR)\watrack.sbr" \
	"$(INTDIR)\winamp.sbr" \
	"$(INTDIR)\wmp.sbr"

"$(OUTDIR)\listeningto.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib ole32.lib oleaut32.lib /nologo /base:"0x3EC20000" /dll /incremental:no /pdb:"$(OUTDIR)\listeningto.pdb" /map:"$(INTDIR)\listeningto.map" /debug /machine:I386 /out:"..\..\bin\release\Plugins\listeningto.dll" /implib:"$(OUTDIR)\listeningto.lib" /filealign:0x200 /ALIGN:4096 /ignore:4108 
LINK32_OBJS= \
	"$(INTDIR)\listeningto.obj" \
	"$(INTDIR)\mir_icons.obj" \
	"$(INTDIR)\mir_options.obj" \
	"$(INTDIR)\music.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\foobar.obj" \
	"$(INTDIR)\generic.obj" \
	"$(INTDIR)\itunes.obj" \
	"$(INTDIR)\player.obj" \
	"$(INTDIR)\watrack.obj" \
	"$(INTDIR)\winamp.obj" \
	"$(INTDIR)\wmp.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\release\Plugins\listeningto.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "listeningto - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "..\..\bin\debug\Plugins\listeningto.dll" "$(OUTDIR)\listeningto.bsc"


CLEAN :
	-@erase "$(INTDIR)\foobar.obj"
	-@erase "$(INTDIR)\foobar.sbr"
	-@erase "$(INTDIR)\generic.obj"
	-@erase "$(INTDIR)\generic.sbr"
	-@erase "$(INTDIR)\itunes.obj"
	-@erase "$(INTDIR)\itunes.sbr"
	-@erase "$(INTDIR)\listeningto.obj"
	-@erase "$(INTDIR)\listeningto.sbr"
	-@erase "$(INTDIR)\mir_icons.obj"
	-@erase "$(INTDIR)\mir_icons.sbr"
	-@erase "$(INTDIR)\mir_options.obj"
	-@erase "$(INTDIR)\mir_options.sbr"
	-@erase "$(INTDIR)\music.obj"
	-@erase "$(INTDIR)\music.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\player.obj"
	-@erase "$(INTDIR)\player.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\watrack.obj"
	-@erase "$(INTDIR)\watrack.sbr"
	-@erase "$(INTDIR)\winamp.obj"
	-@erase "$(INTDIR)\winamp.sbr"
	-@erase "$(INTDIR)\wmp.obj"
	-@erase "$(INTDIR)\wmp.sbr"
	-@erase "$(OUTDIR)\listeningto.bsc"
	-@erase "$(OUTDIR)\listeningto.exp"
	-@erase "$(OUTDIR)\listeningto.lib"
	-@erase "$(OUTDIR)\listeningto.pdb"
	-@erase "..\..\bin\debug\Plugins\listeningto.dll"
	-@erase "..\..\bin\debug\Plugins\listeningto.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G4 /MTd /W3 /GX /ZI /Od /I "..\..\include" /I "..\..\include_API" /I "sdk" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\listeningto.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\listeningto.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\listeningto.sbr" \
	"$(INTDIR)\mir_icons.sbr" \
	"$(INTDIR)\mir_options.sbr" \
	"$(INTDIR)\music.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\foobar.sbr" \
	"$(INTDIR)\generic.sbr" \
	"$(INTDIR)\itunes.sbr" \
	"$(INTDIR)\player.sbr" \
	"$(INTDIR)\watrack.sbr" \
	"$(INTDIR)\winamp.sbr" \
	"$(INTDIR)\wmp.sbr"

"$(OUTDIR)\listeningto.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib ole32.lib oleaut32.lib /nologo /base:"0x3EC20000" /dll /incremental:yes /pdb:"$(OUTDIR)\listeningto.pdb" /debug /machine:I386 /out:"..\..\bin\debug\Plugins\listeningto.dll" /implib:"$(OUTDIR)\listeningto.lib" /pdbtype:sept /filealign:0x200 /ALIGN:4096 /ignore:4108 
LINK32_OBJS= \
	"$(INTDIR)\listeningto.obj" \
	"$(INTDIR)\mir_icons.obj" \
	"$(INTDIR)\mir_options.obj" \
	"$(INTDIR)\music.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\foobar.obj" \
	"$(INTDIR)\generic.obj" \
	"$(INTDIR)\itunes.obj" \
	"$(INTDIR)\player.obj" \
	"$(INTDIR)\watrack.obj" \
	"$(INTDIR)\winamp.obj" \
	"$(INTDIR)\wmp.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\debug\Plugins\listeningto.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "listeningto - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode
# Begin Custom Macros
OutDir=.\Release_Unicode
# End Custom Macros

ALL : "..\..\bin\release unicode\Plugins\listeningtoW.dll" "$(OUTDIR)\listeningto.bsc"


CLEAN :
	-@erase "$(INTDIR)\foobar.obj"
	-@erase "$(INTDIR)\foobar.sbr"
	-@erase "$(INTDIR)\generic.obj"
	-@erase "$(INTDIR)\generic.sbr"
	-@erase "$(INTDIR)\itunes.obj"
	-@erase "$(INTDIR)\itunes.sbr"
	-@erase "$(INTDIR)\listeningto.obj"
	-@erase "$(INTDIR)\listeningto.sbr"
	-@erase "$(INTDIR)\mir_icons.obj"
	-@erase "$(INTDIR)\mir_icons.sbr"
	-@erase "$(INTDIR)\mir_options.obj"
	-@erase "$(INTDIR)\mir_options.sbr"
	-@erase "$(INTDIR)\music.obj"
	-@erase "$(INTDIR)\music.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\player.obj"
	-@erase "$(INTDIR)\player.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\watrack.obj"
	-@erase "$(INTDIR)\watrack.sbr"
	-@erase "$(INTDIR)\winamp.obj"
	-@erase "$(INTDIR)\winamp.sbr"
	-@erase "$(INTDIR)\wmp.obj"
	-@erase "$(INTDIR)\wmp.sbr"
	-@erase "$(OUTDIR)\listeningto.bsc"
	-@erase "$(OUTDIR)\listeningtoW.exp"
	-@erase "$(OUTDIR)\listeningtoW.lib"
	-@erase "$(OUTDIR)\listeningtoW.map"
	-@erase "$(OUTDIR)\listeningtoW.pdb"
	-@erase "..\..\bin\release unicode\Plugins\listeningtoW.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G4 /MT /W3 /GX /Zi /O2 /Ob0 /I "..\..\include" /I "..\..\include_API" /I "sdk" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\listeningto.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\listeningto.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\listeningto.sbr" \
	"$(INTDIR)\mir_icons.sbr" \
	"$(INTDIR)\mir_options.sbr" \
	"$(INTDIR)\music.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\foobar.sbr" \
	"$(INTDIR)\generic.sbr" \
	"$(INTDIR)\itunes.sbr" \
	"$(INTDIR)\player.sbr" \
	"$(INTDIR)\watrack.sbr" \
	"$(INTDIR)\winamp.sbr" \
	"$(INTDIR)\wmp.sbr"

"$(OUTDIR)\listeningto.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib ole32.lib oleaut32.lib /nologo /base:"0x3EC20000" /dll /incremental:no /pdb:"$(OUTDIR)\listeningtoW.pdb" /map:"$(INTDIR)\listeningtoW.map" /debug /machine:I386 /out:"..\..\bin\release unicode\Plugins\listeningtoW.dll" /implib:"$(OUTDIR)\listeningtoW.lib" /filealign:0x200 /ALIGN:4096 /ignore:4108 
LINK32_OBJS= \
	"$(INTDIR)\resource.res" \
	"$(INTDIR)\listeningto.obj" \
	"$(INTDIR)\mir_icons.obj" \
	"$(INTDIR)\mir_options.obj" \
	"$(INTDIR)\music.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\foobar.obj" \
	"$(INTDIR)\generic.obj" \
	"$(INTDIR)\itunes.obj" \
	"$(INTDIR)\player.obj" \
	"$(INTDIR)\watrack.obj" \
	"$(INTDIR)\winamp.obj" \
	"$(INTDIR)\wmp.obj"

"..\..\bin\release unicode\Plugins\listeningtoW.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "listeningto - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode
# Begin Custom Macros
OutDir=.\Debug_Unicode
# End Custom Macros

ALL : "..\..\bin\debug unicode\Plugins\listeningtoW.dll" "$(OUTDIR)\listeningto.bsc"


CLEAN :
	-@erase "$(INTDIR)\foobar.obj"
	-@erase "$(INTDIR)\foobar.sbr"
	-@erase "$(INTDIR)\generic.obj"
	-@erase "$(INTDIR)\generic.sbr"
	-@erase "$(INTDIR)\itunes.obj"
	-@erase "$(INTDIR)\itunes.sbr"
	-@erase "$(INTDIR)\listeningto.obj"
	-@erase "$(INTDIR)\listeningto.sbr"
	-@erase "$(INTDIR)\mir_icons.obj"
	-@erase "$(INTDIR)\mir_icons.sbr"
	-@erase "$(INTDIR)\mir_options.obj"
	-@erase "$(INTDIR)\mir_options.sbr"
	-@erase "$(INTDIR)\music.obj"
	-@erase "$(INTDIR)\music.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\player.obj"
	-@erase "$(INTDIR)\player.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\watrack.obj"
	-@erase "$(INTDIR)\watrack.sbr"
	-@erase "$(INTDIR)\winamp.obj"
	-@erase "$(INTDIR)\winamp.sbr"
	-@erase "$(INTDIR)\wmp.obj"
	-@erase "$(INTDIR)\wmp.sbr"
	-@erase "$(OUTDIR)\listeningto.bsc"
	-@erase "$(OUTDIR)\listeningtoW.exp"
	-@erase "$(OUTDIR)\listeningtoW.lib"
	-@erase "$(OUTDIR)\listeningtoW.pdb"
	-@erase "..\..\bin\debug unicode\Plugins\listeningtoW.dll"
	-@erase "..\..\bin\debug unicode\Plugins\listeningtoW.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G4 /MTd /W3 /GX /ZI /Od /I "..\..\include" /I "..\..\include_API" /I "sdk" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\listeningto.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\listeningto.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\listeningto.sbr" \
	"$(INTDIR)\mir_icons.sbr" \
	"$(INTDIR)\mir_options.sbr" \
	"$(INTDIR)\music.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\foobar.sbr" \
	"$(INTDIR)\generic.sbr" \
	"$(INTDIR)\itunes.sbr" \
	"$(INTDIR)\player.sbr" \
	"$(INTDIR)\watrack.sbr" \
	"$(INTDIR)\winamp.sbr" \
	"$(INTDIR)\wmp.sbr"

"$(OUTDIR)\listeningto.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib ole32.lib oleaut32.lib /nologo /base:"0x3EC20000" /dll /incremental:yes /pdb:"$(OUTDIR)\listeningtoW.pdb" /debug /machine:I386 /out:"..\..\bin\debug unicode\Plugins\listeningtoW.dll" /implib:"$(OUTDIR)\listeningtoW.lib" /pdbtype:sept /filealign:0x200 /ALIGN:4096 /ignore:4108 
LINK32_OBJS= \
	"$(INTDIR)\resource.res" \
	"$(INTDIR)\listeningto.obj" \
	"$(INTDIR)\mir_icons.obj" \
	"$(INTDIR)\mir_options.obj" \
	"$(INTDIR)\music.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\foobar.obj" \
	"$(INTDIR)\generic.obj" \
	"$(INTDIR)\itunes.obj" \
	"$(INTDIR)\player.obj" \
	"$(INTDIR)\watrack.obj" \
	"$(INTDIR)\winamp.obj" \
	"$(INTDIR)\wmp.obj"

"..\..\bin\debug unicode\Plugins\listeningtoW.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x417 /fo"$(INTDIR)\resource.res" /d "NDEBUG" 

!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("listeningto.dep")
!INCLUDE "listeningto.dep"
!ELSE 
!MESSAGE Warning: cannot find "listeningto.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "listeningto - Win32 Release" || "$(CFG)" == "listeningto - Win32 Debug" || "$(CFG)" == "listeningto - Win32 Release Unicode" || "$(CFG)" == "listeningto - Win32 Debug Unicode"
SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\listeningto.cpp

"$(INTDIR)\listeningto.obj"	"$(INTDIR)\listeningto.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\utils\mir_icons.cpp

"$(INTDIR)\mir_icons.obj"	"$(INTDIR)\mir_icons.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\utils\mir_options.cpp

"$(INTDIR)\mir_options.obj"	"$(INTDIR)\mir_options.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\music.cpp

"$(INTDIR)\music.obj"	"$(INTDIR)\music.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\options.cpp

"$(INTDIR)\options.obj"	"$(INTDIR)\options.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\players\foobar.cpp

"$(INTDIR)\foobar.obj"	"$(INTDIR)\foobar.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\players\generic.cpp

"$(INTDIR)\generic.obj"	"$(INTDIR)\generic.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\players\itunes.cpp

"$(INTDIR)\itunes.obj"	"$(INTDIR)\itunes.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\players\player.cpp

"$(INTDIR)\player.obj"	"$(INTDIR)\player.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\players\watrack.cpp

"$(INTDIR)\watrack.obj"	"$(INTDIR)\watrack.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\players\winamp.cpp

"$(INTDIR)\winamp.obj"	"$(INTDIR)\winamp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\players\wmp.cpp

"$(INTDIR)\wmp.obj"	"$(INTDIR)\wmp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

