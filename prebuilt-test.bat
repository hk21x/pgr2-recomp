@echo off
setlocal
rem Copies what a prebuilt release would hold out of build.bat's output into a new folder, to test the first start:
rem the exe, the runtime's DLLs and the compiler, without the game's code. Run build.bat first.
rem   prebuilt-test.bat C:\Games\PGR2Test
cd /d "%~dp0"
set "OUT=out\build\release"
set "DEST=%~1"
if "%DEST%"=="" echo Usage: prebuilt-test.bat ^<a new folder^> & exit /b 1
if exist "%DEST%\PGR2.exe" echo %DEST% already holds a test copy: delete it or name another folder. & exit /b 1
if not exist "%OUT%\PGR2.exe" echo Run build.bat first: %OUT%\PGR2.exe is missing. & exit /b 1
if not exist "%OUT%\compiler\clang.exe" echo %OUT%\compiler\clang.exe is missing: this is not the prebuilt branch's build. & exit /b 1
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -property installationPath`) do set "VSDIR=%%i"
for /d %%v in ("%VSDIR%\VC\Redist\MSVC\1*") do for /d %%c in ("%%v\x64\Microsoft.VC*.CRT") do set "VCRUNTIME=%%c"
if not exist "%VCRUNTIME%\vcruntime140.dll" echo The Visual C++ runtime was not found under %VSDIR%. & exit /b 1
mkdir "%DEST%\assets" 2>nul
copy /y "%OUT%\PGR2.exe" "%DEST%\" >nul || exit /b 1
robocopy "%OUT%\compiler" "%DEST%\compiler" /e /xd work /njh /njs /nfl /ndl >nul
if errorlevel 8 echo Copying the compiler failed. & exit /b 1
for %%f in (msvcp140 vcruntime140 vcruntime140_1) do (
    copy /y "%VCRUNTIME%\%%f.dll" "%DEST%\" >nul
    copy /y "%VCRUNTIME%\%%f.dll" "%DEST%\compiler\" >nul
)
set "D3DCOMPILER=%ProgramFiles(x86)%\Windows Kits\10\Redist\D3D\x64\d3dcompiler_47.dll"
if exist "%D3DCOMPILER%" copy /y "%D3DCOMPILER%" "%DEST%\" >nul
if exist "%DEST%\PGR2.dll" del /q "%DEST%\PGR2.dll"
echo Copied to %DEST%. Put your game in %DEST%\assets, then run %DEST%\PGR2.exe
endlocal
