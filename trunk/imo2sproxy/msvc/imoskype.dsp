# Microsoft Developer Studio Project File - Name="imoskype" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=imoskype - Win32 Debug
!MESSAGE Dies ist kein g�ltiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und f�hren Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "imoskype.mak".
!MESSAGE 
!MESSAGE Sie k�nnen beim Ausf�hren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "imoskype.mak" CFG="imoskype - Win32 Debug"
!MESSAGE 
!MESSAGE F�r die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "imoskype - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "imoskype - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "imoskype - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../src/common" /I "../src/imo2skype" /I "../src/imolib" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /c
# ADD BASE RSC /l 0xc07 /d "NDEBUG"
# ADD RSC /l 0xc07 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"../bin/imo2sproxy.exe"

!ELSEIF  "$(CFG)" == "imoskype - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../src/common" /I "../src/imo2skype" /I "../src/imolib" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0xc07 /d "_DEBUG"
# ADD RSC /l 0xc07 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/imo2sproxy.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "imoskype - Win32 Release"
# Name "imoskype - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\imo2skype\buddylist.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\callqueue.c
# End Source File
# Begin Source File

SOURCE=..\src\common\cJSON.c
# End Source File
# Begin Source File

SOURCE=..\src\common\fifo.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\imo2skypeapi.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\imo2sproxy.c
# End Source File
# Begin Source File

SOURCE=..\src\imolib\imo_request.c
# End Source File
# Begin Source File

SOURCE=..\src\imolib\imo_skype.c
# End Source File
# Begin Source File

SOURCE=..\src\imolib\io_layer_win32.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\main.c
# End Source File
# Begin Source File

SOURCE=..\src\common\memlist.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\msgqueue.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\queue.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\socksproxy.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\w32browser.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\w32skypeemu.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\imo2skype\buddylist.h
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\callqueue.h
# End Source File
# Begin Source File

SOURCE=..\src\common\cJSON.h
# End Source File
# Begin Source File

SOURCE=..\src\common\fifo.h
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\imo2skypeapi.h
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\imo2sproxy.h
# End Source File
# Begin Source File

SOURCE=..\src\imolib\imo_request.h
# End Source File
# Begin Source File

SOURCE=..\src\imolib\imo_skype.h
# End Source File
# Begin Source File

SOURCE=..\src\imolib\io_layer.h
# End Source File
# Begin Source File

SOURCE=..\src\common\memlist.h
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\msgqueue.h
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\queue.h
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\socksproxy.h
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\w32browser.h
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\w32skypeemu.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
