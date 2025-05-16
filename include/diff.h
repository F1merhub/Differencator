#ifndef _DIFF_H
#define _DIFF_H

#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

const double VAR_VALUE = M_PI;
const size_t DUMP_BUFFER_SIZE = 50000;
const int NAME_SIZE = 10;

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
    EXP,
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
Errors OpFuncValue(enum NodeType type, int value, char* str);
#endif
