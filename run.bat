@echo off
cls

REM Create logs directory if it doesn't exist
if not exist logs (
    mkdir logs
)

echo 📦 Removing old files...
del /f /q compiler.exe src\parser.tab.c src\parser.tab.h src\y.tab.c src\y.tab.h src\lex.yy.c
del /f /q logs\bison_errors.log logs\flex_errors.log logs\gcc_errors.log logs\runtime_errors.log
del /f /q outputs\*.txt 2>nul

echo Running Bison...
bison -y -d src\parser.y -o src\y.tab.c 2> logs\bison_errors.log
if errorlevel 1 (
    echo Bison failed!
    exit /b 1
)

echo Running Flex...
flex -o src\lex.yy.c src\lexer.l 2> logs\flex_errors.log
if errorlevel 1 (
    echo Flex failed!
    exit /b 1
)

echo Compiling with GCC...
g++ src\compiler.cpp src\y.tab.c src\lex.yy.c -o compiler.exe 2> logs\gcc_errors.log
if errorlevel 1 (
    echo GCC compilation failed!
    exit /b 1
)

echo Running compiler...
compiler.exe
if errorlevel 1 (
    echo ❌ Runtime error!
    exit /b 1
)

del /f /q logs\bison_errors.log logs\flex_errors.log logs\gcc_errors.log logs\runtime_errors.log

echo ✅ Compilation and execution completed successfully!
pause
