@echo off
:: erzeuge ein Build zum ausliefern
::

:: Definitionen
SET PROJECTBASE=C:\DATEN\Entwicklung\QT-Projekte\spx42Control
SET PROJECTFILE=%PROJECTBASE%\spx42Control.pro
SET PROJECTBUILDDIR=DEPLOYDIR
SET PROJECTTYPE=release
SET TRANSLATION=%PROJECTBASE%\src\translations
SET INSTALLERBASE=%PROJECTBASE%\installer
SET APP_INSTALLER_FILE=spx42Control.7z
SET APP_INSTALLER_FILE_PATH=%INSTALLERBASE%\packages\app\data
SET RUNTIME_INSTALLER_FILE=qtRuntime_5120.7z
SET RUNTIME_INSTALLER_FILE_PATH=%INSTALLERBASE%\packages\qtRuntime\data
SET ONLINEINSTALLER=spx42ControlOnlineInstaller
SET OFFLINEINSTALLER=spx42ControlOfflineInstaller

SET QT_BASEDIR=C:\Qt\5.12.0\msvc2017_64\bin
SET QT_QMAKE=%QT_BASEDIR%\qmake.exe
SET QT_JOM=C:\Qt\Tools\QtCreator\bin\jom.exe
SET WINDEPLOY=C:\Qt\5.11.2\msvc2017_64\bin\windeployqt.exe
SET ARCHIVEGEN=C:\Qt\Tools\QtInstallerFramework\3.0\bin\archivegen.exe
SET BINARYCREATOR=C:\Qt\Tools\QtInstallerFramework\3.0\bin\binarycreator.exe
SET MAKE_DONE=false

echo.
echo.

pushd %cd%
cd ..
if not exist %PROJECTBUILDDIR% (
  mkdir %PROJECTBUILDDIR%
)
cd %PROJECTBUILDDIR%

echo directory %cd%
del /s /q /f *.*
echo del installer in %INSTALLERBASE%\%ONLINEINSTALLER%
if exist %INSTALLERBASE%\%ONLINEINSTALLER%.exe ( del /f /q %INSTALLERBASE%\%ONLINEINSTALLER%.exe )
if exist %INSTALLERBASE%\%OFFLINEINSTALLER%.exe ( del /f /q %INSTALLERBASE%\%OFFLINEINSTALLER%.exe )

echo Umgebung einrichten...
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"

echo.
echo qmake ausfuehren
%QT_QMAKE% %PROJECTFILE%  -Wall -nocache -spec win32-msvc "CONFIG += %PROJECTTYPE%"

echo build Projekt %PROJECTTYPE%
%QT_JOM% qmake_all first && SET MAKE_DONE=true

:: falls das falsch ist, zum Ende kommen
if "%MAKE_DONE%" == "false" goto false_end

echo Deployment... 
SET MAKE_DONE=false
%WINDEPLOY% --%PROJECTTYPE% --no-quick-import --no-system-d3d-compiler --no-opengl-sw "out" && SET MAKE_DONE=true

:: falls das falsch ist, zum Ende kommen
if "%MAKE_DONE%" == "false" goto false_end

:: Sprachdateien kopieren
echo kopiere translations (%TRANSLATION%\*.qm)  ...
if exist "%TRANSLATION%\*.qm" (
  copy /Y  "%TRANSLATION%\*.qm"  ".\out\"
)

cd out
echo erzeuge updater packages

if exist "%APP_INSTALLER_FILE_PATH%\%APP_INSTALLER_FILE%" (
  echo loesche alte datei "%APP_INSTALLER_FILE_PATH%\%APP_INSTALLER_FILE%"
  del /F /Q "%APP_INSTALLER_FILE_PATH%\%APP_INSTALLER_FILE%"
)

SET MAKE_DONE=false
:: BUGFIX von visual studio/qt deploy
cp -f %INSTALLERBASE%\ucrtbased.dll ucrtbased.dll
echo %ARCHIVEGEN% %APP_INSTALLER_FILE_PATH%\%APP_INSTALLER_FILE% *.exe *.qm *.ilk *.pdb ucrtbased.dll
%ARCHIVEGEN% %APP_INSTALLER_FILE_PATH%\%APP_INSTALLER_FILE% *.exe *.qm *.ilk *.pdb ucrtbased.dll && SET MAKE_DONE=true

:: falls das falsch ist, zum Ende kommen
if "%MAKE_DONE%" == "false" goto false_end

if exist spx42Control.* ( del /f /q  spx42Control.* )
if exist *.qm ( del /f /q  *.qm )
if exist ucrtbased.dll ( del /f /q ucrtbased.dll )
if exist %RUNTIME_INSTALLER_FILE_PATH%\%RUNTIME_INSTALLER_FILE% ( del /f /q %RUNTIME_INSTALLER_FILE_PATH%\%RUNTIME_INSTALLER_FILE% )

SET MAKE_DONE=false
echo erzeuge runtime package (%RUNTIME_INSTALLER_FILE_PATH%\%RUNTIME_INSTALLER_FILE%)...
%ARCHIVEGEN% %RUNTIME_INSTALLER_FILE_PATH%\%RUNTIME_INSTALLER_FILE% * && SET MAKE_DONE=true
del /f /q /s *.*

cd ..

:: falls das falsch ist, zum Ende kommen
if "%MAKE_DONE%" == "false" goto false_end

:: den installer erzeugen
echo erzeuge das installerprogramm
cd %INSTALLERBASE%\tool
echo verzeichnis %cd%

%QT_QMAKE% spx42ControlOfflineInstaller.pro -Wall -spec  win32-msvc "CONFIG -= release"
%QT_JOM% release && SET MAKE_DONE=true

%QT_QMAKE% spx42ControlOnlineInstaller.pro -Wall -spec  win32-msvc "CONFIG -= release"
%QT_JOM% release && SET MAKE_DONE=true

:false_end
echo fertig (%MAKE_DONE%)...
echo.

popd



