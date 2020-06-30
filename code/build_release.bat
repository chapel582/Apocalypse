@echo off

REM TODO: can we just build both with one exe?

call .\setup.bat

set BuildFolder=..\build
REM TODO: take arguments from batch file and use those to set slow and internal flags 
set CommonCompilerFlags=-DAPOCALYPSE_SLOW=1 -DAPOCALYPSE_WIN32 -MTd -nologo -GR- -EHa- -Oi -O2 -W4 -FC -Z7 -w44062 /wd4533 /wd4201
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

IF NOT EXIST %BuildFolder% mkdir %BuildFolder%
pushd %BuildFolder%
del *.pdb > nul 2> nul
rem NOTE: Not using WX because I want to keep the 4100 warning but it will
rem CONT: always be hit on the WinMain callback 
rem NOTE: -w44062 is here because, for some reason, it is not seen by the 
rem CONT: level 4 warnings
rem NOTE: -EHA- Disabled exception handling
rem NOTE: -GR- Disabled runtime type information
rem NOTE: -MT is used so that we don't use the c-runtime library DLL but 
rem CONT: statically link instead 
cl %CommonCompilerFlags% ..\code\win32_apocalypse.cpp -Fmwin32_apocalypse.map /link %CommonLinkerFlags%
popd