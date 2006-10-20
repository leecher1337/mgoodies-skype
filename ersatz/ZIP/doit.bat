rem @echo off

rem Batch file to build and upload files
rem 
rem TODO: Integration with FL

set name=ersatz

rem To upload, this var must be set here or in other batch
rem set ftp=ftp://<user>:<password>@<ftp>/<path>

echo Building %name% ...

msdev ..\%name%.dsp /MAKE "%name% - Win32 Release" /REBUILD

echo Generating files for %name% ...

del *.zip
del *.dll
copy ..\..\..\bin\release\Plugins\aaa%name%.dll
copy ..\Docs\%name%_changelog.txt
copy ..\Docs\%name%_version.txt
mkdir Docs
cd Docs
del /Q *.*
copy ..\..\m_%name%.h
cd ..
mkdir src
cd src
mkdir %name%
cd %name%
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

"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%.zip aaa%name%.dll Docs
"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%_src.zip src\%name% src\utils

del *.dll
cd Docs
del /Q *.*
cd ..
rmdir Docs
cd src
cd %name%
del /Q *.*
cd Docs
del /Q *.*
cd ..
rmdir Docs
cd sdk
del /Q *.*
cd ..
rmdir sdk
cd ..
rmdir %name%
cd utils
del /Q *.*
cd ..
rmdir utils
cd ..
rmdir src

if "%ftp%"=="" GOTO END

echo Going to upload files...
pause

"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%.zip %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_changelog.txt %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_version.txt %ftp% -overwrite -close 

:END

echo Done.
