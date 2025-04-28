
echo Removing old files...
del /Q compiler.exe y.tab.c y.tab.h lex.yy.c bison_errors.log flex_errors.log gcc_errors.log runtime_errors.log   ./outputs/*.txt 2>nul
del /Q outputs\* 2>nul
echo Running Bison...
bison -y -d parser.y


echo Running Flex...
flex lexer.l 

REM Compile with GCC
echo Compiling with GCC...
g++ compiler.cpp  y.tab.c lex.yy.c 


REM Run the compiler
echo Running compiler...
a.exe 


del /Q bison_errors.log flex_errors.log gcc_errors.log runtime_errors.log 2>nul

echo  Compilation and execution completed successfully!
