@echo off

rem Batch file to build and upload files
rem 
rem TODO: Integration with FL

set name=bclist

rem To upload, this var must be set here or in other batch
rem set ftp=ftp://<user>:<password>@<ftp>/<path>

echo Building %name% ...

msdev ..\%name%.dsp /MAKE "%name% - Win32 Release" /REBUILD
msdev ..\%name%.dsp /MAKE "%name% - Win32 Release Unicode" /REBUILD

echo Generating files for %name% ...

del *.zip
del *.dll
copy ..\..\..\bin\release\Plugins\clist_blind.dll
copy ..\Docs\%name%_changelog.txt
copy ..\Docs\%name%_version.txt
copy ..\Docs\%name%_readme.txt
mkdir Docs
cd Docs
del /Q *.*
copy ..\..\Docs\%name%_readme.txt
rem copy ..\..\Docs\langpack_%name%.txt
rem copy ..\..\Docs\helppack_%name%.txt
rem copy ..\..\m_%name%.h
cd ..
mkdir src
cd src
del /Q *.*
copy ..\..\*.h
copy ..\..\*.c*
copy ..\..\*.
copy ..\..\*.rc
copy ..\..\*.dsp
copy ..\..\*.dsw
mkdir Docs
cd Docs
del /Q *.*
copy ..\..\..\Docs\*.*
cd ..
cd ..

"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%.zip clist_blind.dll Docs

del *.dll
copy "..\..\..\bin\release unicode\Plugins\clist_blind.dll"
"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%W.zip clist_blind.dll Docs

"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%_src.zip src\*.*

del *.dll
cd Docs
del /Q *.*
cd ..
rmdir Docs
cd src
del /Q *.*
cd Docs
del /Q *.*
cd ..
rmdir Docs
cd ..
rmdir src

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
