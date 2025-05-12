# Crazy Compiler :D

This is a custom C++ compiler project built using **Lex** and **Yacc** (Flex and Bison), which performs:
- Lexical analysis
- Syntax analysis
- Semantic analysis
- Symbol table construction
- Intermediate code generation (Quadruples)


📁 Project Structure


.
├── src
│   └── compiler.cpp
│   └── compiler.h
│   └── lexer.l
│   └── parser.y
├── logs
│   └── bison_errors.log
│   └── flex_errors.log
│   └── gcc_errors.log
├── outputs
│   └── action.txt
│   └── error.txt
│   └── symbol.txt
├── GUI
│   └── gui.py
├── run.sh
├── run.bat
└── README.md