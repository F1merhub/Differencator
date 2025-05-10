#include "diff.h"

//TODO Тейлор, рекурсивный спуск, Латех файл, define, функции, юнит тесты, makefile, свертка констант.

int main() {
    Node *Root = nullptr;

    Errors err = BuildTreeFromFile("./expression.txt", &Root); //

    if (err == OK) {
        printf("tree was parsed\n");
    } else {
        printf("wrong format of file\n");
        return err;
    }

    Node *DifRoot = Diff(Root);

    TreeDumpDot(Root);
    // TreeDumpDot(DifRoot);

    double result = Eval(DifRoot);
    printf("Производная функции в точке %lg равна: %lg\n", VAR_VALUE, result);

    FreeTree(&Root);
    FreeTree(&DifRoot);
    return OK;
}
