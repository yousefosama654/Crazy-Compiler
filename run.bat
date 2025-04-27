@echo off
setlocal enabledelayedexpansion

REM Enable debug mode (manually echoing each command)
echo Removing old files...
del /Q compiler.exe y.tab.c y.tab.h lex.yy.c bison_errors.log flex_errors.log gcc_errors.log runtime_errors.log   ./outputs/*.txt 2>nul
del /Q outputs\* 2>nul
REM Run Bison
echo Running Bison...
bison -y -d parser.y
if errorlevel 1 (
    echo  Bison failed! Check bison_errors.log
    type bison_errors.log
    exit /b 1
)

REM Run Flex
echo Running Flex...
flex lexer.l 
if errorlevel 1 (
    echo  Flex failed! Check flex_errors.log
    type flex_errors.log
    exit /b 1
)

REM Compile with GCC
echo Compiling with GCC...
g++ compiler.cpp  y.tab.c lex.yy.c 


REM Run the compiler
echo Running compiler...
a.exe 


del /Q bison_errors.log flex_errors.log gcc_errors.log runtime_errors.log 2>nul

echo  Compilation and execution completed successfully!
