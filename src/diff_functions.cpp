#include "diff.h"
#include "dsl.h"

double VAR_VALUE = 5;

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

Node* Diff(Node *node) {
    assert(node);

    // FIXME switch
    
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

    else if (node->type == OP) { // TODO в функцию
        Node* temp = NewNode(OP, NodeValue {.op = node->value.op}, SimplifyTree(node->left), SimplifyTree(node->right));
        if (temp->left == nullptr || temp->right == nullptr) {
            FreeTree(&temp);
            return nullptr;
        }

        if (node->value.op == ADD || node->value.op == SUB) { // СУММА РАЗНОСТЬ
            if (temp->left->type == NUM && temp->right->type == NUM) { // Если две константы
                double temp_op = 0;
                if (temp->value.op == ADD)
                    temp_op = temp->left->value.num + temp->right->value.num;
                else if (temp->value.op == SUB)
                    temp_op = temp->left->value.num - temp->right->value.num;

                NewNode(NUM, NodeValue {.num = temp_op}, nullptr, nullptr);
                FreeTree(&temp);
                return temp_num;

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
    return nullptr;
}

int CompareDoubles(double x, double y) {
    double result = x - y;
    if (fabs(result) < EPSILON)
        return 1;
    else
        return 0;

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

Errors ExpandInTaylorSeries(const char* expression_file, int order) {
    assert(order >= 0);

    Node *Root = nullptr;
    Errors err = BuildTreeFromFile(expression_file, &Root);

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
