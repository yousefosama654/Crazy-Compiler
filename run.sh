#!/bin/bash

clear 

# Create necessary folders if they don't exist
mkdir -p logs


echo "📦 Removing old files..."
rm -f compiler compiler.exe src/parser.tab.c src/parser.tab.h src/y.tab.c src/y.tab.h src/lex.yy.c \
      logs/bison_errors.log logs/flex_errors.log logs/gcc_errors.log logs/runtime_errors.log \
      outputs/*.txt

echo "Running Bison..."
bison -y -d src/parser.y -o src/y.tab.c 2> logs/bison_errors.log
if [ $? -ne 0 ]; then
    echo "Bison failed!" >&2
    exit 1
fi

echo "Running Flex..."
flex -o src/lex.yy.c src/lexer.l  2> logs/flex_errors.log
if [ $? -ne 0 ]; then
    echo "Flex failed!" >&2
    exit 1
fi

echo "Compiling with GCC..."
g++ src/compiler.cpp src/y.tab.c src/lex.yy.c -o compiler.exe 2> logs/gcc_errors.log
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