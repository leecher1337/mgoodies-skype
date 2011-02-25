# Microsoft Developer Studio Generated NMAKE File, Based on imoskype.dsp
!IF "$(CFG)" == ""
CFG=imoskype - Win32 Debug
!MESSAGE Keine Konfiguration angegeben. imoskype - Win32 Debug wird als Standard verwendet.
!ENDIF 

!IF "$(CFG)" != "imoskype - Win32 Release" && "$(CFG)" != "imoskype - Win32 Debug"
!MESSAGE UngÅltige Konfiguration "$(CFG)" angegeben.
!MESSAGE Sie kînnen beim AusfÅhren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "imoskype.mak" CFG="imoskype - Win32 Debug"
!MESSAGE 
!MESSAGE FÅr die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "imoskype - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "imoskype - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE 
!ERROR Eine ungÅltige Konfiguration wurde angegeben.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "imoskype - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\bin\imo2sproxy.exe"


CLEAN :
	-@erase "$(INTDIR)\buddylist.obj"
	-@erase "$(INTDIR)\callqueue.obj"
	-@erase "$(INTDIR)\cJSON.obj"
	-@erase "$(INTDIR)\fifo.obj"
	-@erase "$(INTDIR)\imo2skypeapi.obj"
	-@erase "$(INTDIR)\imo2sproxy.obj"
	-@erase "$(INTDIR)\imo_request.obj"
	-@erase "$(INTDIR)\imo_skype.obj"
	-@erase "$(INTDIR)\io_layer_win32.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\memlist.obj"
	-@erase "$(INTDIR)\msgqueue.obj"
	-@erase "$(INTDIR)\queue.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\w32browser.obj"
	-@erase "..\bin\imo2sproxy.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=xicl6.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "../src/common" /I "../src/imo2skype" /I "../src/imolib" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\imoskype.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\imoskype.bsc" 
BSC32_SBRS= \
	
LINK32=xilink6.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\imo2sproxy.pdb" /machine:I386 /out:"../bin/imo2sproxy.exe" 
LINK32_OBJS= \
	"$(INTDIR)\buddylist.obj" \
	"$(INTDIR)\callqueue.obj" \
	"$(INTDIR)\cJSON.obj" \
	"$(INTDIR)\fifo.obj" \
	"$(INTDIR)\imo2skypeapi.obj" \
	"$(INTDIR)\imo2sproxy.obj" \
	"$(INTDIR)\imo_request.obj" \
	"$(INTDIR)\imo_skype.obj" \
	"$(INTDIR)\io_layer_win32.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\memlist.obj" \
	"$(INTDIR)\msgqueue.obj" \
	"$(INTDIR)\queue.obj" \
	"$(INTDIR)\w32browser.obj"

"..\bin\imo2sproxy.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "imoskype - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\imo2sproxy.exe"


CLEAN :
	-@erase "$(INTDIR)\buddylist.obj"
	-@erase "$(INTDIR)\callqueue.obj"
	-@erase "$(INTDIR)\cJSON.obj"
	-@erase "$(INTDIR)\fifo.obj"
	-@erase "$(INTDIR)\imo2skypeapi.obj"
	-@erase "$(INTDIR)\imo2sproxy.obj"
	-@erase "$(INTDIR)\imo_request.obj"
	-@erase "$(INTDIR)\imo_skype.obj"
	-@erase "$(INTDIR)\io_layer_win32.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\memlist.obj"
	-@erase "$(INTDIR)\msgqueue.obj"
	-@erase "$(INTDIR)\queue.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\w32browser.obj"
	-@erase "$(OUTDIR)\imo2sproxy.exe"
	-@erase "$(OUTDIR)\imo2sproxy.ilk"
	-@erase "$(OUTDIR)\imo2sproxy.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=xicl6.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "../src/common" /I "../src/imo2skype" /I "../src/imolib" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\imoskype.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\imoskype.bsc" 
BSC32_SBRS= \
	
LINK32=xilink6.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\imo2sproxy.pdb" /debug /machine:I386 /out:"$(OUTDIR)\imo2sproxy.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\buddylist.obj" \
	"$(INTDIR)\callqueue.obj" \
	"$(INTDIR)\cJSON.obj" \
	"$(INTDIR)\fifo.obj" \
	"$(INTDIR)\imo2skypeapi.obj" \
	"$(INTDIR)\imo2sproxy.obj" \
	"$(INTDIR)\imo_request.obj" \
	"$(INTDIR)\imo_skype.obj" \
	"$(INTDIR)\io_layer_win32.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\memlist.obj" \
	"$(INTDIR)\msgqueue.obj" \
	"$(INTDIR)\queue.obj" \
	"$(INTDIR)\w32browser.obj"

"$(OUTDIR)\imo2sproxy.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("imoskype.dep")
!INCLUDE "imoskype.dep"
!ELSE 
!MESSAGE Warning: cannot find "imoskype.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "imoskype - Win32 Release" || "$(CFG)" == "imoskype - Win32 Debug"
SOURCE=\temp\debug\imo.im\src\imo2skype\buddylist.c

"$(INTDIR)\buddylist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=\temp\debug\imo.im\src\imo2skype\callqueue.c

"$(INTDIR)\callqueue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=\temp\debug\imo.im\src\common\cJSON.c

"$(INTDIR)\cJSON.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=\temp\debug\imo.im\src\common\fifo.c

"$(INTDIR)\fifo.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=\temp\debug\imo.im\src\imo2skype\imo2skypeapi.c

"$(INTDIR)\imo2skypeapi.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=\temp\debug\imo.im\src\imo2skype\imo2sproxy.c

"$(INTDIR)\imo2sproxy.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=\temp\debug\imo.im\src\imolib\imo_request.c

"$(INTDIR)\imo_request.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=\temp\debug\imo.im\src\imolib\imo_skype.c

"$(INTDIR)\imo_skype.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=\temp\debug\imo.im\src\imolib\io_layer_win32.c

"$(INTDIR)\io_layer_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\imo2skype\main.c

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=\temp\debug\imo.im\src\common\memlist.c

"$(INTDIR)\memlist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=\temp\debug\imo.im\src\imo2skype\msgqueue.c

"$(INTDIR)\msgqueue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=\temp\debug\imo.im\src\imo2skype\queue.c

"$(INTDIR)\queue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=\temp\debug\imo.im\src\imo2skype\w32browser.c

"$(INTDIR)\w32browser.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

