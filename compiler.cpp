#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include "compiler.h"
#include "y.tab.h"
#include <string.h>
#include <sstream>
#include <fstream>
using namespace std;
extern int line;
static int label = 0;
static int level = 0;
void log_errors(int l, const char *msg)
{
    printf("Error: %s at line %d\n", msg, l);
    FILE *ef = fopen("./outputs/error.txt", "w");
    if (ef == NULL)
    {
        fprintf(ef, "Error: couldn't open error.txt for writing.\n");
        return;
    }
    if (string(msg).find("Semantic ERROR") != string::npos)
    {
        fprintf(ef, "Line %d: %s\n", l, msg);
        return;
    }
    ifstream input("input.txt");
    if (!input.is_open())
    {
        fprintf(ef, "Warning: couldn't read input.txt for context.\n");
        return;
    }

    string lineText, lastValidLine;
    int currentLine = 1, lastValidLineNumber = 0;
    while (getline(input, lineText) && currentLine < l)
    {
        const char *raw = lineText.c_str();
        while (*raw == ' ' || *raw == '\t')
            raw++; // skip leading spaces

        if (*raw != '\0' && *raw != '\n' && *raw != '/')
        {
            lastValidLine = lineText;
            lastValidLineNumber = currentLine;
        }
        currentLine++;
    }

    if (!lastValidLine.empty())
    {
        const char *str = lastValidLine.c_str();
        // trim str
        int len = strlen(str);
        while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t'))
        {
            len--;
        }
        string trimmedStr = string(str, len);

        len = trimmedStr.length();

        // check if the last character is a semicolon

        if (len >= 2 && trimmedStr[len - 1] != ';')
        {
            fprintf(ef, "syntax error at line %d\n", lastValidLineNumber);
            fprintf(ef, "Note: maybe a semicolon \n");
        }
        else
        {
            fprintf(ef, "syntax error at line %d\n", line);
        }
    }
    else
    {
        fprintf(ef, "syntax error at line %d\n", line);
    }
}
char *get_type(int type)
{
    switch (type)
    {
    case INT_TYPE:
        return "int";
    case FLOAT_TYPE:
        return "float";
    case BOOL_TYPE:
        return "bool";
    case STRING_TYPE:
        return "string";
    default:
        return "unknown";
    }
}
FILE *fp = NULL;
map<int, vector<Symbol *>> symbol_table;           // symbol table
map<int, vector<SymbolFunction *>> function_table; // function table
void add_symbol(char *name, int type, int qualifier, Scope scope, bool isused, bool isInitialized)
{

    // check if global symbol already exists
    if (symbol_table.find(0) == symbol_table.end())
    {
        symbol_table[0] = vector<Symbol *>();
    }
    // heck if varibable is already declared in the current scope or any parent scope
    // get all parent scopes
    Scope *current_scope = &scope;
    while (current_scope != nullptr)
    {
        for (auto &symbol : symbol_table[current_scope->level])
        {
            if (symbol->name == name)
            {
                char msg[1024];
                sprintf(msg, "Semantic ERROR: variable %s already declared in scope %d\n", name, scope.level);
                yyerror(msg);
                return;
            }
        }
        current_scope = current_scope->parent;
    }

    Symbol *s = new Symbol;
    s->name = name;
    s->type = type;
    s->Qualfier = qualifier;
    s->scope_level = scope.level;
    s->used = isused;
    s->isInitialized = isInitialized;
    symbol_table[scope.level].push_back(s);
}
void add_func(char *name, int type, vector<Symbol> argTypes, Scope scope)
{
    // check if global symbol already exists
    if (function_table.find(0) == function_table.end())
    {
        function_table[0] = vector<SymbolFunction *>();
    }
    // heck if varibable is already declared in the current scope or any parent scope
    // get all parent scopes
    Scope *current_scope = &scope;
    while (current_scope != nullptr)
    {
        for (auto &symbol : function_table[current_scope->level])
        {
            if (symbol->name == name)
            {
                char msg[1024];
                sprintf(msg, "Semantic ERROR: function %s already declared in scope %d\n", name, scope.level);
                yyerror(msg);
                return;
            }
        }
        current_scope = current_scope->parent;
    }

    SymbolFunction *s = new SymbolFunction;
    s->name = name;
    s->returnType = type;
    s->argTypes = argTypes;
    s->used = false;
    function_table[scope.level].push_back(s);
}
void add_scope(Scope *scope)
{
    // check if global symbol already exists
    if (symbol_table.find(scope->level) == symbol_table.end())
    {
        symbol_table[scope->level] = vector<Symbol *>(); // create new scope in symbol table
    }
}

int use_symbol(char *name, Scope scope)
{
    // check if varibable is already declared in the current scope or any parent scope
    // get all parent scopes
    Scope *current_scope = &scope;
    while (current_scope != nullptr)
    {
        for (auto &symbol : symbol_table[current_scope->level])
        {
            if (symbol->name == name)
            {
                symbol->used = true; // mark as used
                printf("used %s\n", symbol->name.c_str());
                return symbol->type; // return type
            }
        }

        current_scope = current_scope->parent;
    }
    char msg[1024];
    sprintf(msg, "Semantic ERROR: variable %s not declared in scope %d\n", name, scope.level);
    yyerror(msg);
}
void initilize_symbol(char *name, Scope scope)
{
    // check if varibable is already declared in the current scope or any parent scope
    // get all parent scopes
    Scope *current_scope = &scope;
    while (current_scope != nullptr)
    {
        for (auto &symbol : symbol_table[current_scope->level])
        {

            if (symbol->name == name)
            {
                symbol->isInitialized = true; // mark as used
                return;
            }
        }
        current_scope = current_scope->parent;
    }
}

void check_if_const(char *name, Scope scope)
{
    // check if varibable is already declared in the current scope or any parent scope
    // get all parent scopes
    Scope *current_scope = &scope;
    while (current_scope != nullptr)
    {
        for (auto &symbol : symbol_table[current_scope->level])
        {
            if (symbol->name == name && symbol->Qualfier == CONST)
            {
                char msg[1024];
                sprintf(msg, "Semantic ERROR: variable %s is constant and cannot be modified\n", name);
                yyerror(msg);
                return;
            }
        }
        current_scope = current_scope->parent;
    }
}

void open_file()
{
    if (fp == NULL)
    {
        fp = fopen("./outputs/action.txt", "w");
    }
}

void print_symbol_table()
{
    // open symbol.txt
    FILE *sf = fopen("./outputs/symbol.txt", "w");
    if (sf == NULL)
    {
        fprintf(sf, "Error: couldn't open symbol.txt for writing.\n");
        return;
    }
    fprintf(sf, "Variables:\n");
    for (auto &entry : symbol_table)
    {
        int scope = entry.first;
        vector<Symbol *> symbols = entry.second;
        bool f=0;
        for (auto &symbol : symbols)
        {
            if(f==0){
                fprintf(sf, "Scope %d:\n", scope);
                f=1;
            }
            fprintf(sf, "\tName: %s, Type: %s, isConst: %d isUsed: %d, isIntilized: %d\n", symbol->name.c_str(), get_type(symbol->type), symbol->Qualfier == CONST, symbol->used, symbol->isInitialized);
        }

    }
    fprintf(sf, "\nFunctions:\n");
    for (auto &entry : function_table)
    {
        int scope = entry.first;
        vector<SymbolFunction *> symbols = entry.second;
        // fprintf(sf, "Scope %d:\n", scope);
        for (auto &symbol : symbols)
        {
            fprintf(sf, "Name: %s, Return Type: %s\n", symbol->name.c_str(), get_type(symbol->returnType));
            for (auto &arg : symbol->argTypes)
            {
                fprintf(sf, "\tArg Name: %s, Type: %s\n", arg.name.c_str(), get_type(arg.type));
            }
        }
    }
    fclose(sf);
}

void add_argument(char *name, char *argName, int type, int qualifier, Scope scope)
{
    printf("add argument %s %s\n", name, argName);
    Scope *current_scope = &scope;
    while (current_scope != nullptr)
    {
        for (auto &symbol : function_table[current_scope->level])
        {
            if (symbol->name == name)
            {
                Symbol s;
                s.name = argName;
                s.type = type;
                s.Qualfier = qualifier;
                symbol->argTypes.push_back(s);
                return;
            }
        }
        current_scope = current_scope->parent;
    }
}
int check_function(char *name, Scope scope)
{
    // check if varibable is already declared
    // get all parent scopes
    Scope *current_scope = &scope;
    while (current_scope != nullptr)
    {
        for (auto &symbol : function_table[current_scope->level])
        {
            if (symbol->name == name)
            {
                symbol->used = true; // mark as used
                return symbol->returnType; // return type
            }
        }
        current_scope = current_scope->parent;
    }
    char msg[1024];
    sprintf(msg, "Semantic ERROR: function %s not declared in scope %d\n", name, scope.level);
    yyerror(msg);
}
int begin_compile(Node *p, Scope scope_level, bool flag, int brk, int cont, int isFunction, char *funcName)
{
    // open action files
    open_file();
    if (p == NULL)
    {
        return 0;
    }
    switch (p->type)
    {
    case CONSTANT:
    {
        switch (p->con.dataType)
        {

        case INT_TYPE:
            fprintf(fp, "push (%s) %d\n", get_type(p->con.dataType), p->con.value.intVal);
            return INT_TYPE;
            break;
        case FLOAT_TYPE:
            fprintf(fp, "push (%s) %f\n", get_type(p->con.dataType), p->con.value.floatVal);
            return FLOAT_TYPE;
            break;
        case BOOL_TYPE:
            fprintf(fp, "push (%s) %d\n", get_type(p->con.dataType), p->con.value.boolVal);
            return BOOL_TYPE;
            break;
        case STRING_TYPE:
            fprintf(fp, "push (%s) %s\n", get_type(p->con.dataType), p->con.value.strVal);
            return STRING_TYPE;
            break;
        default:
            printf("ERROR");
            return p->con.dataType;
            break;
        }
        break;
    }
    case IDENTIFIER:
    {
        int type = use_symbol(p->id.name, scope_level);
        if (!flag)
            fprintf(fp, "push %s\n", p->id.name);
        return type;
        break;
    }
    case DECLARATION:
    {
        if (isFunction == 1)
        {
            add_argument(funcName, p->dec.symbol, p->dec.dataType, p->dec.qualifier, scope_level);
        }
        add_symbol(p->dec.symbol, p->dec.dataType, p->dec.qualifier, scope_level, false, false);
        return p->dec.dataType;
        break;
    }

    case OPERATION:
    {
        switch (p->opr.symbol)
        {

        case PRINT:
        {
            int op = begin_compile(p->opr.op[0], scope_level, 0, brk, cont, isFunction, funcName);
            fprintf(fp, "print (%s)\n", get_type(op));
            return 0;
            break;
        }
        case CALL:
        {
            ///check if function exists in the symbol table
           int t= check_function(p->opr.op[0]->id.name, scope_level);
            fprintf(fp, "call %s\n", p->opr.op[0]->id.name);
           return t;
            break;
        }
        case BLOCK:
        {
            // open new scope
            Scope *new_scope = new Scope(++level, &scope_level);
            add_scope(new_scope);
            // compile the block
            for (int i = 0; i < p->opr.nops; i++)
            {
                begin_compile(p->opr.op[i], *new_scope, 0, brk, cont, isFunction, funcName);
            }
            break;
        }
        case COMMA:

            begin_compile(p->opr.op[0], scope_level, 0, brk, cont, isFunction, funcName);
            begin_compile(p->opr.op[1], scope_level, 0, brk, cont, isFunction, funcName);
            break;
        case NOT:
        {
            int op = begin_compile(p->opr.op[0], scope_level, 0, brk, cont, isFunction, funcName);
            if (op != BOOL_TYPE)
            {
                char msg[1024];
                sprintf(msg, "Semantic ERROR: cannot apply unary NOT to %s\n", get_type(op));
                yyerror(msg);
            }
            fprintf(fp, "not\n");
            return op;
            break;
        }
        case ';':
        {
            for (int i = 0; i < p->opr.nops; i++)
                begin_compile(p->opr.op[i], scope_level, 0, brk, cont, isFunction, funcName);
            return VOID_TYPE;
            break;
        }
        case IF:
        {

            int if_expr = begin_compile(p->opr.op[0], scope_level, 0, brk, cont, isFunction, funcName);
            if (if_expr != BOOL_TYPE)
            {
                char msg[1024];
                sprintf(msg, "Semantic ERROR:expression must be bool\n");
                yyerror(msg);
            }
            // if
            Scope *new_scope = new Scope(++level, &scope_level);
            add_scope(new_scope);
            if (p->opr.nops == 2)
            {

                int end = label;
                fprintf(fp, "jz L%d\n", label++);

                begin_compile(p->opr.op[1], *new_scope, 0, brk, cont, isFunction, funcName);

                fprintf(fp, "L%d\n", end);
            }
            else if (p->opr.nops == 3)
            {
                int end = label;
                fprintf(fp, "jz L%d\n", label++);
                begin_compile(p->opr.op[1], *new_scope, 0, brk, cont, isFunction, funcName); // if body
                fprintf(fp, "L%d\n", end);
                Scope *new_scope2 = new Scope(++level, &scope_level);
                add_scope(new_scope2);
                begin_compile(p->opr.op[2], *new_scope2, 0, brk, cont, isFunction, funcName); // else body
            }
            return 0;
            break;
        }
        case FOR:
        {

            Scope *new_scope = new Scope(++level, &scope_level);
            add_scope(new_scope);
            begin_compile(p->opr.op[0], scope_level, 0, brk, cont, isFunction, funcName); // initialize
            int l1 = label++;
            fprintf(fp, "L%d\n", l1);

            int cond = begin_compile(p->opr.op[1], scope_level, 0, brk, cont, isFunction, funcName); // condition
            if (cond != BOOL_TYPE)
            {
                yyerror("Semantic ERROR: Condition must be of a BOOL Value");
            }

            int l2 = label++;
            fprintf(fp, "jz L%d\n", l2);

            begin_compile(p->opr.op[3], *new_scope, 0, l2, l1, isFunction, funcName);     // body
            begin_compile(p->opr.op[2], scope_level, 0, brk, cont, isFunction, funcName); // next iter inc/dec
            fprintf(fp, "jmp L%d\n", l1);                                                 // jump to condition
            fprintf(fp, "L%d\n", l2);                                                     // end of loop
            break;
        }
        case BREAK:
        {
            if (brk == -1)
            {
                yyerror("Semantic ERROR: No loop to Break from");
                break;
            }
            fprintf(fp, "jmp L%d\n", brk);
            break;
        }
        /*SWITCH CASE*/
        case SWITCH:
        {

            int endLabel = label++;     // Unique end label for switch
            int defaultLabel = label++; // Default case label
            NodeTypeTag *switch_var = p->opr.op[0];

            // pop the switch variable

            NodeTypeTag *n = p->opr.op[1];
            int i = 1;
            while (n->opr.symbol == CASE)
            {
                bool last_case = (n->opr.nops == 3); // CASE with 3 operands is last in the list

                fprintf(fp, "L%d:\n", label);

                fprintf(fp, "push %s\n", switch_var->id.name);

                begin_compile(n->opr.op[0], scope_level, 0, brk, cont, isFunction, funcName); // push the case constant
                fprintf(fp, "\tcompEQ\n");

                if (last_case)
                {
                    if (p->opr.nops == 3) // switch has a default case
                    {
                        fprintf(fp, " jz L%d\n", defaultLabel);
                    }
                    else
                    {
                        fprintf(fp, "jz L%d\n", endLabel);
                    }
                }
                else
                {
                    fprintf(fp, "jz L%d\n", label + 1);
                }
                Scope *new_scope = new Scope(++level, &scope_level);
                add_scope(new_scope);
                printf("new scope parent %d\n", new_scope->parent->level);
                printf("curr scope parent %d\n", new_scope->level);
                begin_compile(n->opr.op[1], *new_scope, 0, brk, cont, isFunction, funcName); // execute case body
                fprintf(fp, "jmp L%d\n", endLabel);

                if (last_case)
                {
                    break;
                }
                else
                {
                    n = n->opr.op[3]; // move to next case
                    label++;
                }
            }

            if (p->opr.nops == 3) // handle default if exists
            {
                fprintf(fp, "L%d:\n", defaultLabel);
                Scope *new_scope = new Scope(++level, &scope_level);
                add_scope(new_scope);
                begin_compile(p->opr.op[2]->opr.op[0], *new_scope, 0, brk, cont, isFunction, funcName); // default case body
                fprintf(fp, "jmp L%d\n", endLabel);
            }

            fprintf(fp, "L%d:\n", endLabel);

            break;
        }
        case FUNCTION:
        {
            printf("function %s %s\n", p->opr.op[0]->id.name, get_type(p->opr.op[0]->dec.dataType));
            fprintf(fp, "proc %s\n", p->opr.op[0]->id.name);
            Scope *new_scope = new Scope(++level, &scope_level);
            add_scope(new_scope);
            add_func(p->opr.op[0]->id.name, p->opr.op[0]->dec.dataType, vector<Symbol>(), scope_level); // add function to symbol table
            if (p->opr.op[1])
            {
                begin_compile(p->opr.op[1], *new_scope, 0, brk, cont, 1, p->opr.op[0]->id.name); // function arguments
            }
          
            begin_compile(p->opr.op[2], *new_scope, 0, brk, cont, isFunction, funcName);               // function body
            int ret_typ = begin_compile(p->opr.op[3], *new_scope, 0, brk, cont, isFunction, funcName); // return statement
            if (ret_typ != p->opr.op[0]->dec.dataType)
            {
                char msg[1024];
                sprintf(msg, "Semantic ERROR: function %s return type mismatch\n", p->opr.op[0]->id.name);
                yyerror(msg);
            }
            return ret_typ;
            break;
        }
        case RETURN:

        {

            int t = begin_compile(p->opr.op[0], scope_level, 0, brk, cont, isFunction, funcName); // return statement
            fprintf(fp, "ret\n");
            fprintf(fp, "endproc\t\n");
            return t;
            break;
        }
        case CONTINUE:
        {
            printf("continue %d\n", cont);
            if (cont == -1)
            {
                yyerror("Semantic ERROR: Continue statement not in loop");
                break;
            }
            fprintf(fp, "jmp L%d\n", cont);
            break;
        }
        case DO:
        {
            Scope *new_scope = new Scope(++level, &scope_level);
            int l1 = label++;
            int l2 = label++;
            fprintf(fp, "L%d\n", l1);

            begin_compile(p->opr.op[0], *new_scope, 0, l2, l1, isFunction, funcName);                // body
            int cond = begin_compile(p->opr.op[1], scope_level, 0, brk, cont, isFunction, funcName); // condition
            if (cond != BOOL_TYPE)
            {
                yyerror("Semantic ERROR: Condition must be of a BOOL value");
            }
            fprintf(fp, "jnz L%d\n", l1);

            fprintf(fp, "L%d\n", l2);

            break;
        }
        case WHILE:
        {

            Scope *new_scope = new Scope(++level, &scope_level);
            int l1 = label;
            fprintf(fp, "L%d\n", label++);

            int type1 = begin_compile(p->opr.op[0], scope_level, 0, brk, cont, isFunction, funcName); // condition
            if (type1 != BOOL_TYPE)
            {
                yyerror("semantic ERROR: Conditions must be of a BOOL Value");
            }
            int l2 = label++;
            fprintf(fp, "jz L%d\n", l2);

            begin_compile(p->opr.op[1], scope_level, 0, l2, l1, isFunction, funcName); // body

            fprintf(fp, "jmp L%d\n", l1);

            fprintf(fp, "L%d\n", l2);

            break;
        }
        case NEGATIVE:
        {
            int op = begin_compile(p->opr.op[0], scope_level, 0, brk, cont, isFunction, funcName);
            if (op != INT_TYPE && op != FLOAT_TYPE)
            {
                char msg[1024];
                sprintf(msg, "Semantic ERROR: cannot apply unary minus to %s\n", get_type(op));
                yyerror(msg);
            }
            fprintf(fp, "neg\n");
            return op;
            break;
        }
        case '=':
        {
            int ass = begin_compile(p->opr.op[1], scope_level, 0, brk, cont, isFunction, funcName);
            // int op1 = begin_compile(p->opr.op[0]);
            // check if op[0] is dec or id
            int t = begin_compile(p->opr.op[0], scope_level, true, brk, cont, isFunction, funcName);
            if (p->opr.op[0]->type == DECLARATION)
            {
                if (p->opr.op[0]->dec.dataType != ass)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR: cannot assign %s to %s\n", get_type(ass), get_type(p->opr.op[0]->id.dataType));
                    yyerror(msg);
                }
                fprintf(fp, "pop %s\n", p->opr.op[0]->dec.symbol);
                initilize_symbol(p->opr.op[0]->dec.symbol, scope_level);
            }
            else
            {
                // check if identifer is constant
                check_if_const(p->opr.op[0]->id.name, scope_level);
                if (t != ass)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR: cannot assign %s to %s\n", get_type(ass), get_type(p->opr.op[0]->id.dataType));
                    yyerror(msg);
                }
                fprintf(fp, "pop %s\n", p->opr.op[0]->id.name);
                initilize_symbol(p->opr.op[0]->id.name, scope_level);
            }

            return ass;
            break;
        }
        default:
        { // two operands
            int op1 = begin_compile(p->opr.op[0], scope_level, 0, brk, cont, isFunction, funcName);
            int op2 = begin_compile(p->opr.op[1], scope_level, 0, brk, cont, isFunction, funcName);
            if (op1 != op2)
            {
                char msg[1024];
                sprintf(msg, "Semantic ERROR: MISMATCH Operands data types ( operand1 %s and operand2 %s)", get_type(op1), get_type(op2));
                yyerror(msg);
            }
            switch (p->opr.symbol)
            {
            case '+':
                if (op1 != INT_TYPE && op1 != FLOAT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "cannot add operands of type %s", get_type(op1));
                    yyerror(msg);
                }
                fprintf(fp, "add\n");
                return op1;
                break;
            case '-':
                if (op1 != INT_TYPE && op1 != FLOAT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "cannot subtract operands of type %s", get_type(op1));
                    yyerror(msg);
                }
                fprintf(fp, "sub\n");
                return op1;
                break;
            case '*':
                if (op1 != INT_TYPE && op1 != FLOAT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "cannot multiply operands of type %s", get_type(op1));
                    yyerror(msg);
                }
                fprintf(fp, "mul\n");
                return op1;
                break;
            case '/':
                if (op1 != INT_TYPE && op1 != FLOAT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "cannot divide operands of type %s", get_type(op1));
                    yyerror(msg);
                }
                fprintf(fp, "div\n");
                return op1;
                break;
            case '%':
                if (op1 != INT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "cannot mod operands of type %s", get_type(op1));
                    yyerror(msg);
                }
                fprintf(fp, "mod\n");
                return op1;
                break;
            case '<':
                if (op1 != INT_TYPE && op1 != FLOAT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "cannot compare operands of type %s", get_type(op1));
                    yyerror(msg);
                }
                fprintf(fp, "lt\n");
                return BOOL_TYPE;
                break;
            case '>':
                if (op1 != INT_TYPE && op1 != FLOAT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "cannot compare operands of type %s", get_type(op1));
                    yyerror(msg);
                }
                fprintf(fp, "gt\n");
                return BOOL_TYPE;
                break;

            case GREATER_EQUAL:
                if (op1 != INT_TYPE && op1 != FLOAT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "cannot compare operands of type %s", get_type(op1));
                    yyerror(msg);
                }
                fprintf(fp, "ge\n");
                return BOOL_TYPE;
                break;
            case LESS_EQUAL:
                if (op1 != INT_TYPE && op1 != FLOAT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "cannot compare operands of type %s", get_type(op1));
                    yyerror(msg);
                }
                fprintf(fp, "le\n");
                return BOOL_TYPE;
                break;
            case NOT_EQUAL:
                fprintf(fp, "ne\n");
                return BOOL_TYPE;
                break;
            case EQUAL:
                fprintf(fp, "eq\n");
                return BOOL_TYPE;
                break;
            case AND:
                if (op1 != BOOL_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR: cannot apply AND to %s\n", get_type(op1));
                    yyerror(msg);
                }
                fprintf(fp, "and\n");
                return op1;
                break;
            case OR:
                if (op1 != BOOL_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR: cannot apply OR to %s\n", get_type(op1));
                    yyerror(msg);
                }
                fprintf(fp, "or\n");
                return op1;
                break;
            }
        }
        }
    }
        return 0;
    }
}
