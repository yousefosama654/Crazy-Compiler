/* Part 1 : Definitions */
%{
    #include <stdio.h>
    // #include "compiler.h"
    #include <stdbool.h>
    extern int line;
    int yylex(void);
    void yyerror(const char *){
        if(line==0)
        printf("syntax error at line 1\n");
      else   printf("syntax error at line %d\n", line);
        fflush(stdout);
    }
%}
/* End of Definitions */



/* Part 2 : Unions */
%union
{
    int i;              /* integer */
    float f;            /* float */
    char *c;             /* char */
    int b;           /*  boolean */
    char *s;            /* string */
}

/* End of Unions */

/* Part 3 : Tokens */
/* 
Keyword     Description
%left	    Identifies tokens that are left-associative with other tokens.
%nonassoc	Identifies tokens that are not associative with other tokens.
%right	    Identifies tokens that are right-associative with other tokens.
%start	    Identifies a nonterminal name for the start symbol.
%token	    Identifies the token names that the yacc command accepts. Declares all token names in the declarations section.
%type	    Identifies the type of nonterminals. Type-checking is performed when this construct is present.
%union	    Identifies the yacc value stack as the union of the various type of values desired. By default, the values returned are integers. The effect of this construct is to provide the declaration of YYSTYPE directly from the input.
*/


/* Data types */
%token <i> INTEGER
%token <f> FLOAT
%token <c> CHAR
%token <b> BOOL
%token <s> STRING
%token <s> VARIABLE
%token <s> CONSTANT
%token <s> true_BOOL
%token <s> false_BOOL


/* Keywords */
%token IF                                                           /*  if conditions  */
%token SWITCH CASE DEFAULT                                          /*  switch  */
%token FOR WHILE DO BREAK CONTINUE                              /*  loops */
%token CONST INT_TYPE FLOAT_TYPE BOOL_TYPE CHAR_TYPE STRING_TYPE  VOID  /*  data types */
%token FUNCTION                                                     /*  functions */
%token PRINT                                                        /*  Keyword for print */
%nonassoc RETURN
%nonassoc IFX                                                       /*  If statement precedance handling*/
%nonassoc ELSE                                                      /*  else statement */
/* Operators */
/* The order matters as we go down the precedence of the operator increases */
/* left and right keywords gove  */

%right '='
%left OR
%left AND
%left GREATER_EQUAL LESS_EQUAL EQUAL NOTEQUAL '<' '>'
%left '+' '-'
%left '*' '/' '%'
%right NOT
%left DECREMENT INCREMENT /* Post-increment (x++) and Post-decrement (x--) */


/* Non Terminal Types */

/* End of Tokens */


/* Part 4 : Production Rules */
%%

program : statement_list
        ;

statement_list : statement
               | statement_list statement
               ;

statement : simple_statement
          | compound_statement
          | '{' statement_list '}'
          |'{''}'
            | ';'
          ;

simple_statement : assignment_statement ';'    {{ printf("Assignment statement\n"); fflush(stdout); }}
                 | declaration_statement ';'  {{ printf("Declaration statement\n"); fflush(stdout); }}
                 | expression   ';'       {{ printf("Expression statement\n"); fflush(stdout); }}    
                 | print_statement   {{ printf("Print statement\n"); fflush(stdout); }}
                 | BREAK';'
                 | CONTINUE';'
               
                 
                 ;
PARAMTER_LIST: data_type VARIABLE ',' PARAMTER_LIST
                | data_type VARIABLE
                | 
                ;
compound_statement : for_statement  {{ printf("For statement\n"); fflush(stdout); }}
                   | while_statement  {{ printf("While statement\n"); fflush(stdout); }}
                   | if_statement     {{ printf("If statement\n"); fflush(stdout); }}
                   | do_while_statement   {{ printf("Do while statement\n"); fflush(stdout); }}
                   | switch_statement    {{ printf("Switch statement\n"); fflush(stdout); }}
                     |FUNCTION VOID VARIABLE'('PARAMTER_LIST')'     '{'   return_expression'}'
                     |FUNCTION VOID VARIABLE'('PARAMTER_LIST')'     '{' statement_list  return_expression'}'
                 | FUNCTION data_type VARIABLE '(' PARAMTER_LIST ')' '{'  return_expression '}'
                 | FUNCTION data_type VARIABLE '(' PARAMTER_LIST ')' '{' statement_list return_expression '}'
                   ;


assignment_statement    : VARIABLE '=' expression  
                      | VARIABLE DECREMENT
           | VARIABLE INCREMENT 

                        ;

print_statement : PRINT '(' expression ')' ';'
                ;   

return_expression : RETURN expression ';'
                  | RETURN  ';'
                  |
                  ;
PARAMTER_LIST_CALL: expression ',' PARAMTER_LIST_CALL
                | expression
                | 
                ;

function_call : VARIABLE'('PARAMTER_LIST_CALL ')' 
            ;

data_type : INT_TYPE
          | FLOAT_TYPE
          | BOOL_TYPE
          | CHAR_TYPE
          | STRING_TYPE 
          ;

declaration_statement : CONST data_type VARIABLE '=' expression 
                      | data_type VARIABLE '=' expression      
                      ;


for_statement : FOR '(' declaration_statement ';' expression ';' assignment_statement ')' statement 
              ;

while_statement : WHILE '(' expression ')' statement
                ;

do_while_statement : DO statement WHILE '(' expression ')'
                   ;

/* to make the else belongs to the final if has more percedence */
if_statement : IF '(' expression ')' statement %prec IFX
             | IF '(' expression ')' statement ELSE statement
             ;

switch_statement :  SWITCH '(' VARIABLE ')' '{' cases '}'
                 |  SWITCH '(' VARIABLE ')' '{' cases  default_statement'}'
                 ;

cases : CASE INTEGER ':' statement_list cases
      | CASE true_BOOL ':' statement_list cases
      | CASE false_BOOL ':' statement_list cases
      |
      ;

default_statement : DEFAULT ':' statement

/* define them due to percedence */
expression : expression '+' expression 
           | expression '-' expression 
           | expression '*' expression 
           | expression '/' expression 
           | expression '%' expression
           | '(' expression ')' 
           | NOT expression
           | expression AND expression
           | expression OR expression
           | expression GREATER_EQUAL expression
           | expression LESS_EQUAL expression
           
           | expression EQUAL expression
           | expression NOTEQUAL expression
           | expression '<' expression
           | expression '>' expression     
           | INTEGER
           | FLOAT
           | CHAR
           | STRING                   
           | VARIABLE           
           | CONSTANT 
           |true_BOOL
           |false_BOOL
           |function_call
           
           ;
%%
/* End of Production Rules */


/* Part 5 : Functions and Main */
int main(void)
{
    // Redirect input to a file
    FILE *input_file = fopen("input.txt", "r");
    if (input_file == NULL) {
        perror("Failed to open input file");
        return 1;
    }

    // Redirect output to a file
    FILE *output_file = fopen("output.txt", "w");
    if (output_file == NULL) {
        perror("Failed to open output file");
        fclose(input_file);
        return 1;
    }

    // Set stdin and stdout to the corresponding files
    freopen("input.txt", "r", stdin);   // Redirect input to file
    freopen("output.txt", "w", stdout);  // Redirect output to file

    // Start parsing
    yyparse();

    // Close files
    fclose(input_file);
    fclose(output_file);

    return 0;
}