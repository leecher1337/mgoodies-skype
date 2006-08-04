@echo off

set name=smh

echo Building %name% ...

msdev ..\%name%.dsp /MAKE "%name% - Win32 Release" /REBUILD
msdev ..\%name%.dsp /MAKE "%name% - Win32 Unicode Release" /REBUILD

echo Generating files for %name% ...

del *.zip
del *.dll
copy ..\..\..\bin\release\Plugins\%name%.dll
copy ..\..\..\bin\release\Plugins\%name%W.dll
copy ..\Docs\%name%_changelog.txt
copy ..\Docs\%name%_version.txt
mkdir Docs
cd Docs
del /Q *.*
copy ..\..\Docs\langpack_%name%.txt
copy ..\..\m_%name%.h
cd ..
mkdir src
cd src
del /Q *.*
copy ..\..\*.h
copy ..\..\*.cpp
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

"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%.zip %name%.dll Docs
"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%W.zip %name%W.dll Docs
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

rem For this to work the ftp settings has to be configured
rem echo Going to upload files...
rem pause
rem 
rem "C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%.zip ftp://<user>:<password>@<ftp>/<path> -overwrite -close 
rem "C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%W.zip ftp://<user>:<password>@<ftp>/<path> -overwrite -close 
rem "C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_changelog.txt ftp://<user>:<password>@<ftp>/<path> -overwrite -close 
rem "C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_version.txt ftp://<user>:<password>@<ftp>/<path> -overwrite -close 

echo Done.
exit