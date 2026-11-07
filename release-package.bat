@echo off
setlocal
rem The release's package, built on a PC with the game (a build needs its xbe): PGR2-package.zip and its description, for
rem the release beside PGR2Launcher.exe. Holds the runtime and the compiler, and no game code. Run build.bat first.
cd /d "%~dp0"
set "STAGE=out\package"
if exist "%STAGE%" rd /s /q "%STAGE%"
call "%~dp0prebuilt-test.bat" "%STAGE%" || exit /b 1
rd /s /q "%STAGE%\assets" 2>nul
del /q PGR2-package.zip PGR2-package.zip.json 2>nul
tar -a -c -f PGR2-package.zip -C "%STAGE%" . || (echo Making the zip failed. & exit /b 1)
powershell -NoProfile -Command "$f = Get-Item PGR2-package.zip; $h = (Get-FileHash $f -Algorithm SHA256).Hash.ToLower(); '{\"size\": ' + $f.Length + ', \"sha256\": \"' + $h + '\"}' | Set-Content -Encoding ascii PGR2-package.zip.json" || exit /b 1
for %%f in (PGR2-package.zip) do echo PGR2-package.zip: %%~zf bytes
echo To test: put it beside a launcher built by launcher.bat, in an empty folder.
echo To release: attach PGR2-package.zip and PGR2-package.zip.json to the tag's release.
endlocal
