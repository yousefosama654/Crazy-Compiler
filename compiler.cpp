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

void log_errors(int l, const char *msg)
{
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
        size_t len = strlen(str);
        if (len >= 2 && str[len - 2] != ';')
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
map<int, vector<Symbol *>> symbol_table; // symbol table

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
    printf("current scope %d\n", current_scope->level);
    while (current_scope != nullptr)
    {
        printf("current scope %d\n", current_scope->level);
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
        printf("cuurrent scope %d\n", current_scope->level);
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
void initilize_symbol(char *name)
{
    // check if varibable is already declared in the current scope or any parent scope
    // get all parent scopes
    for(auto &entry : symbol_table)
    {
        for (auto &symbol : entry.second)
        {
            if (symbol->name == name)
            {
                symbol->isInitialized = true; // mark as used
                return;
            }
        }
    }
   
}

void open_file()
{
    if (fp == NULL)
    {
        fp = fopen("./outputs/action.txt", "w");
    }
}
int begin_compile(Node *p, Scope scope_level)
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
        switch (p->con.dataType)
        {

        case INT_TYPE:
            fprintf(fp, "push %s %d\n", get_type(p->con.dataType), p->con.value.intVal);
            return INT_TYPE;
            break;
        case FLOAT_TYPE:
            fprintf(fp, "push %s %f\n", get_type(p->con.dataType), p->con.value.floatVal);
            return FLOAT_TYPE;
            break;
        case BOOL_TYPE:
            fprintf(fp, "push %s %d\n", get_type(p->con.dataType), p->con.value.boolVal);
            return BOOL_TYPE;
            break;
        case STRING_TYPE:
            fprintf(fp, "push %s %s\n", get_type(p->con.dataType), p->con.value.strVal);
            return STRING_TYPE;
            break;
        default:
            printf("ERROR");
            return p->con.dataType;
            break;
        }
        break;
    case IDENTIFIER:
    {
        int type=use_symbol(p->id.name, scope_level);

        fprintf(fp, "push %s %s\n", get_type(type), p->id.name);
        printf("type %s\n", get_type(type));
        return type;
        break;
    }
    case DECLARATION:
    {
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
            int op = begin_compile(p->opr.op[0], scope_level);
            fprintf(fp, "print %s\n", get_type(op));
            return 0;
        }
        case BLOCK:
        {
            // open new scope
            Scope *new_scope = new Scope(scope_level.level + 1, &scope_level);
            add_scope(new_scope);
            // compile the block
            for (int i = 0; i < p->opr.nops; i++)
            {
                begin_compile(p->opr.op[i], *new_scope);
            }
            break;
        }
        case NOT:
        {
            int op = begin_compile(p->opr.op[0], scope_level);
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
        case NEGATIVE:
        {
            int op = begin_compile(p->opr.op[0], scope_level);
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
            int ass = begin_compile(p->opr.op[1], scope_level);
            // int op1 = begin_compile(p->opr.op[0]);
            // check if op[0] is dec or id
            begin_compile(p->opr.op[0], scope_level);
            if (p->opr.op[0]->type == DECLARATION)
            {
                if (p->opr.op[0]->dec.dataType != ass)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR: cannot assign %s to %s\n", get_type(ass), get_type(p->opr.op[0]->id.dataType));
                    yyerror(msg);
                }
                fprintf(fp,"pop %s\n", p->opr.op[0]->dec.symbol);
                initilize_symbol(p->opr.op[0]->dec.symbol);

            }
            else
            {
                if (p->opr.op[0]->id.dataType != ass)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR: cannot assign %s to %s\n", get_type(ass), get_type(p->opr.op[0]->id.dataType));
                    yyerror(msg);
                }
                fprintf(fp,"pop %s\n", p->opr.op[0]->id.name);
                initilize_symbol(p->opr.op[0]->id.name);
            }
            return ass;
            break;
        }
        default:
            // two operands
            int op1 = begin_compile(p->opr.op[0], scope_level);
            int op2 = begin_compile(p->opr.op[1], scope_level);
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
                return op1;
                break;
            case '>':
                if (op1 != INT_TYPE && op1 != FLOAT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "cannot compare operands of type %s", get_type(op1));
                    yyerror(msg);
                }
                fprintf(fp, "gt\n");
                return op1;
                break;

            case GREATER_EQUAL:
                if (op1 != INT_TYPE && op1 != FLOAT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "cannot compare operands of type %s", get_type(op1));
                    yyerror(msg);
                }
                fprintf(fp, "ge\n");
                return op1;
                break;
            case LESS_EQUAL:
                if (op1 != INT_TYPE && op1 != FLOAT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "cannot compare operands of type %s", get_type(op1));
                    yyerror(msg);
                }
                fprintf(fp, "le\n");
                return op1;
                break;
            case NOT_EQUAL:
                fprintf(fp, "ne\n");
                return op1;
                break;
            case EQUAL:
                fprintf(fp, "eq\n");
                return op1;
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
        return 0;
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
    fprintf(sf, "Symbol Table:\n");
    for (auto &entry : symbol_table)
    {
        int scope = entry.first;
        vector<Symbol *> symbols = entry.second;
        fprintf(sf, "Scope %d:\n", scope);
        for (auto &symbol : symbols)
        {
            fprintf(sf, "Name: %s, Type: %s, isConst: %d isUsed: %d, isIntilized: %d\n", symbol->name.c_str(), get_type(symbol->type), symbol->Qualfier == CONST, symbol->used, symbol->isInitialized);
        }
    }
    fclose(sf);
}
