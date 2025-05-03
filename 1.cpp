#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define BUFFER_SIZE 1024

typedef enum NodeType {
    OP,
    NUM,
    FUNC,
    TYPE_COUNT
} NodeType;

typedef enum Op {
    ADD,
    SUB,
    MUL,
    DIV,
    POW,
    OP_COUNT
} Op;

typedef enum Func {
    SIN,
    COS,
    LOG,
    EXP,
    FUNC_COUNT
} Func;

typedef union NodeValue {
    double num;
    Op op;
    Func func;
} NodeValue;

typedef struct Node {
    NodeType type;
    NodeValue value;
    struct Node* left;
    struct Node* right;
    struct Node* parent;
} Node;

typedef struct {
    const char* str;
    int value;
} SymbolMap;

// Таблица соответствия строковых представлений операторов и функций
const SymbolMap op_map[] = {
    {"+", ADD}, {"-", SUB}, {"*", MUL}, {"/", DIV}, {"^", POW}, {NULL, 0}
};

const SymbolMap func_map[] = {
    {"sin", SIN}, {"cos", COS}, {"log", LOG}, {"exp", EXP}, {NULL, 0}
};

Node* create_node(NodeType type, NodeValue value) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = type;
    node->value = value;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    return node;
}

void skip_whitespace(char** expr) {
    while (**expr == ' ' || **expr == '\t' || **expr == '\n') {
        (*expr)++;
    }
}

int match_symbol(char** expr, const SymbolMap* map) {
    skip_whitespace(expr);

    for (int i = 0; map[i].str != NULL; i++) {
        size_t len = strlen(map[i].str);
        if (strncmp(*expr, map[i].str, len) == 0) {
            *expr += len;
            return map[i].value;
        }
    }
    return -1;
}

Node* parse_expression(char** expr);

Node* parse_subexpression(char** expr) {
    skip_whitespace(expr);

    if (**expr == '(') {
        (*expr)++;
        Node* node = parse_expression(expr);
        skip_whitespace(expr);
        if (**expr != ')') {
            fprintf(stderr, "Ожидалась закрывающая скобка\n");
            exit(EXIT_FAILURE);
        }
        (*expr)++;
        return node;
    }

    // Пробуем распознать число
    char* end;
    double num = strtod(*expr, &end);
    if (*expr != end) {
        *expr = end;
        NodeValue value;
        value.num = num;
        return create_node(NUM, value);
    }

    // Пробуем распознать функцию
    int func = match_symbol(expr, func_map);
    if (func >= 0) {
        NodeValue value;
        value.func = func;
        Node* node = create_node(FUNC, value);
        node->left = parse_subexpression(expr);  // У функций один аргумент
        return node;
    }

    fprintf(stderr, "Неизвестный символ: %c\n", **expr);
    exit(EXIT_FAILURE);
}

Node* parse_expression(char** expr) {
    skip_whitespace(expr);

    // Пробуем распознать оператор
    int op = match_symbol(expr, op_map);
    if (op >= 0) {
        NodeValue value;
        value.op = op;
        Node* node = create_node(OP, value);
        node->left = parse_subexpression(expr);
        node->right = parse_subexpression(expr);
        return node;
    }

    // Если не оператор, пробуем подвыражение
    return parse_subexpression(expr);
}

void print_node(Node* node) {
    if (node == NULL) return;

    switch (node->type) {
        case NUM:
            printf("%g", node->value.num);
            break;
        case OP:
            switch (node->value.op) {
                case ADD: printf("+"); break;
                case SUB: printf("-"); break;
                case MUL: printf("*"); break;
                case DIV: printf("/"); break;
                case POW: printf("^"); break;
                default: printf("?");
            }
            break;
        case FUNC:
            switch (node->value.func) {
                case SIN: printf("sin"); break;
                case COS: printf("cos"); break;
                case LOG: printf("log"); break;
                case EXP: printf("exp"); break;
                default: printf("?");
            }
            break;
        default:
            printf("?");
    }
}

void print_tree(Node* root, int depth) {
    if (root == NULL) return;

    for (int i = 0; i < depth; i++) printf("  ");
    print_node(root);
    printf("\n");

    print_tree(root->left, depth + 1);
    print_tree(root->right, depth + 1);
}

void free_tree(Node* root) {
    if (root == NULL) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

int main() {
    char buffer[BUFFER_SIZE];
    printf("Введите выражение в префиксной записи: ");
    fgets(buffer, BUFFER_SIZE, stdin);

    // Удаляем символ новой строки
    buffer[strcspn(buffer, "\n")] = '\0';

    char* expr = buffer;
    Node* root = parse_expression(&expr);

    printf("\nПостроенное дерево:\n");
    print_tree(root, 0);

    free_tree(root);
    return 0;
}
