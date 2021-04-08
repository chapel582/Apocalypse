@echo off

call .\setup.bat

set BuildFolder=..\build
set CommonCompilerFlags=-MTd -nologo -GR- -EHa- -Oi -W4 -FC -Z7
set CommonLinkerFlags=-opt:ref

set FinalFlags=%CommonCompilerFlags% -Od
echo %FinalFlags%
IF NOT EXIST %BuildFolder% mkdir %BuildFolder%
pushd %BuildFolder%
del *.pdb > nul 2> nul
cl %CommonCompilerFlags% ..\code\coredump_reader.cpp /link %CommonLinkerFlags%
popd