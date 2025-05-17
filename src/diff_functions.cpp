#include "diff.h"
#include "dsl.h"

double VAR_VALUE = 0;

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

// Выносим логику обработки токена в отдельную функцию
Errors ProcessToken(const char** str, Node** node, Node* parent) {
    char token[32] = {0};
    int i = 0;

    // Извлечение токена
    while ((*str)[0] != '\0' && !isspace((*str)[0]) && (*str)[0] != '(' && (*str)[0] != ')') {
        token[i++] = (*str)[0];
        (*str)++;
        if (i >= 31) return INVALID_FORMAT;
    }

    if (i == 0) return INVALID_FORMAT;

    // Создание узла
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
                return INVALID_TYPE;
        }
    }
   if (node->type == FUNC) {
        switch(node->value.func) {
            case SIN: return sin(Eval(node->left));
            case COS: return cos(Eval(node->left));
            case TAN: return tan(Eval(node->left));
            case COT: return (1 / tan(Eval(node->left)));
            case LN:  return log(Eval(node->left));
            case ARCSIN: return asin(Eval(node->left));
            case ARCCOS: return acos(Eval(node->left));
            case ARCTAN: return atan(Eval(node->left));
            case ARCCOT: return ((M_PI / 2) - atan(Eval(node->left)));
            default: assert(0); // NOTE могу добавить другие функции
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

    if (node->type == FUNC) {
        switch(node->value.func) {
            case SIN:
                return NewNode(OP, NodeValue {.op = MUL},
                       NewNode(FUNC, NodeValue {.func = COS}, COPY_LEFT, nullptr),
                       DIF_LEFT);

            case COS:
                return NewNode(OP, NodeValue {.op = MUL},
                       NewNode(OP, NodeValue {.op = MUL},
                           NewNode(NUM, NodeValue {.num = -1}, nullptr, nullptr),
                           NewNode(FUNC, NodeValue {.func = SIN}, COPY_LEFT, nullptr)),
                       DIF_LEFT);

            case TAN:
                return NewNode(OP, NodeValue {.op = MUL},
                       NewNode(OP, NodeValue {.op = DIV},
                           NewNode(NUM, NodeValue {.num = 1}, nullptr, nullptr),
                           NewNode(OP, NodeValue {.op = POW},
                               NewNode(FUNC, NodeValue {.func = COS}, COPY_LEFT, nullptr),
                               NewNode(NUM, NodeValue {.num = 2}, nullptr, nullptr))),
                       DIF_LEFT);

            case COT:
                return NewNode(OP, NodeValue {.op = MUL},
                       NewNode(OP, NodeValue {.op = MUL},
                           NewNode(NUM, NodeValue {.num = -1}, nullptr, nullptr),
                           NewNode(OP, NodeValue {.op = DIV},
                               NewNode(NUM, NodeValue {.num = 1}, nullptr, nullptr),
                               NewNode(OP, NodeValue {.op = POW},
                                   NewNode(FUNC, NodeValue {.func = SIN}, COPY_LEFT, nullptr),
                                   NewNode(NUM, NodeValue {.num = 2}, nullptr, nullptr)))),
                       DIF_LEFT);

            case LN:
                return NewNode(OP, NodeValue {.op = MUL},
                       NewNode(OP, NodeValue {.op = DIV},
                           NewNode(NUM, NodeValue {.num = 1}, nullptr, nullptr),
                           COPY_LEFT),
                       DIF_LEFT);

            case ARCSIN:
                return NewNode(OP, NodeValue {.op = MUL},
                    NewNode(OP, NodeValue {.op = DIV},
                        NewNode(NUM, NodeValue {.num = 1}, nullptr, nullptr),
                        NewNode(OP, NodeValue {.op = POW},
                            NewNode(OP, NodeValue {.op = SUB},
                                NewNode(NUM, NodeValue {.num = 1}, nullptr, nullptr),
                                NewNode(OP, NodeValue {.op = POW},
                                    COPY_LEFT,
                                    NewNode(NUM, NodeValue {.num = 2}, nullptr, nullptr))),
                            NewNode(NUM, NodeValue {.num = 0.5}, nullptr, nullptr))),
                    DIF_LEFT);

            case ARCCOS:
                return NewNode(OP, NodeValue {.op = MUL},
                    NewNode(OP, NodeValue {.op = MUL},
                        NewNode(NUM, NodeValue {.num = -1}, nullptr, nullptr),
                        NewNode(OP, NodeValue {.op = DIV},
                            NewNode(NUM, NodeValue {.num = 1}, nullptr, nullptr),
                            NewNode(OP, NodeValue {.op = POW},
                                NewNode(OP, NodeValue {.op = SUB},
                                    NewNode(NUM, NodeValue {.num = 1}, nullptr, nullptr),
                                    NewNode(OP, NodeValue {.op = POW},
                                        COPY_LEFT,
                                        NewNode(NUM, NodeValue {.num = 2}, nullptr, nullptr))),
                                NewNode(NUM, NodeValue {.num = 0.5}, nullptr, nullptr)))),
                    DIF_LEFT);

            case ARCTAN:
                return NewNode(OP, NodeValue {.op = MUL},
                    NewNode(OP, NodeValue {.op = DIV},
                        NewNode(NUM, NodeValue {.num = 1}, nullptr, nullptr),
                        NewNode(OP, NodeValue {.op = ADD},
                            NewNode(NUM, NodeValue {.num = 1}, nullptr, nullptr),
                            NewNode(OP, NodeValue {.op = POW},
                                COPY_LEFT,
                                NewNode(NUM, NodeValue {.num = 2}, nullptr, nullptr)))),
                    DIF_LEFT);

            case ARCCOT:
                return NewNode(OP, NodeValue {.op = MUL},
                    NewNode(OP, NodeValue {.op = MUL},
                        NewNode(NUM, NodeValue {.num = -1}, nullptr, nullptr),
                        NewNode(OP, NodeValue {.op = DIV},
                            NewNode(NUM, NodeValue {.num = 1}, nullptr, nullptr),
                            NewNode(OP, NodeValue {.op = ADD},
                                NewNode(NUM, NodeValue {.num = 1}, nullptr, nullptr),
                                NewNode(OP, NodeValue {.op = POW},
                                    COPY_LEFT,
                                    NewNode(NUM, NodeValue {.num = 2}, nullptr, nullptr))))),
                    DIF_LEFT);

            default:
                return nullptr;
        }
    }

    return nullptr;
}

/*
    если вернул nullptr - недопустимое выражение
    обработать в main
*/

Node* SimplifyTree(Node *node) {

    if (node->type == OP && ((node->left == nullptr) || (node->right == nullptr)))
        return nullptr;
    else if (node->type == FUNC && node->left == nullptr)
        return nullptr;

    if (node->type == NUM)
        return NewNode(NUM, node->value, nullptr, nullptr);
    else if (node->type == VAR)
        return NewNode(VAR, node->value, nullptr, nullptr);

    else if (node->type == OP) {
        Node* temp = NewNode(OP, NodeValue {.op = node->value.op}, SimplifyTree(node->left), SimplifyTree(node->right));
        if (temp->left == nullptr || temp->right == nullptr) {
            FreeTree(&temp);
            return nullptr;
        }

        if (node->value.op == ADD || node->value.op == SUB) { // СУММА РАЗНОСТЬ
            if (temp->left->type == NUM && temp->right->type == NUM) { // Если две константы
                    if (temp->value.op == ADD) {
                        Node* temp_num = NewNode(NUM, NodeValue {.num = temp->left->value.num + temp->right->value.num}, nullptr, nullptr);
                        FreeTree(&temp);
                        return temp_num;
                    }
                    else if (temp->value.op == SUB) {
                        Node* temp_num = NewNode(NUM, NodeValue {.num = temp->left->value.num - temp->right->value.num}, nullptr, nullptr);
                        FreeTree(&temp);
                        return temp_num;
                    }
                    else assert(0);
            }
            else if (temp->left->type == NUM && CompareDoubles(temp->left->value.num, 0)) { // сумма, разность с нулем
                Node* temp_node = temp->right;
                free(temp->left);
                free(temp);
                return temp_node;
            }
            else if (temp->right->type == NUM && CompareDoubles(temp->right->value.num, 0)) {
                Node* temp_node = temp->left;
                free(temp->right);
                free(temp);
                return temp_node;
            }
            else
                return temp;
        }

        else if (node->value.op == MUL || node->value.op == DIV) { // УМНОЖЕНИЕ ДЕЛЕНИЕ

            //умножение на ноль
            if (temp->value.op == MUL &&
                ((temp->left->type == NUM && CompareDoubles(temp->left->value.num, 0)) ||
                (temp->right->type == NUM && CompareDoubles(temp->right->value.num, 0)))) {
                FreeTree(&temp);
                return NewNode(NUM, NodeValue {.num = 0}, nullptr, nullptr);
            }

            // умножение на 1 возвращает другой узел
            else if (temp->value.op == MUL && temp->left->type == NUM && CompareDoubles(temp->left->value.num, 1)) {
                Node* temp_node = temp->right;
                free(temp->left);
                free(temp);
                return temp_node;
            }
            else if (temp->value.op == MUL && temp->right->type == NUM && CompareDoubles(temp->right->value.num, 1)) {
                Node* temp_node = temp->left;
                free(temp->right);
                free(temp);
                return temp_node;
            }

            // Деление на 1 возвращает числитель
            else if (temp->value.op == DIV && temp->right->type == NUM && CompareDoubles(temp->right->value.num, 1)) {
                Node* temp_node = temp->left;
                free(temp->right);
                free(temp);
                return temp_node;
            }

            // Если оба операнда числа - вычисляем результат.
            else if (temp->left->type == NUM && temp->right->type == NUM) {
                Node* temp_num;
                if (temp->value.op == MUL)
                    temp_num = NewNode(NUM, NodeValue {.num = temp->left->value.num * temp->right->value.num}, nullptr, nullptr);
                else if (temp->value.op == DIV) {
                    if (CompareDoubles(temp->right->value.num, 0)) {
                        FreeTree(&temp);
                        return nullptr; // случай деления на ноль
                    }
                    temp_num = NewNode(NUM, NodeValue {.num = temp->left->value.num / temp->right->value.num}, nullptr, nullptr);
                }
                else assert(0);

                FreeTree(&temp);
                return temp_num;
            }
            else
                return temp;
        }
        else if (node->value.op == POW) {

            // возвести в 0
            if (temp->right->type == NUM && CompareDoubles(temp->right->value.num, 0)) {
                FreeTree(&temp);
                return NewNode(NUM, NodeValue {.num = 1}, nullptr, nullptr);
            }
            // возвести в 1
            else if (temp->right->type == NUM && CompareDoubles(temp->right->value.num, 1)) {
                Node* left_child = temp->left;
                free(temp->right);
                free(temp);
                return left_child;
            }
            // 0 возвестив любую
            else if (temp->left->type == NUM && CompareDoubles(temp->left->value.num, 0)) {
                FreeTree(&temp);
                return NewNode(NUM, NodeValue {.num = 0}, nullptr, nullptr);
            }
            // 1 возвести в любую
            else if (temp->left->type == NUM && CompareDoubles(temp->left->value.num, 1)) {
                FreeTree(&temp);
                return NewNode(NUM, NodeValue {.num = 1}, nullptr, nullptr);
            }
            // посчитать константу
            else if (temp->left->type == NUM && temp->right->type == NUM) {
                double result = 1;
                for (int i = 0; i < temp->right->value.num; ++i) {
                    result *= temp->left->value.num;
                }
                FreeTree(&temp);
                return NewNode(NUM, NodeValue {.num = result}, nullptr, nullptr);
            }
            else
                return temp;
        }

        else
            assert(0);
    }
    else if (node->type == FUNC) {
        Node* temp = NewNode(node->type, node->value, SimplifyTree(node->left), nullptr);
        if (temp->left == nullptr) {
            FreeTree(&temp);
            return nullptr;
        }

        else if (temp->left->type == NUM) {
            double arg = temp->left->value.num;
            double result = 0;
            int flag = 1;
            switch (node->value.func) {
                case SIN:
                    result = sin(arg);
                    break;
                case COS:
                    result = cos(arg);
                    break;
                case TAN:
                    result = tan(arg);
                    break;
                case COT:
                    result = 1 / tan(arg);
                    break;
                case LN:
                    if (arg > 0) {
                        result = log(arg);
                    } else {
                        flag = 0;
                    }
                    break;
                case ARCSIN:
                    if (arg >= -1 && arg <= 1) {
                        result = asin(arg);
                    } else {
                        flag = 0;
                    }
                    break;
                case ARCCOS:
                    if (arg >= -1 && arg <= 1) {
                        result = acos(arg);
                    } else {
                        flag = 0;
                    }
                    break;
                case ARCTAN:
                    result = atan(arg);
                    break;
                case ARCCOT:
                    result = (M_PI / 2) - atan(arg);
                    break;
                default:
                    flag = false;
            }
            // NOTE как лучше передавать ошибку из рекурсивной функции
            FreeTree(&temp);
            if (flag) {
                return NewNode(NUM, NodeValue {.num = result}, nullptr, nullptr);
            }
            else return nullptr; // для ошибки
        }
        else
            return temp;

    }
    else
        assert(0); // если неизвестный тип
}

int CompareDoubles(double x, double y) {
    double result = x - y;
    if (fabs(result) < EPSILON)
        return 1;
    else
        return 0;

}

Errors Menu() {

    printf("\nDifferencator\n"
           "(c) F1mer\n\n"
           "Выберите Режим\n"
           "[1] - Посчитать значение функции в точке\n"
           "[2] - Продифференцировать выражение\n"
           "[3] - Разложить в ряд Тейлора\n"
           "[4] - Распечатка Выражения\n"
           "[5] - Выход\n"
           "Ваш ответ: ");

    int command = 0;
    command = GetMode(5); // TODO const

    switch(command) {
        case(KEY_1):
            EvalMode();
            break;
        case(KEY_2):
            DiffMode();
            break;
        case(KEY_3):
            TaylorMode();
            break;
        case(KEY_4):
            DumpMode();
            break;
        case(KEY_5):
            printf("Выход\n");
            break;
        default:
            assert(0);
            break;
    }

    return OK;
}

int GetMode(int mode_count) {
    assert(mode_count > 0 && mode_count <= KEY_COUNT - KEY_1);

    int command = 0;

    while(1) {
        command = getchar();
        while(getchar() != '\n');
        if (command >= KEY_1 && command < KEY_1 + mode_count)
            return command;
        else
            printf("Неверная комманда, попробуйте еще раз\n"
                   "Ваш ответ: ");
    }
}

Errors DumpMode() {
    Node *Root = nullptr;
    Errors err = BuildTreeFromFile("./expression.txt", &Root);

    if (err == OK) {
        printf("\n%sTREE WAS PARCED%s\n", COLOR_PASS, COLOR_RESET);
    } else {
        printf("%sWRONG FORMAT OF FILE%s\n", COLOR_FAIL, COLOR_RESET);
        return err;
    }

    TreeDumpDot(Root);
    int error = system("dot -Tpng ./GraphDump/dump.dot -o ./GraphDump/Expression.png");
    if (error != 0)
        return COMMAND_ERROR;

    printf("Выражение в Expression.png\n");
    FreeTree(&Root);

    Menu();

    return OK;
}

Errors EvalMode() {
    Node *Root = nullptr;
    Errors err = BuildTreeFromFile("./expression.txt", &Root);

    if (err == OK) {
        printf("\n%sTREE WAS PARCED%s\n", COLOR_PASS, COLOR_RESET);
    } else {
        printf("%sWRONG FORMAT OF FILE%s\n", COLOR_FAIL, COLOR_RESET);
        return err;
    }

    printf("\nПосчитать выражение в точке x = ");
    scanf("%lg", &VAR_VALUE);

    double result = Eval(Root);
    printf("Значение функции в точке %lg = %lg\n", VAR_VALUE, result);

    FreeTree(&Root);

    Menu();

    return OK;
}

Errors DiffMode() {
    Node *Root = nullptr;
    Errors err = BuildTreeFromFile("./expression.txt", &Root);

    if (err == OK) {
        printf("\n%sTREE WAS PARCED%s\n", COLOR_PASS, COLOR_RESET);
    } else {
        printf("%sWRONG FORMAT OF FILE%s\n", COLOR_FAIL, COLOR_RESET);
        return err;
    }

    Node *DiffRoot = Diff(Root);
    Node *SimplifyRoot = SimplifyTree(DiffRoot);

    TreeDumpDot(SimplifyRoot);
    int error = system("dot -Tpng ./GraphDump/dump.dot -o ./GraphDump/Diff.png");
    if (error != 0)
        return COMMAND_ERROR;

    printf("Продифференцированное дерево в файле Diff.png");

    printf("\nПосчитать производную в точке x = ");
    scanf("%lg", &VAR_VALUE);

    double result = Eval(SimplifyRoot);
    printf("Производная функции в точке %lg = %lg\n", VAR_VALUE, result);

    FreeTree(&Root);
    FreeTree(&SimplifyRoot);
    FreeTree(&DiffRoot);

    Menu();

    return OK;
}

unsigned long long factorial(int n) {
    assert(n >= 0);

    if (n == 0)
        return 1;
    unsigned long long result = 1;
    for (int i = 1; i <= n; ++i) {
        result *= i;
    }
    return result;
}

Errors TaylorMode() {
    printf("Посчитать разложение в точке: ");
    scanf("%lg\n", &VAR_VALUE);
    printf("Посчитать разложение до порядка: ");
    int order = -1;
    scanf("\n%d\n", &order);

    ExpandInTaylorSeries(order);

    Menu();
    return OK;
}

Errors ExpandInTaylorSeries(int order) {
    assert(order >= 0);

    Node *Root = nullptr;
    Errors err = BuildTreeFromFile("./expression.txt", &Root);

    if (err == OK) {
        printf("\n%sTREE WAS PARCED%s\n", COLOR_PASS, COLOR_RESET);
    } else {
        printf("%sWRONG FORMAT OF FILE%s\n", COLOR_FAIL, COLOR_RESET);
        return err;
    }

    printf("Разложение в ряд Тейлора:\n");

    for(int i = 0; i <= order; i++) {
        Node* temp = MultiplyDiff(Root, i);
        double result = Eval(temp);
        double koef = result / double(factorial(i));
        printf("%lg*(x - %lg)^%d + ", koef, VAR_VALUE, i);

        FreeTree(&temp);
    }
    printf("...\n");

    FreeTree(&Root);
    return OK;
}

Node* MultiplyDiff(Node *node, int order) {
    assert(node != nullptr && order >= 0);

    Node* temp1 = CopyTree(node);

    for (int i = 1; i <= order; i++) {
        Node* temp2 = temp1;
        Node* temp3 = Diff(temp1);
        temp1 = SimplifyTree(temp3);
        FreeTree(&temp2);
        FreeTree(&temp3);
    }

    return temp1;
}
