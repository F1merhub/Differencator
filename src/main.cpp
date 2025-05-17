#include "diff.h"

// FIXME .gitignore и не заливать build

int main(int argc, char *argv[]) {

    // const char* expression_file = argc < 2 ? "./expression.txt" : argv[1];

    const char* expression_file = NULL;

    if (argc == 1)
    {
        expression_file = "./expression.txt";
    }

    else if (argc == 2)
    {
        expression_file = argv[1];
    }

    Menu(expression_file);

    return OK;
}
