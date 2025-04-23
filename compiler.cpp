#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include "compiler.h"
#include <sstream>
#include <fstream>
using namespace std;


void log_errors(int l, const char *msg)
{

    fprintf(stderr, "Line %d: %s\n", l, msg);
    ifstream input("input.txt");
    if (!input.is_open())
    {
        fprintf(stderr, "Warning: couldn't read input.txt for context.\n");
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
        fprintf(stderr, "Last valid line [%d]: %s\n", lastValidLineNumber, lastValidLine.c_str());
        const char *str = lastValidLine.c_str();
        size_t len = strlen(str);
        if (len >= 2 && str[len - 2] != ';')
        {        
            fprintf(stderr, "Note: maybe a semicolon is missing\n");
        }
    }

}