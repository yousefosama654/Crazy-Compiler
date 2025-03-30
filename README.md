# Crazy Compiler :D

<!-- (to generate y.tab.h and y.tab.c) -->

## How to run

bison -y -d parser.y
flex lexar.l
gcc y.tab.c lex.yy.c
Get-Content in.txt | a.exe
