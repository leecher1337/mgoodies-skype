rem @echo off

rem Batch file to build and upload files
rem 
rem TODO: Integration with FL

set name=spellchecker

rem To upload, this var must be set here or in other batch
rem set ftp=ftp://<user>:<password>@<ftp>/<path>

echo Building %name% ...

msdev ..\%name%.dsp /MAKE "%name% - Win32 Release" /REBUILD
msdev ..\%name%.dsp /MAKE "%name% - Win32 Unicode Release" /REBUILD

echo Generating files for %name% ...

del *.zip
del *.dll
del *.pdb
rd /S /Q Plugins
rd /S /Q Docs
rd /S /Q src

copy ..\Release\%name%.pdb
copy ..\Unicode_Release\%name%W.pdb
copy ..\Docs\%name%_changelog.txt
copy ..\Docs\%name%_version.txt
copy ..\Docs\%name%_readme.txt
copy ..\srmm.spellchecker.patch
mkdir Docs
cd Docs
del /Q *.*
copy ..\..\Docs\%name%_readme.txt
copy ..\..\Docs\langpack_%name%.txt
rem copy ..\..\Docs\helppack_%name%.txt
copy ..\..\m_%name%.h
cd ..
mkdir src
cd src
mkdir %name%
cd %name%
del /Q *.*
copy ..\..\..\*.h
copy ..\..\..\*.cpp
copy ..\..\..\*.rc
copy ..\..\..\*.dsp
copy ..\..\..\*.dsw
mkdir res
cd res
del /Q *.*
copy ..\..\..\..\res\*.*
cd ..
mkdir sdk
cd sdk
del /Q *.*
copy ..\..\..\..\sdk\*.*
cd ..
mkdir hunspell
cd hunspell
del /Q *.*
copy ..\..\..\..\hunspell\*.*
cd ..
mkdir Docs
cd Docs
del /Q *.*
copy ..\..\..\..\Docs\*.*
cd ..
cd ..
mkdir utils
cd utils
del /Q *.*
copy ..\..\..\..\utils\*.*
cd ..
cd ..

mkdir Plugins
cd Plugins
del /Q *.dll
copy ..\..\..\..\bin\release\Plugins\%name%.dll
cd ..

"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%.zip Plugins Docs  
"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%.FL.zip Plugins Docs Dictionaries Icons

mkdir Plugins
cd Plugins
del /Q *.dll
copy ..\..\..\..\bin\release\Plugins\%name%W.dll
cd ..

"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%W.zip Plugins Docs
"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%W.FL.zip Plugins Docs Dictionaries Icons

"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%.pdb.zip %name%.pdb
"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%W.pdb.zip %name%W.pdb

"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%_src.zip src\*.*

del *.dll
del *.PDB

rd /S /Q Plugins
rd /S /Q Docs
rd /S /Q src

if "%ftp%"=="" GOTO END

echo Going to upload files...
pause

"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%.zip %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%W.zip %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%.pdb.zip %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%W.pdb.zip %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_changelog.txt %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_version.txt %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_readme.txt %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\srmm.spellchecker.patch %ftp% -overwrite -close 

:END

echo Done.
