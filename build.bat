@echo off
rem FATAL FRAME II: Crimson Butterfly REMAKE - TwinSwap
rem Builds dist\ containing the files users drop into the game root.
setlocal
set "VS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VS%" (echo [NG] vcvars64.bat not found: %VS% & exit /b 1)
call "%VS%" >nul
if errorlevel 1 exit /b 1

set "ROOT=%~dp0"
set "OUT=%ROOT%dist"
set "OBJ=%ROOT%obj"
if not exist "%OUT%\Mods\twinswap" mkdir "%OUT%\Mods\twinswap"
if not exist "%OBJ%" mkdir "%OBJ%"

echo === xinput1_4.dll ===
cl /nologo /LD /O2 /EHsc /MT /W3 /std:c++17 /utf-8 /DNDEBUG /Fo"%OBJ%\t_" /Fe"%OUT%\xinput1_4.dll" "%ROOT%src\twinswap.cpp" /link /DEF:"%ROOT%src\xinput1_4.def" /OPT:REF /OPT:ICF
if errorlevel 1 exit /b 1

echo === copying package files ===
copy /y "%ROOT%package\Mods\twinswap\twinswap.ini" "%OUT%\Mods\twinswap\" >nul
copy /y "%ROOT%package\Mods\twinswap\README.md" "%OUT%\Mods\twinswap\" >nul
copy /y "%ROOT%LICENSE" "%OUT%\Mods\twinswap\LICENSE.txt" >nul

rem import library / export file are build by-products
if exist "%OUT%\xinput1_4.lib" del "%OUT%\xinput1_4.lib"
if exist "%OUT%\xinput1_4.exp" del "%OUT%\xinput1_4.exp"

echo.
echo === done: %OUT% ===
dir /b /s "%OUT%"
endlocal
