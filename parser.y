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
    void print_ast(Node *node, int indent) ;
%}
/* End of Definitions */

/* Part 2 : Unions of yylval */
%union {
  int intValue;                          /* integer  */
  float floatValue;                      /* double   */
  char* stringValue;                     /* string   */
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
%token <boolValue> BOOL

%token <sIndex> VARIABLE

/* Keywords of terminal symbols  to define CFG */
%token IF                                                                /* Keywords for if statement */
%token SWITCH CASE DEFAULT                                               /* Keywords for switch statement */
%token FOR WHILE DO REPEAT BREAK CONTINUE                                       /* Keywords for loops */
%token CONST INT_TYPE FLOAT_TYPE BOOL_TYPE STRING_TYPE  VOID_TYPE  DECLARE_ONLY           /* Keywords for data types */
%token FUNCTION                                                          /* Keyword for function declaration */
%token PRINT                                                             /* Keyword for print */
%token RETURN 
%token BLOCK

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

functions: functions statement { begin_compile($2,Scope (0, NULL));/*execute_all($2); free_node($2);*/ }
          | {$$ = NULL;}
          ;


statement: ';'                                 { $$ = construct_operation_node(';', 2, NULL, NULL); }
          | expression ';'                     { $$ = $1; }
          | PRINT expression ';'               { $$ = construct_operation_node(PRINT, 1, $2); }

          | assignment_statement ';'           { $$ = $1; } 
          | declaration_statement ';'          { $$ = $1; }
          
          | while_statement                    { $$ = $1; }
          | do_while_statement                 { $$ = $1; }
          | for_statement                      { $$ = $1; }
          
          | function_call                      { $$ = $1; }
          | function_declaration               { $$ = $1;}
          | return_statement                   { $$ = $1; }

          | if_statement                       { $$ = $1; }
          | switch_statement                   { $$ = $1; }
          | '{' statement_list '}'             { $$ = construct_operation_node(BLOCK, 1, $2); }
          | BREAK ';'                          { $$ = construct_operation_node(BREAK, 1, NULL); }
          | CONTINUE ';'                       { $$ = construct_operation_node(CONTINUE, 1, NULL); } 
          ;
                    
/* Functions */
function_declaration: FUNCTION declaration_statement'('parameter_list')' '{'statement  return_statement'}'  {$$=construct_operation_node(FUNCTION,4,$2,$4,$7,$8);}

parameter_list: declaration_statement ',' parameter_list          {$$=construct_operation_node(COMMA,2,$1,$3);}
              | declaration_statement                              {$$=$1;}
              |{$$=NULL;}
              ;

function_call : VARIABLE '(' comma_expressions ')' ';'   {$$=construct_operation_node(CALL,2,construct_identifier_node($1),$3);}
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

do_while_statement: DO statement_list REPEAT expression ';' { $$ = construct_operation_node(DO, 2, $2, $4); }
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
      | CASE INTEGER ':' statement  BREAK ';'                {$$=construct_operation_node(CASE,3,construct_constant_node(INTEGER,INT_TYPE,$2),$4,construct_operation_node(BREAK,0));}
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
                    ;

data_type: INT_TYPE       { $$ = INT_TYPE; } 
          | FLOAT_TYPE    { $$ = FLOAT_TYPE; }
          | BOOL_TYPE     { $$ = BOOL_TYPE; }
          | STRING_TYPE   { $$ = STRING_TYPE; }
          | VOID_TYPE     { $$ = VOID_TYPE; }
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
          | expression LESS_EQUAL expression                 { $$ = construct_operation_node(LESS_EQUAL, 2, $1, $3); }
          | expression NOT_EQUAL expression                  { $$ = construct_operation_node(NOT_EQUAL, 2, $1, $3); }
          | expression EQUAL expression                      { $$ = construct_operation_node(EQUAL, 2, $1, $3); }
          | expression AND expression                        { $$ = construct_operation_node(AND, 2, $1, $3); }
          | expression OR expression                         { $$ = construct_operation_node(OR, 2, $1, $3); }
          | '(' expression ')'                               { $$ = $2; }          ;
  /* End of Production Rules */

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
  /* log_symbol_table(); */
  exit(1);
}


void print_ast(Node *node, int indent) {
    if (node == NULL) return;

    // Print indentation
    for (int i = 0; i < indent; i++) printf("  ");

    // Print node type
    switch (node->type) {
        case CONSTANT:
            printf("Constant: ");
            switch (node->con.dataType) {
                case INT_TYPE: printf("%d (int)\n", node->con.value.intVal); break;
                case FLOAT_TYPE: printf("%f (float)\n", node->con.value.floatVal); break;
                case BOOL_TYPE: printf("%s (bool)\n", node->con.value.boolVal ? "true" : "false"); break;
                case STRING_TYPE: printf("\"%s\" (string)\n", node->con.value.strVal); break;
                default: printf("Unknown constant type\n");
            }
            break;

        case IDENTIFIER:
            printf("Identifier: %s\t", node->id.name);
            printf("Data Type: ");
            switch (node->id.dataType) {
                case INT_TYPE: printf("int\t"); break;
                case FLOAT_TYPE: printf("float\t"); break;
                case BOOL_TYPE: printf("bool\t"); break;
                case STRING_TYPE: printf("string\t"); break;
                case VOID_TYPE: printf("void\t"); break;
                default: printf("Unknown data type\t");
            }
            printf("Qualifier: ");
            switch (node->id.qualifier) {
                case CONST: printf("const\n"); break;
                case INT_TYPE: printf("int\n"); break;
                case FLOAT_TYPE: printf("float\n"); break;
                case BOOL_TYPE: printf("bool\n"); break;
                case STRING_TYPE: printf("string\n"); break;
                default: printf("Unknown qualifier\n");
            }
            break;
        case DECLARATION:
            printf("Declaration: %s\t", node->dec.symbol);
            printf("Data Type: ");
            switch (node->dec.dataType) {
                case INT_TYPE: printf("int\t"); break;
                case FLOAT_TYPE: printf("float\t"); break;
                case BOOL_TYPE: printf("bool\t"); break;
                case STRING_TYPE: printf("string\t"); break;
                case VOID_TYPE: printf("void\t"); break;
                default: printf("Unknown data type\t");
            }
            printf("Qualifier: ");
            switch (node->dec.qualifier) {
                case CONST: printf("const\n"); break;
                case INT_TYPE: printf("int\n"); break;
                case FLOAT_TYPE: printf("float\n"); break;
                case BOOL_TYPE: printf("bool\n"); break;
                case STRING_TYPE: printf("string\n"); break;
                default: printf("Unknown qualifier\n");
            }
            break;
        case OPERATION:
            printf("Operator: ");
            switch (node->opr.symbol) {
                case '+': printf("+\n"); break;
                case '-': printf("-\n"); break;
                case '*': printf("*\n"); break;
                case '/': printf("/\n"); break;
                case '%': printf("%%\n"); break;
                case '=': printf("=\n"); break;
                case PRINT: printf("PRINT\n"); break;
                case IF: printf("IF\n"); break;
                case WHILE: printf("WHILE\n"); break;
                case DO: printf("DO-WHILE\n"); break;
                case FOR: printf("FOR\n"); break;
                case SWITCH: printf("SWITCH\n"); break;
                case CASE: printf("CASE\n"); break;
                case DEFAULT: printf("DEFAULT\n"); break;
                case BLOCK: printf("BLOCK\n"); break;
                case RETURN: printf("RETURN\n"); break;
                case CALL: printf("FUNCTION CALL\n"); break;
                case FUNCTION: printf("FUNCTION DECL\n"); break;
                case COMMA: printf("COMMA\n"); break;
                case NEGATIVE: printf("NEGATIVE\n"); break;
                case NOT: printf("NOT\n"); break;
                case AND: printf("AND\n"); break;
                case OR: printf("OR\n"); break;
                case EQUAL: printf("EQUAL\n"); break;
                case NOT_EQUAL: printf("NOT_EQUAL\n"); break;
                case GREATER_EQUAL: printf("GREATER_EQUAL\n"); break;
                case LESS_EQUAL: printf("LESS_EQUAL\n"); break;
                case '<': printf("<\n"); break;
                case '>': printf(">\n"); break;
                case BREAK: printf("BREAK\n"); break;
                case CONTINUE: printf("CONTINUE\n"); break;
                case ';': printf("SEQUENCE (;)\n"); break;
                default: printf("Unknown operation: %d\n", node->opr.symbol);
            }

            for (int i = 0; i < node->opr.nops; ++i) {
                print_ast(node->opr.op[i], indent + 1);
            }
            break;
    }
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
        p->dec.symbol = strdup(name);
        p->dec.dataType = type;
        p->dec.qualifier = qualifier;
        return p;
    }
    