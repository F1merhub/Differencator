#include "diff.h"

//TODO Тейлор, рекурсивный спуск, Латех файл, define, функции, свертка констант.

int main() {
    Node *Root = nullptr;
    Errors err = BuildTreeFromFile("./expression.txt", &Root);

    if (err == OK) {
        printf("tree was parsed\n");
    } else {
        printf("wrong format of file\n");
        return err;
    }
    // Node* node = (Node*)calloc(1, sizeof(Node));

    // RecognizeNodeType("arccos", &(node->type), &(node->value));
    // Node *DifRoot = Diff(Root);
    // TreeDumpDot(Root);
    // printf("%d\n%d\n", node->type, node->value);
    // TreeDumpDot(DifRoot);
    Node *SimpleRoot = SimplifyTree(Root);
    TreeDumpDot(SimpleRoot);
    // double result = Eval(Root);
    // printf("\n%lg\n", result);
    // double result2 = Eval(DifRoot);
    // printf("Производная функции в точке %lg равна: %lg\n", VAR_VALUE, result);
    FreeTree(&SimpleRoot);
    FreeTree(&Root);
    // FreeTree(&DifRoot);
    return OK;
}
