#include "diff.h"
#include "dsl.h"

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

Errors BuildTreeFromPrefix(const char** str, Node** node, Node* parent) {
    *str = SkipWhitespace(*str);

    if ((*str)[0] == '\0') return OK;

    if ((*str)[0] == '(') {
        (*str)++;
        *str = SkipWhitespace(*str);

        char token[32] = {0};
        int i = 0;
        while ((*str)[0] != '\0' && !isspace((*str)[0]) && (*str)[0] != '(' && (*str)[0] != ')') {
            token[i++] = (*str)[0];
            (*str)++;
            if (i >= 31) return INVALID_FORMAT;
        }

        if (i == 0) return INVALID_FORMAT;

        // Проверка на специальные константы перед созданием узла
        if (strcmp(token, "e") == 0) {
            *node = NewNode(NUM, NodeValue{.num = M_E}, nullptr, nullptr);
            (*node)->parent = parent;
        }
        else if (strcmp(token, "pi") == 0) {
            *node = NewNode(NUM, NodeValue{.num = M_PI}, nullptr, nullptr);
            (*node)->parent = parent;
        }
        else {
            Errors err = CreateNode(node, token, parent);  // Объявляем err здесь
            if (err != OK) return err;
        }

        if ((*node)->type == FUNC) {
            *str = SkipWhitespace(*str);
            Errors err = BuildTreeFromPrefix(str, &(*node)->left, *node);  // Объявляем err
            if (err != OK) return err;
            (*node)->right = NULL;
        }
        else {
            *str = SkipWhitespace(*str);
            Errors err = BuildTreeFromPrefix(str, &(*node)->left, *node);  // Объявляем err
            if (err != OK) return err;

            *str = SkipWhitespace(*str);
            err = BuildTreeFromPrefix(str, &(*node)->right, *node);  // Повторно используем err
            if (err != OK) return err;
        }

        *str = SkipWhitespace(*str);
        if ((*str)[0] != ')') return INVALID_FORMAT;
        (*str)++;
    }
    else {
        char token[32] = {0};
        int i = 0;
        while ((*str)[0] != '\0' && !isspace((*str)[0]) && (*str)[0] != '(' && (*str)[0] != ')') {
            token[i++] = (*str)[0];
            (*str)++;
            if (i >= 31) return INVALID_FORMAT;
        }

        if (i == 0) return INVALID_FORMAT;

        // Проверка на специальные константы для терминальных узлов
        if (strcmp(token, "e") == 0) {
            *node = NewNode(NUM, NodeValue{.num = M_E}, nullptr, nullptr);
            (*node)->parent = parent;
        }
        else if (strcmp(token, "pi") == 0) {
            *node = NewNode(NUM, NodeValue{.num = M_PI}, nullptr, nullptr);
            (*node)->parent = parent;
        }
        else {
            Errors err = CreateNode(node, token, parent);  // Объявляем err
            if (err != OK) return err;
        }
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

const char* OpFuncValue(enum NodeType type, int value) {
    assert((value >= 0) && ((type == OP) || (type == FUNC)));

    Decoder op_array[] = {
        {"+",     ADD},
        {"-",     SUB},
        {"*",     MUL},
        {"/",     DIV},
        {"^",     POW},
    };

    Decoder func_array[] = { // TODO остальное
        {"sin",     ADD},
        {"cos",     SUB},
    };

    int op_count = sizeof(op_array) / sizeof(op_array[0]);
    int func_count = sizeof(func_array) / sizeof(func_array[0]);

    assert(value < op_count && value < func_count);

    switch(type) {
        case OP:
            return op_array[value].name;
            break;
        case FUNC:
            return func_array[value].name;
            break;
    }
    return {0};
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

    Decoder func_array[] = { // TODO остальное
        {"sin",     ADD},
        {"cos",     SUB},
    };


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
    else {
        int func_count = sizeof(func_array) / sizeof(func_array[0]);
        for (int i = 0; i < func_count; i++) {
            if (strcmp(str, func_array[i].name) == 0) {
                *type = FUNC;
                value->func = (enum Func)op_array[i].code;
                return OK;
            }
        }
        return INVALID_TYPE;
    }

    return OK;
}

double Eval(Node *node)
{
    assert(node != nullptr);

    if (node->type == NUM) return node->value.num;
    if (node->type == VAR) return VAR_VALUE;
    if (node->type == OP)
    {
        switch (node->value.op)
        {
            case ADD: return Eval(node->left) + Eval(node->right);
            case SUB: return Eval(node->left) - Eval(node->right);
            case MUL: return Eval(node->left) * Eval(node->right);
            case DIV: return Eval(node->left) / Eval(node->right);
            case POW: return pow(Eval(node->left), Eval(node->right));

            default:
                assert(0);
                return INVALID_TYPE;
        }
    }
   if (node->type == FUNC) {
        switch(node->value.func) {
            case SIN: return sin(Eval(node->left));
            case COS: return cos(Eval(node->left));
        }
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

Node* Diff(Node *node) {
    assert(node);
    if (node->type == NUM)
        return NewNode(NUM, NodeValue {.num = 0}, nullptr, nullptr);

    if (node->type == VAR)
        return NewNode(NUM, NodeValue {.num = 1}, nullptr, nullptr);

    if (node->type == OP)
    {
        switch (node->value.op)
        {
            case ADD:
                return NewNode(OP, NodeValue {.op = ADD}, DIF_LEFT, DIF_RIGHT);
            case SUB:
                return NewNode(OP, NodeValue {.op = SUB}, DIF_LEFT, DIF_RIGHT);
            case MUL:
                return NewNode(OP, NodeValue {.op = ADD},
                       NewNode(OP, NodeValue {.op = MUL}, DIF_LEFT, COPY_RIGHT),
                       NewNode(OP, NodeValue {.op = MUL}, COPY_LEFT, DIF_RIGHT));
            case DIV:
                return  NewNode(OP, NodeValue {.op = DIV},
                            NewNode(OP, NodeValue {.op = SUB},
                                NewNode(OP, NodeValue {.op = MUL}, DIF_LEFT, COPY_RIGHT),
                                NewNode(OP, NodeValue {.op = MUL}, COPY_LEFT, DIF_RIGHT)),
                            NewNode(OP, NodeValue {.op = MUL}, COPY_RIGHT, COPY_RIGHT));
            case POW:
                return NewNode(OP, NodeValue {.op = ADD},
                       NewNode(OP, NodeValue {.op = MUL},
                           NewNode(OP, NodeValue {.op = MUL},
                               COPY_RIGHT,
                               NewNode(OP, NodeValue {.op = POW},
                                   COPY_LEFT,
                                   NewNode(OP, NodeValue {.op = SUB},
                                       COPY_RIGHT,
                                       NewNode(NUM, NodeValue {.num = 1}, nullptr, nullptr)))),
                           DIF_LEFT),
                       NewNode(OP, NodeValue {.op = MUL},
                           NewNode(OP, NodeValue {.op = MUL},
                               NewNode(OP, NodeValue {.op = POW},
                                   COPY_LEFT,
                                   COPY_RIGHT),
                               NewNode(FUNC, NodeValue {.func = LN}, COPY_LEFT, nullptr)),
                           DIF_RIGHT));
            default:
                return nullptr;
        }
    }
    if (node->type == FUNC)
    {
        switch(node->value.func) {
            case SIN:
                return  NewNode(OP, NodeValue {.op = MUL},
                            NewNode(FUNC, NodeValue {.func = COS}, COPY_LEFT, nullptr),
                            DIF_LEFT);
            // case COS:
            default:
                assert(0);
        }
    }

    return nullptr;
}
