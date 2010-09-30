rem @echo off

rem Batch file to build and upload files
rem 
rem TODO: Integration with FL

set name=mydetails

rem To upload, this var must be set here or in other batch
rem set ftp=ftp://<user>:<password>@<ftp>/<path>

echo Building %name% ...

msdev ..\%name%.dsp /MAKE "%name% - Win32 Release" /REBUILD

echo Generating files for %name% ...

del *.zip
del *.dll
del *.pdb

copy ..\Docs\%name%_changelog.txt
copy ..\Docs\%name%_version.txt
mkdir Plugins
cd Plugins
copy ..\..\..\..\bin\release\Plugins\%name%.dll
copy ..\..\..\..\bin\release\Plugins\skins.dll
cd ..
mkdir Skins
cd Skins
mkdir Default
cd Default
copy ..\..\..\data\Skins\Default\*.msk
cd..
cd..
mkdir Docs
cd Docs
del /Q *.*
copy ..\..\Docs\langpack_%name%.txt
copy ..\..\m_%name%.h
cd ..
mkdir src
cd src
mkdir mydetails
cd mydetails
del /Q *.*
copy ..\..\..\*.h
copy ..\..\..\*.cpp
copy ..\..\..\*.
copy ..\..\..\*.rc
copy ..\..\..\*.dsp
copy ..\..\..\*.dsw
mkdir Docs
cd Docs
del /Q *.*
copy ..\..\..\..\Docs\*.*
cd ..
mkdir sdk
cd sdk
del /Q *.*
copy ..\..\..\..\sdk\*.*
cd ..
cd ..
mkdir utils
cd utils
copy ..\..\..\..\utils\*.h
copy ..\..\..\..\utils\*.cpp
cd ..
cd ..
copy ..\Release\%name%.pdb

"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%.zip Plugins Docs Skins
"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%_src.zip src\mydetails src\utils
"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%.pdb.zip %name%.pdb

del *.pdb
rd /S /Q Plugins
rd /S /Q Docs
rd /S /Q src

if "%ftp%"=="" GOTO END

echo Going to upload files...
pause

"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%.zip %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%.pdb.zip %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_changelog.txt %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_version.txt %ftp% -overwrite -close 

if "%ftp2%"=="" GOTO END

"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%.zip %ftp2% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%.pdb.zip %ftp2% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_changelog.txt %ftp2% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_version.txt %ftp2% -overwrite -close 

:END

echo Done.
