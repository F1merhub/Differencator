#include "diff.h"

//TODO Тейлор, рекурсивный спуск, Латех файл, define, функции, юнит тесты, makefile, свертка констант.

int main() {
    Node *Root = nullptr;

    char* str = (char*)calloc(10, sizeof(char));
    OpFuncValue((NodeType)2, 5, str);
    
    return OK;
}
