@echo off
setlocal

set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Release

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" > nul 2>&1

msbuild d:\ThrustC\build\thrust.sln /p:Configuration=%CONFIG% /p:Platform=x64 /v:minimal /nologo

echo.
if %ERRORLEVEL%==0 (echo Build succeeded: %CONFIG% -- d:\ThrustC\Release\thrust.exe) else (echo Build FAILED)
endlocal
exit /b %ERRORLEVEL%
