rem @echo off

rem Batch file to build and upload files
rem 
rem TODO: Integration with FL

set name=eSpeak

rem To upload, this var must be set here or in other batch
rem set ftp=ftp://<user>:<password>@<ftp>/<path>

echo Building %name% ...

msdev ..\%name%.dsp /MAKE "%name% - Win32 Release" /REBUILD
msdev ..\%name%.dsp /MAKE "%name% - Win32 Unicode Release" /REBUILD

echo Generating files for %name% ...

del *.zip
del *.dll
copy ..\Docs\%name%_changelog.txt
copy ..\Docs\%name%_version.txt
copy ..\Docs\%name%_readme.txt
mkdir Plugins
cd Plugins
copy ..\..\..\..\bin\release\Plugins\%name%.dll
copy ..\..\..\..\bin\release\Plugins\%name%W.dll
cd ..
mkdir Docs
cd Docs
del /Q *.*
copy ..\..\Docs\%name%_readme.txt
rem copy ..\..\Docs\langpack_%name%.txt
rem copy ..\..\Docs\helppack_%name%.txt
copy ..\..\m_speak.h
cd ..
mkdir Dictionaries
cd Dictionaries
mkdir Voice
cd Voice
xcopy ..\..\..\espeak-data\*.* /S
cd ..
cd ..
mkdir Icons
cd Icons
copy ..\..\..\spellchecker\Flags\flags.dll
cd ..

"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%.zip Plugins Docs Icons Dictionaries
"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%W.zip Plugins Docs Icons Dictionaries

del *.dll
cd Docs
del /Q *.*
cd ..
rmdir Docs
cd Icons
del /Q *.*
cd ..
rmdir Icons
cd Dictionaries
cd Voice
del /Q *.*
cd ..
rmdir Voice
del /Q *.*
cd ..
rmdir Dictionaries

if "%ftp%"=="" GOTO END

echo Going to upload files...
pause

"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%.zip %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%W.zip %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_changelog.txt %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_version.txt %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_readme.txt %ftp% -overwrite -close 

:END

echo Done.
