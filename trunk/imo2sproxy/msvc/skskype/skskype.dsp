# Microsoft Developer Studio Project File - Name="skskype" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=skskype - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "skskype.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "skskype.mak" CFG="skskype - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "skskype - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "skskype - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "skskype - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\src\common" /I "..\..\src\imo2skype" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\api" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\ipc" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\types" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\platform\threading" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\platform\threading\win" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\VideoBuffers" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\platform\se" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\include" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\include\openssl" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "SKYPEKIT" /D "SSL_LIB_CYASSL" /D "NO_FILESYSTEM" /D "NO_RABBIT" /D "NO_HC128" /D "NO_DES" /D "NO_DSA" /D "HAVE_CONFIG_H" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "skskype - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\src\common" /I "..\..\src\imo2skype" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\api" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\ipc" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\types" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\platform\threading" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\platform\threading\win" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\VideoBuffers" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\platform\se" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\include" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\include\openssl" /I "F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "SKYPEKIT" /D "SSL_LIB_CYASSL" /D "NO_FILESYSTEM" /D "NO_RABBIT" /D "NO_HC128" /D "NO_DES" /D "NO_DSA" /D "HAVE_CONFIG_H" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "skskype - Win32 Release"
# Name "skskype - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Skypekit"

# PROP Default_Filter ""
# Begin Group "Types"

# PROP Default_Filter ""
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\types\skype-binary.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\types\skype-filename-list.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\types\skype-filename.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\types\skype-int-dict.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\types\skype-int-list.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\types\skype-obj-dict.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\types\skype-ptr-dict.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\types\skype-ptr-vector.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\types\skype-ptrint-dict.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\types\skype-ref-list.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\types\skype-string-dict.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\types\skype-string-list.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\types\skype-string.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\types\skype-uint64-list.cpp"
# End Source File
# End Group
# Begin Group "ipc"

# PROP Default_Filter ""
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\SidApi.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\SidClientConnection.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\SidClientSession.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\SidCommandProcessor.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\SidConcurrentCircularBuffer.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\SidConnection.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\SidField.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\SidListener.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\platform\se\SidPlatform.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\SidProtocolBinClient.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\SidProtocolBinCommon.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\SidProtocolFactory.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\SidServerConnectionWin.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\SidServerSession.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\SidSession.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\SidTLSEncryption.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\SidTransportLog.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\ipc\skype-clientsession.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\ipc\skype-object.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\SkypekitFrameTransport.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\SkypekitVideoTransportBase.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\SkypekitVideoTransportClient.cpp"
# End Source File
# End Group
# Begin Group "platform"

# PROP Default_Filter ""
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\platform\threading\win\skype-thread-win.cpp"
# End Source File
# End Group
# Begin Group "api"

# PROP Default_Filter ""
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\api\sidg_msgs_Skype.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\api\sidg_Skypeaction_call.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\api\sidg_Skypeevent_dispatch.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\api\sidg_Skypeproperty_get_call.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\api\sidg_Skyperefs.cpp"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\interfaces\skype\cpp_embedded\src\api\skype-embedded_2.cpp"
# End Source File
# End Group
# Begin Group "ssl"

# PROP Default_Filter ""
# Begin Group "taocrypt"

# PROP Default_Filter ""
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\_md4.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\_md5.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\_misc.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\_sha256.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\aes.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\arc4.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\asm.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\asn.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\coding.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\des3.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\dh.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\dsa.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\hc128.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\hmac.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\integer.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\pwdbased.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\rabbit.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\random.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\ripemd.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\rsa.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\sha.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\sha512.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\ctaocrypt\src\tfm.c"
# End Source File
# End Group
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\src\cyassl_int.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\src\cyassl_io.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\src\keys.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\src\sniffer.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\src\ssl.c"
# End Source File
# Begin Source File

SOURCE="F:\miranda09_src\miranda-private-keys\SkypeKit\SDK\ipc\cpp\ssl\cyassl\src\tls.c"
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=..\..\src\imo2skype\imo2sproxy.c
# End Source File
# Begin Source File

SOURCE=..\..\src\imo2skype\main.c
# End Source File
# Begin Source File

SOURCE=..\..\src\common\memlist.c
# End Source File
# Begin Source File

SOURCE=..\..\src\skypekit\skypekit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\imo2skype\socksproxy.c
# End Source File
# Begin Source File

SOURCE=..\..\src\imo2skype\w32skypeemu.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\src\imo2skype\imo2skypeapi.h
# End Source File
# Begin Source File

SOURCE=..\..\src\imo2skype\imo2sproxy.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\memlist.h
# End Source File
# Begin Source File

SOURCE=..\..\src\imo2skype\socksproxy.h
# End Source File
# Begin Source File

SOURCE=..\..\src\imo2skype\w32skypeemu.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
