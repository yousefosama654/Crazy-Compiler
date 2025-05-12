%{
    //INCLUDES
    #include <stdio.h>
    #include "compiler.h"
    #include <stdbool.h>
    //----------------------------------------------
    //Function Prototypes
    extern void remove_errors_file();
    extern void log_errors(int,const char *);
    /* TODO: implement this function and see what it can do */
    // extern void check_unused_variables();
    //----------------------------------------------
    // we will define these functions here 
    Node *construct_operation_node(int oper, int nops, ...);
    Node *construct_identifier_node(char*, int = -1, int = -1);
    Node *construct_constant_node(int, int, ...);
    Node *construct_decleration_node(char*, int, int = -1);
    //----------------------------------------------

    void free_node(Node *p);
    // int execute_all(Node *p, int = -1, int = -1, int = 0, ...);

    //----------------------------------------------
    int yylex(void);
    void yyerror(const char *emsg);
    extern int line;
    Scope p_current_scope (0,NULL); // Global variable to keep track of the current scope
    //----------------------------------------------
%}
/* End of Definitions */

/* Part 2 : Unions of yylval */
%union {
  int intValue;                          /* integer  */
  float floatValue;                      /* double   */
  char* stringValue;                     /* string   */
  char charValue;                      /* char     */
  bool boolValue;                        /* boolean  */

  char *sIndex;                       /* symbol table index */
  char *varType;                      /* variable type      */
  Node *nodePtr;                      /*   node             */
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
%token <intValue> INTEGER
%token <floatValue> FLOAT
%token <stringValue> STRING
%token <charValue> CHAR
%token <boolValue> BOOL

%token <sIndex> VARIABLE

/* Keywords of terminal symbols  to define CFG */
%token IF                                                                /* Keywords for if statement */
%token SWITCH CASE DEFAULT                                               /* Keywords for switch statement */
%token FOR WHILE DO UNTIL BREAK CONTINUE                                       /* Keywords for loops */
%token CONST INT_TYPE FLOAT_TYPE BOOL_TYPE STRING_TYPE  VOID_TYPE  DECLARE_ONLY CHAR_TYPE           /* Keywords for data types */
%token FUNCTION                                                          /* Keyword for function declaration */
%token PRINT                                                             /* Keyword for print */
%token RETURN 
%token BLOCK
%token DECREMENT  INCREMENT
%token POST_INCREMENT POST_DECREMENT PRE_INCREMENT PRE_DECREMENT
/* to resolve dangling if else problem */
%nonassoc IFX
%nonassoc ELSE
%nonassoc FUNC

/* End of Tokens */

/* precedence of Operators */
/* left and right keywords gove  */
%right '='
%left OR
%left AND
%left SHIFT_RIGHT SHIFT_LEFT BITWISE_NOT BITWISE_XOR BITWISE_OR BITWISE_AND
%left GREATER_EQUAL LESS_EQUAL EQUAL NOT_EQUAL '>' '<'
%left '+' '-'
%left '*' '/' '%'
%right NOT 
%nonassoc NEGATIVE


/* Non Terminal Types */
%type <nodePtr> statement statement_list  expression  rhs_nested_expression
%type <nodePtr> declaration_statement  assignment_statement functions
%type <nodePtr> for_statement for_declaration for_mid_stmt for_assignment   
%type <nodePtr> while_statement if_statement do_while_statement switch_statement
%type <nodePtr>  cases default_statement 
%type <nodePtr> function_declaration parameter_list function_call return_statement comma_expressions 
%type <intValue> data_type


/* Part 4 : Production Rules */
%%

/* TODO: remember to make this function */
program: functions      { /*last thing to finish check_unused_variables(); */}
        ;

functions: functions statement { begin_compile($2,Scope (0, NULL),0,-1,-1,0,NULL);/*execute_all($2); free_node($2);*/ }
          | {$$ = NULL;}
          ;


statement: ';'                                 { $$ = construct_operation_node(';', 2, NULL, NULL); }
          | expression ';'                     { $$ = $1; }
          | PRINT expression ';'               { $$ = construct_operation_node(PRINT, 1, $2); }

          | assignment_statement ';'           { $$ = $1;; } 
          | declaration_statement ';'          { $$ = $1;; }
          
          | while_statement                    { $$ = $1;; }
          | do_while_statement                 { $$ = $1; }
          | for_statement                      { $$ = $1; }
          
          | function_call ';'                     { $$ = $1; }
          | function_declaration               { $$ = $1;}
          | return_statement                   { $$ = $1; }

          | if_statement                       { $$ = $1; }
          | switch_statement                   { $$ = $1; }
          | '{' statement_list '}'             { $$ = construct_operation_node(BLOCK, 1, $2); }
          | BREAK ';'                          { $$ = construct_operation_node(BREAK, 1, NULL); }
          | CONTINUE ';'                       { $$ = construct_operation_node(CONTINUE, 1, NULL); }
          ;
                    
/* Functions */
function_declaration: FUNCTION declaration_statement'('parameter_list')' '{'statement_list  return_statement'}'  {$$=construct_operation_node(FUNCTION,4,$2,$4,$7,$8);}
                | FUNCTION declaration_statement'('parameter_list')' '{'return_statement'}'  {$$=construct_operation_node(FUNCTION,4,$2,$4,NULL,$7);}
             ;
                    

parameter_list: declaration_statement ',' parameter_list          {$$=construct_operation_node(COMMA,2,$1,$3);}
              | declaration_statement                              {$$=$1;}
              |{$$=NULL;}
              ;

function_call : VARIABLE '(' comma_expressions ')'   {$$=construct_operation_node(CALL,2,construct_identifier_node($1),$3);}
            ; 

return_statement: RETURN statement { $$ = construct_operation_node(RETURN, 1, $2); }
                ;
                
comma_expressions: comma_expressions','expression { $$ = construct_operation_node(COMMA, 2, $1, $3); }
                | expression                        { $$ = $1; }
                |                                   { $$ = NULL; }
                ;
/* Loops */
while_statement:
  WHILE '(' expression ')' statement { $$ = construct_operation_node(WHILE, 2, $3, $5); }
  ;

do_while_statement: DO statement_list UNTIL expression ';' { $$ = construct_operation_node(DO, 2, $2, $4); }
                  ;

for_statement: FOR '(' for_declaration ';' for_mid_stmt ';' for_assignment ')' '{' statement_list '}' { $$ = construct_operation_node(FOR, 4, $3, $5, $7, $10); }
  ;

/* Conditional Statements */
if_statement: IF '(' expression ')' '{' statement_list '}' %prec IFX { $$ = construct_operation_node(IF, 2, $3, $6); }
            | IF '(' expression ')' '{' statement_list '}' ELSE '{' statement_list '}' { $$ = construct_operation_node(IF, 3, $3, $6, $10); }
            ;


default_statement:
  DEFAULT ':' statement { $$ = construct_operation_node(DEFAULT, 1, $3); }
  ;

switch_statement :  SWITCH '(' VARIABLE ')' '{' cases '}'                           {$$=construct_operation_node(SWITCH,2,construct_identifier_node($3),$6);}
                 |  SWITCH '(' VARIABLE ')' '{' cases  default_statement'}'          {$$=construct_operation_node(SWITCH,3,construct_identifier_node($3),$6,$7);}
                 ;

cases : CASE INTEGER ':' statement BREAK ';' cases                  {$$=construct_operation_node(CASE,4,construct_constant_node(INTEGER,INT_TYPE,$2),$4,construct_operation_node(BREAK,0),$7);}
      /* | CASE INTEGER ':''{'statement_list BREAK';' '}'  cases                   {$$=construct_operation_node(CASE,4,construct_constant_node(INTEGER,INT_TYPE,$2),$5,construct_operation_node(BREAK,0),$9);} */
      | CASE INTEGER ':' statement  BREAK ';'                {$$=construct_operation_node(CASE,3,construct_constant_node(INTEGER,INT_TYPE,$2),$4,construct_operation_node(BREAK,0));}
      /* | CASE INTEGER ':' '{'statement_list  BREAK ';' '}'               {$$=construct_operation_node(CASE,3,construct_constant_node(INTEGER,INT_TYPE,$2),$5,construct_operation_node(BREAK,0));} */

      ;



for_mid_stmt:
  { $$ = construct_operation_node(';', 2, NULL, NULL); }
  | PRINT expression { $$ = construct_operation_node(PRINT, 1, $2); }
  | declaration_statement { $$ = $1; }
  | expression { $$ = $1; }
  ;

assignment_statement: VARIABLE '=' function_call %prec FUNC { $$ = construct_operation_node('=', 2, construct_identifier_node($1), $3); }
                    | VARIABLE '=' rhs_nested_expression    { $$ = construct_operation_node('=', 2, construct_identifier_node($1), $3); }
  ;

for_assignment:
  { $$ = construct_operation_node(';', 2, NULL, NULL); }
  | assignment_statement { $$ = $1; }
  ;

declaration_statement: data_type VARIABLE                                             {     $$ = construct_decleration_node($2, $1); }
                    | data_type VARIABLE '=' rhs_nested_expression                    { $$ = construct_operation_node('=', 2, construct_decleration_node($2, $1), $4); }
                    | CONST data_type VARIABLE '=' rhs_nested_expression              { $$ = construct_operation_node('=', 2, construct_decleration_node($3, $2,CONST), $5); }
                    /* | CONST data_type VARIABLE   '='    function_call %prec FUNC          { $$ = construct_operation_node('=', 2, construct_decleration_node($3, $2,CONST), $5); } */
                    /* | data_type VARIABLE '='  function_call %prec FUNC                            { $$ = construct_operation_node('=', 2, construct_decleration_node($2, $1), $4); } */
                    ;

data_type: INT_TYPE       { $$ = INT_TYPE; } 
          | FLOAT_TYPE    { $$ = FLOAT_TYPE; }
          | BOOL_TYPE     { $$ = BOOL_TYPE; }
          | STRING_TYPE   { $$ = STRING_TYPE; }
          | VOID_TYPE     { $$ = VOID_TYPE; }
          | CHAR_TYPE     { $$ = CHAR_TYPE; }
          ;

for_declaration:
  { $$ = construct_operation_node(';', 2, NULL, NULL); }
  | declaration_statement { $$ = $1; }
  ;

rhs_nested_expression: expression                                          { $$ = $1; }
                      | VARIABLE '=' rhs_nested_expression                 { $$ = construct_operation_node('=', 2, construct_identifier_node($1), $3); }
                      | '(' VARIABLE '=' rhs_nested_expression ')'         { $$ = construct_operation_node('=', 2, construct_identifier_node($2), $4); }
                      ;

statement_list: statement                                                   { $$ = $1; }
              | statement_list statement                                    { $$ = construct_operation_node(';', 2, $1, $2); }
              ;


expression: INTEGER                                          { $$ = construct_constant_node(INTEGER, INT_TYPE, $1); }
          | FLOAT                                            { $$ = construct_constant_node(FLOAT, FLOAT_TYPE, $1); }
          | STRING                                           { $$ = construct_constant_node(STRING, STRING_TYPE, $1); }
          | CHAR                                             { $$ = construct_constant_node(CHAR, CHAR_TYPE, $1); }
          | VARIABLE                                         { $$ = construct_identifier_node($1); }
          | BOOL                                             { $$ = construct_constant_node (BOOL, BOOL_TYPE, $1); }
          | '-' expression %prec NEGATIVE                    { $$ = construct_operation_node(NEGATIVE, 1, $2); }
          | NOT expression                                   { $$ = construct_operation_node(NOT, 1, $2); }
          | expression '+' expression                        { $$ = construct_operation_node('+', 2, $1, $3); }
          | expression '-' expression                        { $$ = construct_operation_node('-', 2, $1, $3); }
          | expression '*' expression                        { $$ = construct_operation_node('*', 2, $1, $3); }
          | expression '/' expression                        { $$ = construct_operation_node('/', 2, $1, $3); }
          | expression '%' expression                        { $$ = construct_operation_node('%', 2, $1, $3); }
          | expression '<' expression                        { $$ = construct_operation_node('<', 2, $1, $3); }
          | expression '>' expression                        { $$ = construct_operation_node('>', 2, $1, $3); }
          | expression GREATER_EQUAL expression              { $$ = construct_operation_node(GREATER_EQUAL, 2, $1, $3); }
          | expression SHIFT_LEFT expression                 { $$ = construct_operation_node(SHIFT_LEFT, 2, $1, $3); }
          | expression SHIFT_RIGHT expression                { $$ = construct_operation_node(SHIFT_RIGHT, 2, $1, $3); }
          | expression BITWISE_AND expression                { $$ = construct_operation_node(BITWISE_AND, 2, $1, $3); }
          | expression BITWISE_OR expression                 { $$ = construct_operation_node(BITWISE_OR, 2, $1, $3); }
          | BITWISE_NOT expression                { $$ = construct_operation_node(BITWISE_NOT, 1, $2); }
          | expression BITWISE_XOR expression                   { $$ = construct_operation_node(BITWISE_XOR, 2, $1, $3); }
          | expression LESS_EQUAL expression                 { $$ = construct_operation_node(LESS_EQUAL, 2, $1, $3); }
          | expression NOT_EQUAL expression                  { $$ = construct_operation_node(NOT_EQUAL, 2, $1, $3); }
          | expression EQUAL expression                      { $$ = construct_operation_node(EQUAL, 2, $1, $3); }
          | expression AND expression                        { $$ = construct_operation_node(AND, 2, $1, $3); }
          | expression OR expression                         { $$ = construct_operation_node(OR, 2, $1, $3); }
          | '(' expression ')'                               { $$ = $2; }         
          |INCREMENT VARIABLE    {$$=construct_operation_node(PRE_INCREMENT, 1, construct_identifier_node($2));}
          |VARIABLE INCREMENT     {$$=construct_operation_node(POST_INCREMENT, 1, construct_identifier_node($1));}
          |DECREMENT VARIABLE     {$$=construct_operation_node(PRE_DECREMENT, 1, construct_identifier_node($2));}
          |VARIABLE DECREMENT      {$$=construct_operation_node(POST_DECREMENT, 1, construct_identifier_node($1));}
;  /* End of Production Rules */

%%
/* type is the token in bison i defined before  */
/* TODO: remove the type as long it is not used here  */
Node *construct_constant_node(int type, int dataType, ...) {
  va_list ap;
  Node *p;
  size_t nodeSize;

  /* allocate Node */
  nodeSize = SIZEOFNODE + sizeof(ConstantNode);
  if ((p = (Node*)malloc(nodeSize)) == NULL)
    yyerror("out of memory");

  /* copy information */
  p->type = CONSTANT;
    p->line= line; // Set the line number for error reporting

  p->con.dataType = dataType;
  va_start(ap, dataType);
  p->con.value = va_arg(ap, ValueType);
  va_end(ap);
  return p;
}

Node *construct_identifier_node(char* i, int dataType, int qualifier) {
  Node *p;
  size_t nodeSize;
  /* allocate Node */
  nodeSize = SIZEOFNODE + sizeof(IdentifierNode);
  if ((p = (Node*)malloc(nodeSize)) == NULL)
    yyerror("out of memory");

  /* copy information */
  p->type = IDENTIFIER;
  p->line= line; // Set the line number for error reporting
  p->id.name = strdup(i);
  p->id.dataType = dataType;
  p->id.qualifier = qualifier;
  return p;
}

Node *construct_operation_node(int oper, int nops, ...) {
  va_list ap;
  Node *p;
  size_t nodeSize;
  int i;
  
  /* allocate Node */
  /*  (nops - 1) as it is aready there is one location in memory */
  nodeSize = SIZEOFNODE + sizeof(OperationNode) + (nops - 1) * sizeof(Node*);
  if ((p = (Node*)malloc(nodeSize)) == NULL)
  {
    yyerror("out of memory");
  }

  /* copy information */
  p->type = OPERATION;
  p->line= line; // Set the line number for error reporting
  p->opr.symbol = oper;
  p->opr.nops = nops;
  va_start(ap, nops);
  for (i = 0; i < nops; i++)
    p->opr.op[i] = va_arg(ap, Node*);
  va_end(ap);
  return p;
}

void free_node(Node *p) {
  int i;
  if (!p) return;
  if (p->type == OPERATION) {
    for (i = 0; i < p->opr.nops; i++)
        free_node(p->opr.op[i]);
  }
  free (p);
}
extern FILE *yyin;  // Declare yyin so you can set it

int main(void) {
 yyin = fopen("input.txt", "r");  // Open the input file
    if (!yyin) {
        perror("Failed to open input.txt");
        return 1;
    }

    yyparse();  // Start parsing
print_symbol_table();
    fclose(yyin);  // Clean up
    return 0;
}


void yyerror(const char *msg) {
  log_errors(line,msg);
  printf("Error: %s\n", msg);
  printf("Line: %d\n", line);
  /* log_symbol_table(); */
  exit(1);
}




    Node *construct_decleration_node(char* name, int type, int qualifier) {
        Node *p;
        size_t nodeSize;
        /* allocate Node */
        nodeSize = SIZEOFNODE + sizeof(DeclerationNode);
        if ((p = (Node*)malloc(nodeSize)) == NULL)
            yyerror("out of memory");

        /* copy information */
        p->type = DECLARATION;
          p->line= line; // Set the line number for error reporting

        p->dec.symbol = strdup(name);
        p->dec.dataType = type;
        p->dec.qualifier = qualifier;
        return p;
    }
    

    
