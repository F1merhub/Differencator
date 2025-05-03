#include "diff.h"

int main() {
    Node *Root = nullptr;

    Errors err = BuildTreeFromFile("example1.txt", &Root);

    if (err == OK) {
        printf("tree was parsed\n");
    } else {
        printf("wrong format of file\n");
        return err;
    }

    // TreeDumpDot(Root);
    Node *DifRoot = Diff(Root);
    TreeDumpDot(DifRoot);

    double result = Eval(DifRoot);
    printf("Производная функции в точке %lg равна: %lg\n", VAR_VALUE, result);

    FreeTree(&Root);
    FreeTree(&DifRoot);
    return OK;
}
