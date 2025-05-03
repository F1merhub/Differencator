#include "diff.h"




int main() {
    Node *Root = nullptr;
    CreateNode(&Root, "-94.9", nullptr);
    // printf("%d", Root->value);
    printf("\n%d", Root->value.op);
    printf("\n%d", Root->type);
    // printf("\n%p", Root);
    CreateNode(&(Root->left), "5", Root);
    CreateNode(&(Root->right), "10", Root);
    TreeDumpDot(Root);
    FreeTree(&Root);
    return 0;
}

Errors FreeTree(Node **node) {
    assert(node);

    if (*node == nullptr)
    {
        return OK;
    }

    FreeTree(&((*node)->left));
    FreeTree(&((*node)->right));

    // free((*node)->value);
    free(*node);
    *node = nullptr;

    return OK;
}

Errors CreateNode(Node **dest, const char *str, Node *parent) { // NOTE освободить память после использования
    assert(dest != nullptr && str != nullptr);

    Node* node = (Node*)calloc(1, sizeof(Node));
    if (node == nullptr)
        return MEMORY_ALLOCATION_ERROR;

    RecognizeNodeType(str, &(node->type), &(node->value));
    node->left = nullptr;
    node->right = nullptr;
    node->parent = parent;
    // printf("%d", node->type);
    // printf("%lg", node->value.num);
    *dest = node;
    // printf("\n%p", node);
    return OK;
}

Errors RecognizeNodeType(const char *str, NodeType* type, NodeValue* value) {
    assert(str != nullptr && type != nullptr && value != nullptr);

    Decoder op_array[] = {
        {"+",     ADD},
        {"-",     SUB},
        {"*",     MUL},
        {"/",     DIV},
        {"^",     POW},
    };

    // Decoder func_array[] = { // TODO func
    //     {"sin",   SIN},
    //     {"cos",   COS},
    // };

    char* endptr = nullptr;
    double num = strtod(str, &endptr);

    if (*endptr == '\0') { // если число
        *type = NUM;
        value->num = num;
        return OK;
    }
    else if (strlen(str) == 1) {

        if (isalpha(str[0])) {
            *type = VAR;
            return OK;
        }
        else {
            int op_count = sizeof(op_array) / sizeof(op_array[0]);
            for (int i = 0; i < op_count; i++) {
                if (strcmp(str, op_array[i].name) == 0) {
                    *type = OP;
                    value->op = (enum Op)op_array[i].code;
                    return OK;
                }
            }
            return INVALID_TYPE;
        }
    }
    assert(0); // TODO func case

}
