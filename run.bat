@echo off
setlocal enabledelayedexpansion

REM Enable debug mode (manually echoing each command)
echo Removing old files...
del /Q compiler.exe y.tab.c y.tab.h lex.yy.c bison_errors.log flex_errors.log gcc_errors.log runtime_errors.log 2>nul

REM Run Bison
echo Running Bison...
bison -y -d parser.y 2> bison_errors.log
if errorlevel 1 (
    echo  Bison failed! Check bison_errors.log
    type bison_errors.log
    exit /b 1
)

REM Run Flex
echo Running Flex...
flex lexer.l 2> flex_errors.log
if errorlevel 1 (
    echo  Flex failed! Check flex_errors.log
    type flex_errors.log
    exit /b 1
)

REM Compile with GCC
echo Compiling with GCC...
gcc y.tab.c lex.yy.c 2> gcc_errors.log
if errorlevel 1 (
    echo  GCC compilation failed! Check gcc_errors.log
    type gcc_errors.log
    exit /b 1
)

REM Run the compiler
echo Running compiler...
a.exe  2> runtime_errors.log
if errorlevel 1 (
    echo Runtime error! Check runtime_errors.log
    type runtime_errors.log
    exit /b 1
)

REM If everything succeeded, delete logs
del /Q bison_errors.log flex_errors.log gcc_errors.log runtime_errors.log 2>nul

echo  Compilation and execution completed successfully!
