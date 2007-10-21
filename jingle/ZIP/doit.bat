@echo off

rem Batch file to build and upload files
rem 
rem TODO: Integration with FL

set name=jingle

rem To upload, this var must be set here or in other batch
rem set ftp=ftp://<user>:<password>@<ftp>/<path>

echo Building %name% ...

"C:\Program Files\Microsoft Visual Studio 8\Common7\IDE\devenv.com" /Rebuild Release ..\%name%.sln
"C:\Program Files\Microsoft Visual Studio 8\Common7\IDE\devenv.com" /Rebuild "Unicode Release" ..\%name%.sln

echo Generating files for %name% ...

del *.zip
del *.dll
copy ..\..\..\bin\release\Plugins\%name%.dll
copy ..\..\..\bin\release\Plugins\%name%W.dll
copy ..\Docs\%name%_changelog.txt
copy ..\Docs\%name%_version.txt
copy ..\Docs\%name%_readme.txt
mkdir Docs
cd Docs
del /Q *.*
copy ..\..\Docs\%name%_readme.txt
cd ..

"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%.zip %name%.dll Docs
"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%W.zip %name%W.dll Docs

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
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_version.txt %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_readme.txt %ftp% -overwrite -close 

:END

echo Done.
