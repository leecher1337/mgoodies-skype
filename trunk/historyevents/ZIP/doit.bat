@echo off

rem Batch file to build and upload files
rem 
rem TODO: Integration with FL

set name=historyevents

rem To upload, this var must be set here or in other batch
rem set ftp=ftp://<user>:<password>@<ftp>/<path>

echo Building %name% ...

msdev ..\%name%.dsp /MAKE "%name% - Win32 Release" /REBUILD
msdev ..\%name%.dsp /MAKE "%name% - Win32 Unicode Release" /REBUILD

echo Generating files for %name% ...

del *.zip
del *.dll
copy "..\..\..\bin\release\Plugins\aa_%name%.dll"
copy "..\..\..\bin\release unicode\Plugins\aa_%name%W.dll"
copy ..\Docs\%name%_changelog.txt
copy ..\Docs\%name%_version.txt
copy ..\Docs\%name%_readme.txt
mkdir Docs
cd Docs
del /Q *.*
rem copy ..\..\Docs\langpack_%name%.txt
copy ..\..\m_%name%.h
cd ..

"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%.zip aa_%name%.dll Docs Plugins
"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%W.zip aa_%name%W.dll Docs Plugins

del *.dll
cd Docs
del /Q *.*
cd ..
rmdir Docs

if "%ftp%"=="" GOTO END

echo Going to upload files...
pause

"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%.zip %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%W.zip %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_changelog.txt %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_readme.txt %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_version.txt %ftp% -overwrite -close 

:END

echo Done.
