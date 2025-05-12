%{
    #include "compiler.h"
    #include "y.tab.h"
    #include <stdbool.h>
    void yyerror (const char *emsg); 
    int line=1;
%}

%option yylineno
/* End of Definitions */


/* Part 2 : Regular Expressions */
%%
[0-9]+                       { /* INTEGERS */ 
                              yylval.intValue = atoi(yytext); 
                              return INTEGER; }

(0|[1-9][0-9]*)\.[0-9]*     { /* FLOATS */ 
                              char * x = strdup(yytext); // to ensure null-terminated string
                              yylval.floatValue = atof(x);
                              return FLOAT; }
"true"                      { /* BOOLS */ 
                              yylval.boolValue = 1;
                               return BOOL; }
"false"                     {/* BOOLS */ 
                              yylval.boolValue = 0;
                               return BOOL; }

"const"                     { return CONST;       }
"int"                       { return INT_TYPE;    }
"float"                     { return FLOAT_TYPE;  }
"string"                    { return STRING_TYPE; }
"char"                      { return CHAR_TYPE;   }
"bool"                      { return BOOL_TYPE;   }
"void"                      { return VOID_TYPE;   }

"print"                     { return PRINT;  }
"return"                    { return RETURN; }

[-()<>=+*/%{}.:;,]             { return yytext[0]; }

">="                        { return GREATER_EQUAL; }
"<="                        { return LESS_EQUAL;    }
"=="                        { return EQUAL;         } 
"!="                        { return NOT_EQUAL;      }

"&"                          { return BITWISE_AND;  }
"|"                         { return BITWISE_OR;   }
"^"                         { return BITWISE_XOR;      }
"~"                         { return BITWISE_NOT;  }
"<<"                        { return SHIFT_LEFT;   }
">>"                        { return SHIFT_RIGHT;  }




"and"                       { return AND;           }
"or"                        { return OR;            }
"not"                       { return NOT;           }


 

"for"                       { return FOR;      }

"do"                        { return DO;       }
"while"                     { return WHILE;    }
"until"                   { return UNTIL;    }

"if"                        { return IF;       }
"else"                      { return ELSE;     }

"switch"                    { return SWITCH;   }
"case"                      { return CASE;     }

"def"                       { return FUNCTION; }   
"default"                   { return DEFAULT;  }

"break"                     { return BREAK;    }
"continue"                  { return CONTINUE; }

[a-zA-Z_][a-zA-Z0-9_]*      { 
                              yylval.sIndex = strdup(yytext);
                              return VARIABLE;  }
"++"                      { return INCREMENT; }
"--"                      { return DECREMENT; }

\".*\"                      {
                              yylval.stringValue = strdup(yytext);
                              return STRING;    }
'([^'])'                   {   yylval.charValue = yytext[1];
                              return CHAR;      }


[ \t\r]+  { /* ignore whitespace */ }
\n        { line++; }

"#".*                        { /*Add a Comments*/
                                if (yytext[yyleng - 1] == '\n') 
                                {
                                    yylineno--;   
                                 }
                                }

.   { 
    char msg[1024];
    printf(msg, "Syntax ERROR: Unknown character: %s", yytext);
    yyerror(msg);
  }

%%
/* End of Regular Expressions */

int yywrap(void) {
    return 1;
}