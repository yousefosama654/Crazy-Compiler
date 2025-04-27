#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <string.h>
#include <string>
#include <stdarg.h>

#include <iostream>

#include <vector>
#include <map>

using namespace std;

/* Add All Types and structs Here*/
#define SIZEOFNODE ((char *)&p->con - (char *)p)

#define ASSIGNMENT 1
#define STATEMENT_LIST 2
#define COMMA 3
#define CALL 4
typedef enum
{
    CONSTANT,
    IDENTIFIER,
    OPERATION,
    DECLARATION
} NodeType;

struct ValueType
{
    union
    {
        int intVal;
        double floatVal;
        bool boolVal;
        char *strVal;
    };
};

/* constants */
typedef struct
{
    ValueType value; /* value */
    int dataType;    /* type */

} ConstantNode;

/* variables */
typedef struct
{
    char *name;    /* name */
    int dataType;  /* type */
    int qualifier; /* qualifier*/
} IdentifierNode;

/* operators */
typedef struct
{
    int symbol;                /* symbol */
    int nops;                  /* number of operands */
    struct NodeTypeTag *op[1]; /* expandable */
} OperationNode;

typedef struct
{
    char *symbol;
    int dataType; /* type */
    /* symbol */
    int qualifier; /* qualifier */
} DeclerationNode;

typedef struct NodeTypeTag
{
    NodeType type; /* type of Node */
    int line;     /* line number */
    union
    {
        ConstantNode con;    /* constants */
        IdentifierNode id;   /* identifiers */
        OperationNode opr;   /* operators */
        DeclerationNode dec; /* declarations */
    };
} Node;

extern void free_node(Node *p);
extern void yyerror(const char *emsg);

typedef struct Scope
{
    Scope *parent; // parent scope
    int level;     // scope level
    Scope(int l, Scope *p) : level(l), parent(p) {};
} Scope;
typedef struct Symbol
{
    std::string name;   // variable name
    int type;           // DataType:     {int, float, ..}
    int Qualfier;       // SymbolType:   {variable, constant,..}
    int scope_level;    // Scope:        {global:0, local:1}
    bool used;          // Used:         {true, false}
    bool isInitialized; // Initialized:  {true, false}
    bool isFunction;    // isFunction:   {true, false}
    Symbol() : name(""), type(0), Qualfier(0), scope_level(0) {}
} Symbol;

typedef struct SymbolFunction
{
    std::string name; // function name
    int returnType;   // DataType:     {int, float, ..}
    bool used;
    std::vector<Symbol> argTypes; // argument types
} SymbolFunction;

void print_symbol_table();
int begin_compile(Node *p, Scope, bool flag = false, int brk = -1, int cont = -1, int isFunction = 0, char *funcName = NULL);
#endif