@echo off

REM TODO - can we just build both with one exe?

call .\setup.bat

set BuildFolder=..\build
set CommonCompilerFlags=-MTd -nologo -GR- -EHa- -Oi -W4 -FC -Z7
set CommonLinkerFlags=-opt:ref user32.lib gdi32.lib winmm.lib

IF NOT EXIST %BuildFolder% mkdir %BuildFolder%
pushd %BuildFolder%
del *.pdb > nul 2> nul
rem NOTE: Not using WX because I want to keep the 4100 warning but it will
rem NOTE: always be hit on the WinMain callback 
rem NOTE: Disabled exception handling
rem NOTE: Disabled runtime type information
rem NOTE: -MT is used so that we don't use the c-runtime library DLL but 
rem NOTE: statically link instead 
rem NOTE: -Od should be removed for non-debug builds
cl %CommonCompilerFlags% ..\code\win32_main.cpp -Fmwin32_main.map /link %CommonLinkerFlags%
popd