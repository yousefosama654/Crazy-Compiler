#!/bin/bash

# Enable debug mode to show each command
set -x

# Remove old files
rm -f compiler.exe y.tab.c y.tab.h lex.yy.c

# Run Bison (capture errors)
bison -y -d parser.y 2> bison_errors.log
if [ $? -ne 0 ]; then
    echo "❌ Bison failed! Check bison_errors.log"
    cat bison_errors.log
    exit 1
fi

# Run Flex (capture errors)
flex lexer.l 2> flex_errors.log
if [ $? -ne 0 ]; then
    echo "❌ Flex failed! Check flex_errors.log"
    cat flex_errors.log
    exit 1
fi

# Compile with GCC (capture errors)
gcc y.tab.c lex.yy.c -o compiler.exe -ll -lm 2> gcc_errors.log
if [ $? -ne 0 ]; then
    echo "❌ GCC compilation failed! Check gcc_errors.log"
    cat gcc_errors.log
    exit 1
fi

# Run the parser and capture runtime errors
./compiler.exe < in.txt 2> runtime_errors.log
if [ $? -ne 0 ]; then
    echo "❌ Runtime error! Check runtime_errors.log"
    cat runtime_errors.log
    exit 1
fi

echo "✅ Compilation and execution completed successfully!"
