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

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "imoskype - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "..\bin\imo2sproxy.exe"


CLEAN :
	-@erase "$(INTDIR)\buddylist.obj"
	-@erase "$(INTDIR)\buddylist.sbr"
	-@erase "$(INTDIR)\callqueue.obj"
	-@erase "$(INTDIR)\callqueue.sbr"
	-@erase "$(INTDIR)\cJSON.obj"
	-@erase "$(INTDIR)\cJSON.sbr"
	-@erase "$(INTDIR)\fifo.obj"
	-@erase "$(INTDIR)\fifo.sbr"
	-@erase "$(INTDIR)\imo2skypeapi.obj"
	-@erase "$(INTDIR)\imo2skypeapi.sbr"
	-@erase "$(INTDIR)\imo2sproxy.obj"
	-@erase "$(INTDIR)\imo2sproxy.sbr"
	-@erase "$(INTDIR)\imo_request.obj"
	-@erase "$(INTDIR)\imo_request.sbr"
	-@erase "$(INTDIR)\imo_skype.obj"
	-@erase "$(INTDIR)\imo_skype.sbr"
	-@erase "$(INTDIR)\io_layer_win32.obj"
	-@erase "$(INTDIR)\io_layer_win32.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\memlist.obj"
	-@erase "$(INTDIR)\memlist.sbr"
	-@erase "$(INTDIR)\msgqueue.obj"
	-@erase "$(INTDIR)\msgqueue.sbr"
	-@erase "$(INTDIR)\queue.obj"
	-@erase "$(INTDIR)\queue.sbr"
	-@erase "$(INTDIR)\socksproxy.obj"
	-@erase "$(INTDIR)\socksproxy.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\w32browser.obj"
	-@erase "$(INTDIR)\w32browser.sbr"
	-@erase "$(INTDIR)\w32skypeemu.obj"
	-@erase "$(INTDIR)\w32skypeemu.sbr"
	-@erase "$(OUTDIR)\imoskype.bsc"
	-@erase "..\bin\imo2sproxy.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "../src/common" /I "../src/imo2skype" /I "../src/imolib" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\imoskype.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\imoskype.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\buddylist.sbr" \
	"$(INTDIR)\callqueue.sbr" \
	"$(INTDIR)\cJSON.sbr" \
	"$(INTDIR)\fifo.sbr" \
	"$(INTDIR)\imo2skypeapi.sbr" \
	"$(INTDIR)\imo2sproxy.sbr" \
	"$(INTDIR)\imo_request.sbr" \
	"$(INTDIR)\imo_skype.sbr" \
	"$(INTDIR)\io_layer_win32.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\memlist.sbr" \
	"$(INTDIR)\msgqueue.sbr" \
	"$(INTDIR)\queue.sbr" \
	"$(INTDIR)\socksproxy.sbr" \
	"$(INTDIR)\w32browser.sbr" \
	"$(INTDIR)\w32skypeemu.sbr"

"$(OUTDIR)\imoskype.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib ws2_32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\imo2sproxy.pdb" /out:"../bin/imo2sproxy.exe" 
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
	"$(INTDIR)\socksproxy.obj" \
	"$(INTDIR)\w32browser.obj" \
	"$(INTDIR)\w32skypeemu.obj"

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
	-@erase "$(INTDIR)\socksproxy.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\w32browser.obj"
	-@erase "$(INTDIR)\w32skypeemu.obj"
	-@erase "$(OUTDIR)\imo2sproxy.exe"
	-@erase "$(OUTDIR)\imo2sproxy.ilk"
	-@erase "$(OUTDIR)\imo2sproxy.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "../src/common" /I "../src/imo2skype" /I "../src/imolib" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\imoskype.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\imoskype.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib ws2_32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\imo2sproxy.pdb" /debug /out:"$(OUTDIR)\imo2sproxy.exe" /pdbtype:sept 
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
	"$(INTDIR)\socksproxy.obj" \
	"$(INTDIR)\w32browser.obj" \
	"$(INTDIR)\w32skypeemu.obj"

"$(OUTDIR)\imo2sproxy.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("imoskype.dep")
!INCLUDE "imoskype.dep"
!ELSE 
!MESSAGE Warning: cannot find "imoskype.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "imoskype - Win32 Release" || "$(CFG)" == "imoskype - Win32 Debug"
SOURCE=..\src\imo2skype\buddylist.c

!IF  "$(CFG)" == "imoskype - Win32 Release"


"$(INTDIR)\buddylist.obj"	"$(INTDIR)\buddylist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "imoskype - Win32 Debug"


"$(INTDIR)\buddylist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\imo2skype\callqueue.c

!IF  "$(CFG)" == "imoskype - Win32 Release"


"$(INTDIR)\callqueue.obj"	"$(INTDIR)\callqueue.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "imoskype - Win32 Debug"


"$(INTDIR)\callqueue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\common\cJSON.c

!IF  "$(CFG)" == "imoskype - Win32 Release"


"$(INTDIR)\cJSON.obj"	"$(INTDIR)\cJSON.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "imoskype - Win32 Debug"


"$(INTDIR)\cJSON.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\common\fifo.c

!IF  "$(CFG)" == "imoskype - Win32 Release"


"$(INTDIR)\fifo.obj"	"$(INTDIR)\fifo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "imoskype - Win32 Debug"


"$(INTDIR)\fifo.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\imo2skype\imo2skypeapi.c

!IF  "$(CFG)" == "imoskype - Win32 Release"


"$(INTDIR)\imo2skypeapi.obj"	"$(INTDIR)\imo2skypeapi.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "imoskype - Win32 Debug"


"$(INTDIR)\imo2skypeapi.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\imo2skype\imo2sproxy.c

!IF  "$(CFG)" == "imoskype - Win32 Release"


"$(INTDIR)\imo2sproxy.obj"	"$(INTDIR)\imo2sproxy.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "imoskype - Win32 Debug"


"$(INTDIR)\imo2sproxy.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\imolib\imo_request.c

!IF  "$(CFG)" == "imoskype - Win32 Release"


"$(INTDIR)\imo_request.obj"	"$(INTDIR)\imo_request.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "imoskype - Win32 Debug"


"$(INTDIR)\imo_request.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\imolib\imo_skype.c

!IF  "$(CFG)" == "imoskype - Win32 Release"


"$(INTDIR)\imo_skype.obj"	"$(INTDIR)\imo_skype.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "imoskype - Win32 Debug"


"$(INTDIR)\imo_skype.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\imolib\io_layer_win32.c

!IF  "$(CFG)" == "imoskype - Win32 Release"


"$(INTDIR)\io_layer_win32.obj"	"$(INTDIR)\io_layer_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "imoskype - Win32 Debug"


"$(INTDIR)\io_layer_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\imo2skype\main.c

!IF  "$(CFG)" == "imoskype - Win32 Release"


"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "imoskype - Win32 Debug"


"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\common\memlist.c

!IF  "$(CFG)" == "imoskype - Win32 Release"


"$(INTDIR)\memlist.obj"	"$(INTDIR)\memlist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "imoskype - Win32 Debug"


"$(INTDIR)\memlist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\imo2skype\msgqueue.c

!IF  "$(CFG)" == "imoskype - Win32 Release"


"$(INTDIR)\msgqueue.obj"	"$(INTDIR)\msgqueue.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "imoskype - Win32 Debug"


"$(INTDIR)\msgqueue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\imo2skype\queue.c

!IF  "$(CFG)" == "imoskype - Win32 Release"


"$(INTDIR)\queue.obj"	"$(INTDIR)\queue.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "imoskype - Win32 Debug"


"$(INTDIR)\queue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\imo2skype\socksproxy.c

!IF  "$(CFG)" == "imoskype - Win32 Release"


"$(INTDIR)\socksproxy.obj"	"$(INTDIR)\socksproxy.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "imoskype - Win32 Debug"


"$(INTDIR)\socksproxy.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\imo2skype\w32browser.c

!IF  "$(CFG)" == "imoskype - Win32 Release"


"$(INTDIR)\w32browser.obj"	"$(INTDIR)\w32browser.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "imoskype - Win32 Debug"


"$(INTDIR)\w32browser.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\imo2skype\w32skypeemu.c

!IF  "$(CFG)" == "imoskype - Win32 Release"


"$(INTDIR)\w32skypeemu.obj"	"$(INTDIR)\w32skypeemu.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "imoskype - Win32 Debug"


"$(INTDIR)\w32skypeemu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

