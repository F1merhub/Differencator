#include "diff.h"

Errors ReadFileToBuffer(const char* filename, char** buffer) {
    FILE* file = fopen(filename, "r");
    if (!file)
        return FILE_NOT_OPEN;

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    *buffer = (char*)calloc(file_size + 1, sizeof(char));
    if (!*buffer) {
        fclose(file);
        return MEMORY_ALLOCATION_ERROR;
    }

    fread(*buffer, 1, file_size, file);
    (*buffer)[file_size] = '\0';

    fclose(file);
    return OK;
}

const char* SkipWhitespace(const char* str) {
    while (isspace(*str)) str++;
    return str;
}


Errors ProcessToken(const char** str, Node** node, Node* parent) {
    char token[32] = {0};
    int i = 0;

    while ((*str)[0] != '\0' && !isspace((*str)[0]) && (*str)[0] != '(' && (*str)[0] != ')') {
        token[i++] = (*str)[0];
        (*str)++;
        if (i >= 31) return INVALID_FORMAT;
    }

    if (i == 0) return INVALID_FORMAT;

    if (strcmp(token, "e") == 0) {
        *node = NewNode(NUM, NodeValue{.num = M_E}, nullptr, nullptr);
    } else if (strcmp(token, "pi") == 0) {
        *node = NewNode(NUM, NodeValue{.num = M_PI}, nullptr, nullptr);
    } else {
        Errors err = CreateNode(node, token, parent);
        if (err != OK) return err;
    }

    (*node)->parent = parent;
    return OK;
}

Errors BuildTreeFromPrefix(const char** str, Node** node, Node* parent) {
    *str = SkipWhitespace(*str);

    if ((*str)[0] == '\0') return OK;

    if ((*str)[0] == '(') {
        (*str)++;
        *str = SkipWhitespace(*str);

        Errors error = ProcessToken(str, node, parent);
        if (error != OK) return error;

        if ((*node)->type == FUNC) { // обработка одного арг функции
            *str = SkipWhitespace(*str);
            Errors err = BuildTreeFromPrefix(str, &(*node)->left, *node);
            if (err != OK) return err;
            (*node)->right = NULL;
        }
        else {                       // обработка двух арг оператора
            *str = SkipWhitespace(*str);
            Errors err = BuildTreeFromPrefix(str, &(*node)->left, *node);
            if (err != OK) return err;

            *str = SkipWhitespace(*str);
            err = BuildTreeFromPrefix(str, &(*node)->right, *node);
            if (err != OK) return err;
        }

        *str = SkipWhitespace(*str);
        if ((*str)[0] != ')') return INVALID_FORMAT;
        (*str)++;
    }
    else {
        Errors err = ProcessToken(str, node, parent);
        if (err != OK) return err;
    }

    return OK;
}

Errors BuildTreeFromFile(const char* filename, Node** root) {
    char* buffer = NULL;
    Errors err = ReadFileToBuffer(filename, &buffer);
    if (err != OK) return err;

    const char* ptr = buffer;
    err = BuildTreeFromPrefix(&ptr, root, NULL);

    free(buffer);
    return err;
}

Errors FreeTree(Node **node) {
    assert(node);

    if (*node == nullptr)
    {
        return OK;
    }

    FreeTree(&((*node)->left));
    FreeTree(&((*node)->right));

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
    *dest = node;

    return OK;
}

Errors OpFuncValue(enum NodeType type, int value, char* str) {
    assert(str != nullptr);

    Decoder op_array[] = {
        {"+",     ADD},
        {"-",     SUB},
        {"*",     MUL},
        {"/",     DIV},
        {"^",     POW},
    };

    Decoder func_array[] = { // TODO остальное
        {"sin",        SIN},
        {"cos",        COS},
        {"tan",        TAN},
        {"cot",        COT},
        {"ln",         LN},
        {"arcsin",     ARCSIN},
        {"arccos",     ARCCOS},
        {"arctan",     ARCTAN},
        {"arccot",     ARCCOT},
    };

    int op_count = sizeof(op_array) / sizeof(op_array[0]);
    int func_count = sizeof(func_array) / sizeof(func_array[0]);

    assert((type == OP && value < op_count) || (type == FUNC && value < func_count));

    switch(type) {
        case OP:
            strcpy(str, op_array[value].name);
            break;
        case FUNC:
            strcpy(str, func_array[value].name);
            // printf("%d", value);
            break;
        case NUM:
            assert(0);
        case VAR:
            assert(0);
        default:
            assert(0 && "Error text");
            break;
     }
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

    Decoder func_array[] = {
        {"sin",        SIN},
        {"cos",        COS},
        {"tan",        TAN},
        {"cot",        COT},
        {"ln",         LN},
        {"arcsin",     ARCSIN},
        {"arccos",     ARCCOS},
        {"arctan",     ARCTAN},
        {"arccot",     ARCCOT},
    };


    char* endptr = nullptr;
    double num = strtod(str, &endptr);

    if (*endptr == '\0') {
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
    else {
        int func_count = sizeof(func_array) / sizeof(func_array[0]);
        for (int i = 0; i < func_count; i++) {
            if (strcmp(str, func_array[i].name) == 0) {
                *type = FUNC;
                value->func = (enum Func)func_array[i].code;
                // printf("%d", op_array[i].code);
                return OK;
            }
        }
        return INVALID_TYPE;
    }

    return OK;
}

Node* NewNode(NodeType type, NodeValue value, Node* left, Node* right) {
    Node* node = (Node*)calloc(1, sizeof(Node));
    if (node == nullptr)
        return nullptr;

    node->type = type;
    node->value = value;

    node->left = left;
    node->right = right;

    if (left)  left->parent  = node;
    if (right) right->parent = node;

    return node;
}

Node* CopyTree(Node *root) {
    assert(root != nullptr);

    Node* node = NewNode(root->type, root->value, nullptr, nullptr);

    if (root->left != nullptr)
        node->left  = CopyTree(root->left);
    if (root->right != nullptr)
        node->right = CopyTree(root->right);

    return node;
}
