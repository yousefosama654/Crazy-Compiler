#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include "compiler.h"
#include "y.tab.h"
#include <string.h>
#include <sstream>
#include <fstream>

extern map<Node *, int> error_line;
using namespace std;
static int label = 0;
static int level = 0;
void log_errors(int l, const char *msg)
{
    FILE *ef = fopen("./outputs/error.txt", "a");
    if (ef == NULL)
    {
        fprintf(ef, "Error: couldn't open error.txt for writing.\n");
        return;
    }
    if (string(msg).find("Semantic ERROR") != string::npos || string(msg).find("warning") != string::npos)
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
            fprintf(ef, "syntax error at line %d\n", l);
        }
    }
    else
    {
        fprintf(ef, "syntax error at line %d\n", l);
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
    case CHAR_TYPE:
        return "char";
    case VOID_TYPE:
        return "void";
    default:
        return "unknown";
    }
}
FILE *fp = NULL;
map<int, vector<Symbol *>> symbol_table;           // symbol table
map<int, vector<SymbolFunction *>> function_table; // function table
void add_symbol(char *name, int type, int qualifier, Scope scope, bool isused, bool isInitialized, int line)
{

    // check if global symbol already exists
    if (symbol_table.find(0) == symbol_table.end())
    {
        symbol_table[0] = vector<Symbol *>();
    }
    // heck if varibable is already declared in the current scope or any parent scope
    // get all parent scopes
    Scope *current_scope = &scope;
    // while (current_scope != nullptr)
    {
        for (auto &symbol : symbol_table[current_scope->level])
        {
            if (symbol->name == name)
            {
                char msg[1024];
                sprintf(msg, "Semantic ERROR: variable %s already declared in scope %d\n", name, scope.level);
                log_errors(line, msg);
                return;
            }
        }
        // current_scope = current_scope->parent;
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
void add_func(char *name, int type, vector<Symbol> argTypes, Scope scope, int line)
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
                log_errors(line, msg);
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

int use_symbol(char *name, Scope scope, int line, bool flag)
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
                if ((symbol->isInitialized == false && !flag))
                {
                    char msg[1024];
                    sprintf(msg, "warning: variable %s not initialized in scope %d\n", name, scope.level);
                    log_errors(line, msg);
                }
                if (!flag)
                    symbol->used = true; // mark as used
                return symbol->type;     // return type
            }
        }

        current_scope = current_scope->parent;
    }
    char msg[1024];
    sprintf(msg, "Semantic ERROR: variable %s not declared in scope %d\n", name, scope.level);
    log_errors(line, msg);
    return 0;
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

void check_if_const(char *name, Scope scope, int line)
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
                log_errors(line, msg);
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
        bool f = 0;
        for (auto &symbol : symbols)
        {
            if (f == 0)
            {
                fprintf(sf, "Scope %d:\n", scope);
                f = 1;
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
            fprintf(sf, "Name: %s, Return Type: %s used: %d\n", symbol->name.c_str(), get_type(symbol->returnType), symbol->used);
            for (size_t i = 0; i < symbol->argTypes.size(); ++i)
            {
                fprintf(sf, "\tArg Name: %s, Type: %s: isdeafult %d\n",
                        symbol->argTypes[i].name.c_str(),
                        get_type(symbol->argTypes[i].type),
                        *symbol->isdefault[i]);
            }
        }
    }
    fclose(sf);
}

void add_argument(char *name, char *argName, int type, int qualifier, Scope scope)
{
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
                symbol->isdefault.push_back((new bool(false)));
                return;
            }
        }
        current_scope = current_scope->parent;
    }
}
int check_function(char *name, Scope scope, int line, vector<int> arg_types)
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
                // check arguments of function match the call and their types
                // if (symbol->argTypes.size() != arg_types.size())
                // {
                //     char msg[1024];
                //     sprintf(msg, "Semantic ERROR: function %s argument mismatch\n", name);
                //     log_errors(line, msg);
                // }
                // else
                {
                    int j = 0;
                    for (int i = arg_types.size() - 1; i >= 0; i--)
                    {
                        if (symbol->argTypes[j++].type != arg_types[i])
                        {
                            char msg[1024];
                            sprintf(msg, "Semantic ERROR: function %s argument type mismatch\n", name);
                            log_errors(line, msg);
                        }
                    }
                    while (j < symbol->argTypes.size())
                    {
                        if (*symbol->isdefault[j] == false)
                        {
                            char msg[1024];
                            sprintf(msg, "Semantic ERROR: function %s argument type mismatch\n", name);
                            log_errors(line, msg);
                        }
                        j++;
                    }
                }

                return symbol->returnType; // return type
            }
        }
        current_scope = current_scope->parent;
    }
    char msg[1024];
    sprintf(msg, "Semantic ERROR: function %s not declared in scope %d\n", name, scope.level);
    log_errors(line, msg);
}
void fix_deafault(char *name, Scope scope)
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
                // for (size_t i = 0; i < symbol->argTypes.size(); ++i)
                // hange last argument to default
                if (symbol->isdefault.size() > 0)
                {
                    printf("fix default %s arg name %s\n", symbol->name.c_str(), symbol->argTypes.back().name.c_str());
                    *symbol->isdefault.back() = true;
                    return ;
                }
                // {
                //     *symbol->isdefault[i] = false;
                // }
                return;
            }
        }
        current_scope = current_scope->parent;
    }
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
        if (isFunction == 1)
        {
            // fix_deafault(funcName,scope_level);
        }
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
        case CHAR_TYPE:
            fprintf(fp, "push (%s) %c\n", get_type(p->con.dataType), p->con.value.charVal);
            return CHAR_TYPE;
            break;
        default:
            return p->con.dataType;
            break;
        }
        break;
    }
    case IDENTIFIER:
    {
        int type = use_symbol(p->id.name, scope_level, p->line, flag);
        if (!flag)
        {
            fprintf(fp, "push %s\n", p->id.name);
        }
        return type;
        break;
    }
    case DECLARATION:
    {
        if (isFunction == 1)
        {
            add_argument(funcName, p->dec.symbol, p->dec.dataType, p->dec.qualifier, scope_level);
           
        }
        if (isFunction == 0)
            add_symbol(p->dec.symbol, p->dec.dataType, p->dec.qualifier, scope_level, false, false, p->line);
        else
            add_symbol(p->dec.symbol, p->dec.dataType, p->dec.qualifier, scope_level, false, true, p->line);

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
            /// check if function exists in the symbol table
            // get arguments types
            vector<int> argTypes;
            Node *n = p->opr.op[1];
            while (n)
            {
                if (n->type == OPERATION)
                {
                    if (n->opr.symbol == COMMA)
                    {
                        int t1 = begin_compile(n->opr.op[1], scope_level, 0, brk, cont, isFunction, funcName);
                        argTypes.push_back(t1);
                        n = n->opr.op[0];
                    }
                }
                else if (n->type == IDENTIFIER)
                {
                    int t1 = begin_compile(n, scope_level, 0, brk, cont, isFunction, funcName);
                    argTypes.push_back(t1);
                    n = NULL;
                }
                else if (n->type == CONSTANT)
                {
                    int t1 = begin_compile(n, scope_level, 0, brk, cont, isFunction, funcName);
                    argTypes.push_back(t1);
                    n = NULL;
                }
            }
          

            int t = check_function(p->opr.op[0]->id.name, scope_level, p->line, argTypes);
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
                log_errors(p->line, msg);
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

            int if_expr = begin_compile(p->opr.op[0], scope_level, 0, brk, cont, isFunction, funcName); // condition
            if (if_expr != BOOL_TYPE)
            {
                char msg[1024];
                sprintf(msg, "Semantic ERROR:expression must be bool\n");
                log_errors(p->line, msg);
            }
            // if
            Scope *new_scope = new Scope(++level, &scope_level);
            add_scope(new_scope);
            if (p->opr.nops == 2)
            {

                int end = label;
                fprintf(fp, "jz L%d\n", label++);

                begin_compile(p->opr.op[1], *new_scope, 0, brk, cont, isFunction, funcName); // if body

                fprintf(fp, "L%d\n", end);
            }
            else if (p->opr.nops == 3)
            {
                int l2 = label++;
                int end = label;
                fprintf(fp, "jz L%d\n", label++);
                begin_compile(p->opr.op[1], *new_scope, 0, brk, cont, isFunction, funcName); // if body
                fprintf(fp, "jmp L%d\n", l2);                                                           // jump to end of if
                fprintf(fp, "L%d\n", end);
                Scope *new_scope2 = new Scope(++level, &scope_level);
                add_scope(new_scope2);
                begin_compile(p->opr.op[2], *new_scope2, 0, brk, cont, isFunction, funcName); // else body
                fprintf(fp, "L%d\n", l2);                                                                // end of if
            }
            return 0;
            break;
        }
        case FOR:
        {

            Scope *new_scope = new Scope(++level, &scope_level);
            add_scope(new_scope);
            begin_compile(p->opr.op[0], *new_scope, 0, brk, cont, isFunction, funcName); // initialize
            int l1 = label++;
          
            fprintf(fp, "L%d\n", l1);
            int cond = begin_compile(p->opr.op[1], *new_scope, 0, brk, cont, isFunction, funcName); // condition
            if (cond != BOOL_TYPE)
            {
                log_errors(p->line, "Semantic ERROR: Condition must be of a BOOL Value");
            }

            int l2 = label++;
            int connn=label++;
            fprintf(fp, "jz L%d\n", l2);

            begin_compile(p->opr.op[3], *new_scope, 0, l2, connn, isFunction, funcName);    // body
            fprintf(fp, "L%d\n", connn); // jump to inc/dec
            begin_compile(p->opr.op[2], *new_scope, 0, brk, cont, isFunction, funcName); // next iter inc/dec
            fprintf(fp, "jmp L%d\n", l1);                                                           // jump to condition
            fprintf(fp, "L%d\n", l2);                                                               // end of loop
            break;
        }
        case BREAK:
        {
            if (brk == -1)
            {
                log_errors(p->line, "Semantic ERROR: No loop to Break from");

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
            fprintf(fp, "proc %s\n", p->opr.op[0]->id.name);
            Scope *new_scope = new Scope(++level, &scope_level);
            add_scope(new_scope);
            add_func(p->opr.op[0]->id.name, p->opr.op[0]->dec.dataType, vector<Symbol>(), scope_level, p->line); // add function to symbol table
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
                log_errors(p->line, msg);
            }
            return ret_typ;
            break;
        }
        case RETURN:

        {

            int t = begin_compile(p->opr.op[0], scope_level, 0, brk, cont, isFunction, funcName); // return statement
            if (t != VOID_TYPE)
                fprintf(fp, "ret\n");
            fprintf(fp, "endproc\t\n");

            return t;
            break;
        }
        case CONTINUE:
        {
            if (cont == -1)
            {
                log_errors(p->line, "Semantic ERROR: Continue statement not in loop");

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
                log_errors(p->line, "Semantic ERROR: Condition must be of a BOOL value");
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
                log_errors(p->line, "semantic ERROR: Conditions must be of a BOOL Value");
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
                log_errors(p->line, msg);
            }
            fprintf(fp, "neg\n");
            return op;
            break;
        }
        case BITWISE_NOT:
        {
            int op1 = begin_compile(p->opr.op[0], scope_level, 0, brk, cont, isFunction, funcName);

            if (op1 != INT_TYPE)
            {
                char msg[1024];
                sprintf(msg, "Semantic ERROR: cannot apply bitwise to  %s", get_type(op1));
                log_errors(p->line, msg);
            }
            fprintf(fp, "bitwise_not\n");
            return INT_TYPE;
            break;
        }
        case POST_INCREMENT:
        {
            int op = begin_compile(p->opr.op[0], scope_level, 0, brk, cont, isFunction, funcName);
            if (op != INT_TYPE && op != FLOAT_TYPE && op != CHAR_TYPE)
            {
                char msg[1024];
                sprintf(msg, "Semantic ERROR: cannot apply unary increment to %s\n", get_type(op));
                log_errors(p->line, msg);
            }
            fprintf(fp, "post inc\n");
            return op;
            break;
        }
        case POST_DECREMENT:
        {
            int op = begin_compile(p->opr.op[0], scope_level, 0, brk, cont, isFunction, funcName);
            if (op != INT_TYPE && op != FLOAT_TYPE && op != CHAR_TYPE)
            {
                char msg[1024];
                sprintf(msg, "Semantic ERROR: cannot apply unary decrement to %s\n", get_type(op));
                log_errors(p->line, msg);
            }
            fprintf(fp, "post dec\n");
            return op;
            break;
        }
        case PRE_INCREMENT:
        {
            int op = begin_compile(p->opr.op[0], scope_level, 0, brk, cont, isFunction, funcName);
            if (op != INT_TYPE && op != FLOAT_TYPE && op != CHAR_TYPE)
            {
                char msg[1024];
                sprintf(msg, "Semantic ERROR: cannot apply unary increment to %s\n", get_type(op));
                log_errors(p->line, msg);
            }
            fprintf(fp, "pre inc\n");
            return op;
            break;
        }
        case PRE_DECREMENT:
        {
            int op = begin_compile(p->opr.op[0], scope_level, 0, brk, cont, isFunction, funcName);
            if (op != INT_TYPE && op != FLOAT_TYPE && op != CHAR_TYPE)
            {
                char msg[1024];
                sprintf(msg, "Semantic ERROR: cannot apply unary decrement to %s\n", get_type(op));
                log_errors(p->line, msg);
            }
            fprintf(fp, "pre dec\n");
            return op;
            break;
        }

        case '=':
        {
            int ass = begin_compile(p->opr.op[1], scope_level, 0, brk, cont, isFunction, funcName);
            // int op1 = begin_compile(p->opr.op[0]);
            // check if op[0] is dec or id
            
            int t = begin_compile(p->opr.op[0], scope_level, true, brk, cont, isFunction, funcName);
            if (isFunction )
            {

                fix_deafault(funcName, scope_level);
            }
            if (p->opr.op[0]->type == DECLARATION)
            {
               
                // Check if type is int and other is float
                if ((p->opr.op[0]->dec.dataType == INT_TYPE && ass == FLOAT_TYPE))
                {
                    fprintf(fp, "convert from float to int\n");
                }
                else if ((p->opr.op[0]->dec.dataType == FLOAT_TYPE && ass == INT_TYPE))
                {
                    fprintf(fp, "convert from float to int\n");
                }
                else if ((p->opr.op[0]->dec.dataType == CHAR_TYPE && ass == INT_TYPE))
                {
                    fprintf(fp, "convert from int to char\n");
                }

                else if (p->opr.op[0]->dec.dataType != ass)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR: cannot assign %s to %s\n", get_type(ass), get_type(t));
                    log_errors(p->line, msg);
                }
                fprintf(fp, "pop %s\n", p->opr.op[0]->dec.symbol);
                initilize_symbol(p->opr.op[0]->dec.symbol, scope_level);
            }
            else
            {
                // check if identifer is constant
                check_if_const(p->opr.op[0]->id.name, scope_level, p->line);
                if (t != ass)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR: cannot assign %s to %s\n", get_type(ass), get_type(t));
                    log_errors(p->line, msg);
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
                log_errors(p->line, msg);
            }
            switch (p->opr.symbol)
            {
            case '+':
                if (op1 != INT_TYPE && op1 != FLOAT_TYPE && op1 != STRING_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR:cannot add operands of type %s", get_type(op1));
                    log_errors(p->line, msg);
                }
                fprintf(fp, "add\n");
                return op1;
                break;
            case '-':
                if (op1 != INT_TYPE && op1 != FLOAT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR:cannot subtract operands of type %s", get_type(op1));
                    log_errors(p->line, msg);
                }
                fprintf(fp, "sub\n");
                return op1;
                break;
            case '*':
                if (op1 != INT_TYPE && op1 != FLOAT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR:cannot multiply operands of type %s", get_type(op1));
                    log_errors(p->line, msg);
                }
                fprintf(fp, "mul\n");
                return op1;
                break;
            case '/':
                if (op1 != INT_TYPE && op1 != FLOAT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR:cannot divide operands of type %s", get_type(op1));
                    log_errors(p->line, msg);
                }
                fprintf(fp, "div\n");
                return op1;
                break;
            case '%':
                if (op1 != INT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR:cannot mod operands of type %s", get_type(op1));
                    log_errors(p->line, msg);
                }
                fprintf(fp, "mod\n");
                return op1;
                break;
            case '<':
                if (op1 != INT_TYPE && op1 != FLOAT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR:cannot compare operands of type %s", get_type(op1));
                    log_errors(p->line, msg);
                }
                fprintf(fp, "lt\n");
                return BOOL_TYPE;
                break;
            case '>':
                if (op1 != INT_TYPE && op1 != FLOAT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR:cannot compare operands of type %s", get_type(op1));
                    log_errors(p->line, msg);
                }
                fprintf(fp, "gt\n");
                return BOOL_TYPE;
                break;

            case GREATER_EQUAL:
                if (op1 != INT_TYPE && op1 != FLOAT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR:cannot compare operands of type %s", get_type(op1));
                    log_errors(p->line, msg);
                }
                fprintf(fp, "ge\n");
                return BOOL_TYPE;
                break;
            case LESS_EQUAL:
                if (op1 != INT_TYPE && op1 != FLOAT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR:cannot compare operands of type %s", get_type(op1));
                    log_errors(p->line, msg);
                }
                fprintf(fp, "le\n");
                return BOOL_TYPE;
                break;
            case BITWISE_XOR:
                if (op1 != INT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR");
                    log_errors(p->line, msg);
                }
                fprintf(fp, "bitwise_xor\n");
                return INT_TYPE;

                break;
            case BITWISE_AND:
                if (op1 != INT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR");
                    log_errors(p->line, msg);
                }
                fprintf(fp, "bitwise_and\n");
                return INT_TYPE;
                break;
            case BITWISE_OR:
                if (op1 != INT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR");
                    log_errors(p->line, msg);
                }
                fprintf(fp, "bitwise_or\n");
                return INT_TYPE;
                break;

            case SHIFT_LEFT:
                if (op1 != INT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR");
                    log_errors(p->line, msg);
                }
                fprintf(fp, "shift_left\n");
                return INT_TYPE;
                break;
            case SHIFT_RIGHT:
                if (op1 != INT_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR");
                    log_errors(p->line, msg);
                }
                fprintf(fp, "shift_right\n");
                return INT_TYPE;
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
                    log_errors(p->line, msg);
                }
                fprintf(fp, "and\n");
                return op1;
                break;
            case OR:
                if (op1 != BOOL_TYPE)
                {
                    char msg[1024];
                    sprintf(msg, "Semantic ERROR: cannot apply OR to %s\n", get_type(op1));
                    log_errors(p->line, msg);
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
