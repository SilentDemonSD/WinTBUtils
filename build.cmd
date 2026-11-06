@echo off
setlocal
set "HERE=%~dp0"
set "OUT=%~1"
if "%OUT%"=="" set "OUT=TaskbarClear"
set "PATH=%PATH%;%ProgramFiles(x86)%\Microsoft Visual Studio\Installer"
set "VCVARS="
for /f "usebackq delims=" %%P in (`vswhere.exe -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find VC\Auxiliary\Build\vcvars64.bat`) do set "VCVARS=%%P"
if not defined VCVARS (
    echo Visual Studio with the "Desktop development with C++" workload is required.
    exit /b 1
)
call "%VCVARS%" >nul || exit /b 1
if not exist "%HERE%bin" mkdir "%HERE%bin"
pushd "%HERE%bin"
cl /nologo /std:c++20 /permissive- /EHsc /O1 /GL /GS /Gw /Gy /MT /W3 /DWIN32_LEAN_AND_MEAN /DNOMINMAX /DWINRT_LEAN_AND_MEAN /LD "%HERE%native\TaskbarClear.cpp" /Fe:"%OUT%.dll" /link /LTCG /OPT:REF /OPT:ICF /guard:cf windowsapp.lib user32.lib advapi32.lib ole32.lib
set "RESULT=%ERRORLEVEL%"
del /q *.obj *.exp *.lib 2>nul
popd
exit /b %RESULT%
