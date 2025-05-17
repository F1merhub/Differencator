#ifndef _DIFF_H
#define _DIFF_H

#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "color.h"

extern double VAR_VALUE;
const size_t DUMP_BUFFER_SIZE = 5000000;
const int NAME_SIZE = 15;
const double EPSILON = 0.001;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum Errors
{
    OK                          = 0,
    INVALID_ARGUMENT            = 1,
    INVALID_FORMAT              = 2,
    MEMORY_ALLOCATION_ERROR     = 3,
    FILE_NOT_OPEN               = 4,
    INVALID_OPERATION           = 5,
    INVALID_TYPE                = 6,
    COMMAND_ERROR               = 7
};

enum NodeType
{
    OP,
    NUM,
    FUNC,
    VAR,
};

enum Op
{
    ADD,
    SUB,
    MUL,
    DIV,
    POW,
};

enum Func // TODO остальное
{
    SIN,
    COS,
    TAN,
    COT,
    LN,
    ARCSIN,
    ARCCOS,
    ARCTAN,
    ARCCOT
};

enum Keys
{
    KEY_1 = 49,
    KEY_2 = 50,
    KEY_3 = 51,
    KEY_4 = 52,
    KEY_5 = 53,
    KEY_COUNT
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

union NodeValue
{
    double num;
    Op op;
    Func func;
};

typedef struct DiffDecoder
{
    const char* name;
    int code;
} Decoder;

typedef struct Node
{
    NodeType type;
    NodeValue value;

    struct Node* left;
    struct Node* right;
    struct Node* parent;
} Node;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Errors FreeTree(Node **node);
Errors CreateNode(Node **node, const char *str, Node *parent);
Errors RecognizeNodeType(const char *str, NodeType* type, NodeValue* value);
Errors TreeDumpDot(Node* Root);
int GenerateGraph(Node *node, char* buffer, int* buffer_len, const size_t BUFFER_SIZE);
Errors ReadFileToBuffer(const char* filename, char** buffer);
const char* SkipWhitespace(const char* str);
Errors BuildTreeFromPrefix(const char** str, Node** node, Node* parent);
Errors BuildTreeFromFile(const char* filename, Node** root);
double Eval(Node *node);
Node* Diff(Node *node);
Node* NewNode(NodeType type, NodeValue value, Node* left, Node* right);
Node* CopyTree(Node *root);
Errors OpFuncValue(enum NodeType type, int value, char* str);
Node* SimplifyTree(Node *node);
int CompareDoubles(double x, double y);
int GetMode(int mode_count);
Errors Menu();
Errors DumpMode();
Errors EvalMode();
Errors DiffMode();
unsigned long long factorial(int n);
Node* MultiplyDiff(Node *node, int order);
Errors ExpandInTaylorSeries(int order);
Errors TaylorMode();
Errors ProcessToken(const char** str, Node** node, Node* parent);

#endif
