#include "diff.h"

Errors Menu(const char* expression_file) {

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
            EvalMode(expression_file);
            break;
        case(KEY_2):
            DiffMode(expression_file);
            break;
        case(KEY_3):
            TaylorMode(expression_file);
            break;
        case(KEY_4):
            DumpMode(expression_file);
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
        while(getchar() != '\n'); // FIXME fix buffer clear
        if (command >= KEY_1 && command < KEY_1 + mode_count)
            return command;
        else
            printf("Неверная комманда, попробуйте еще раз\n"
                   "Ваш ответ: ");
    }
}

Errors DumpMode(const char* expression_file) {
    Node *Root = nullptr;
    Errors err = BuildTreeFromFile(expression_file, &Root);

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

    Menu(expression_file);

    return OK;
}

Errors EvalMode(const char* expression_file) {
    Node *Root = nullptr;
    Errors err = BuildTreeFromFile(expression_file, &Root);

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

    Menu(expression_file);

    return OK;
}

Errors DiffMode(const char* expression_file) {
    Node *Root = nullptr;
    Errors err = BuildTreeFromFile(expression_file, &Root);

    // FIXME ошибки пиши в stderr 
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

    Menu(expression_file);

    return OK;
}

Errors TaylorMode(const char* expression_file) {
    printf("Посчитать разложение в точке: ");
    scanf("%lg", &VAR_VALUE);
    printf("Посчитать разложение до порядка: ");
    int order = -1;
    scanf("%d", &order);

    ExpandInTaylorSeries(expression_file, order);

    Menu(expression_file);
    return OK;
}
