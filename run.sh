#!/bin/bash

clear 

echo "Removing old files..."
rm -f compiler compiler.exe y.tab.c y.tab.h lex.yy.c bison_errors.log flex_errors.log gcc_errors.log runtime_errors.log outputs/*.txt

echo "Running Bison..."
bison -y -d parser.y 2> bison_errors.log
if [ $? -ne 0 ]; then
    echo "Bison failed!" >&2
    exit 1
fi

echo "Running Flex..."
flex lexer.l 2> flex_errors.log
if [ $? -ne 0 ]; then
    echo "Flex failed!" >&2
    exit 1
fi

echo "Compiling with GCC..."
g++ compiler.cpp y.tab.c lex.yy.c -o compiler.exe 2> gcc_errors.log
if [ $? -ne 0 ]; then
    echo "GCC compilation failed!" >&2
    exit 1
fi

echo "Running compiler..."
./compiler.exe
if [ $? -ne 0 ]; then
    echo " ❌ Runtime error!" >&2
    exit 1
fi

rm -f bison_errors.log flex_errors.log gcc_errors.log runtime_errors.log

echo "✅ Compilation and execution completed successfully!"