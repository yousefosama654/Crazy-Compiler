/* Part 1 : Definitions */
%{
     #include "y.tab.h"
    #include <stdbool.h>
    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>

int line= 1;
void yyerror(const char *s) ;

%}

%option yylineno

/* End of Definitions */


/* Part 2 : Regular Expressions */
%%

[1-9][0-9]*                       {  
                              yylval.i = atoi(yytext); 
                              return INTEGER; }
0                            { 
                              yylval.i = atoi(yytext); 
                              return INTEGER; }
(0|[1-9][0-9]*)\.[0-9]*     {    yylval.f = atof(yytext);
                                return FLOAT; }

'[^\\']' { 
                                yylval.c = strdup(yytext); 
                                return CHAR; }

\"([^\\"]|\\.)*\" {
                                yylval.s = strdup(yytext); 
                                return STRING; }              


"print"                     { return PRINT;  }
"return"                    { return RETURN; }

"true"                      { return true_BOOL; }
"false"                     { return false_BOOL; }


[-()+*/{}<>=.^%,:]             { return yytext[0]; }

"["                 { return '['; }
"]"                 { return ']'; }
"++"                { return INCREMENT; }
"--"                { return DECREMENT; }


">="                        { return GREATER_EQUAL; }
"<="                        { return LESS_EQUAL; }
"=="                        { return EQUAL; } 
"!="                        { return NOTEQUAL; }
"and"                       { return AND; }
"or"                        { return OR; }
"not"                       { return NOT; }

"const"                     { return CONST;  }
"int"                       { return INT_TYPE;    }
"float"                     { return FLOAT_TYPE;  }
"char"                      { return CHAR_TYPE;   }
"string"                    { return STRING_TYPE; }
"bool"                      { return BOOL_TYPE;   }
"void"                     { return VOID;  }

"for"                       { return FOR;     }

"do"                        { return DO;     }
"while"                     { return WHILE;  }

"break"                     { return BREAK;    }
"continue"                  { return CONTINUE; }


"else"                      { return ELSE;    }
"if"                        { return IF;      }

"case"                      { return CASE;    }
"switch"                    { return SWITCH;  }

"function"                  { return FUNCTION; }   
"default"                   { return DEFAULT; }



[a-zA-Z_][a-zA-Z0-9_]*      { return VARIABLE;  }
 ;  {  return ';';}


"//".*                     { /* ignore single line comments */ }
"/*"([^*]|\*+[^*/])*\*+"/" { /* ignore multi-line comments */ }
\n     {
    line+=1; 
  
     }
[ \t]+  { /* ignore whitespace */ }







.    { 
    char msg[1024];
    snprintf(msg, sizeof(msg), "Unknown character: %s", yytext);
    yyerror(msg);
}


%%
/* End of Regular Expressions */

int yywrap(void) {
    return 1;
}