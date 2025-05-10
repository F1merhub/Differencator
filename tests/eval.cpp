#include "diff.h"

Errors EvalTests(char* argv[]);

int main(int argc, char* argv[])
{
    if (argc == 4 && strcmp(argv[1], "eval") == 0) {
        Errors err = EvalTests(argv);
        if (err != OK) {
            return err;
        }
    }
}

Errors EvalTests(char* argv[])
{
    const char* input_path = argv[2];
    const char* output_path = argv[3];

    Node *Root = nullptr;

    Errors err = BuildTreeFromFile(input_path, &Root);

    if (err == OK) {
        printf("tree was parsed\n");
    } else {
        printf("wrong format of file\n");
        return err;
    }

    double result = Eval(Root);

    FILE* fout = fopen(output_path, "w");
    if (fout == nullptr) {
        printf("output file was not open");
        return FILE_NOT_OPEN;
    }

    fprintf(fout, "%.10lf\n", result);
    fclose(fout);
    FreeTree(&Root);
    return OK;
}
