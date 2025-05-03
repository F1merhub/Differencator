#include <stdio.h>
#include <stdlib.h>
#include "diff.h"

/*
    Как добавить в новый проект?
    1.добавить константу DUMP_BUFFER_SIZE +
    2.изменить название ошибок и структуры дерева.
    3.добавить заголовки функций.
    4.изменить положение файла.dot
    5.изменить кол-во аргументов
    6.заменить спецификаторы

    +dot -Tpng ./GraphDump/dump.dot -o ./GraphDump/dump.png изменить вызов создания .png
*/

Errors TreeDumpDot(Node* Root)
{
    char* buffer = (char*)calloc(DUMP_BUFFER_SIZE, sizeof(char));
    if (buffer == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return MEMORY_ALLOCATION_ERROR;
    }

    int buffer_len = 0;

    // Начало .dot файла
    buffer_len += snprintf(buffer + buffer_len, DUMP_BUFFER_SIZE - (size_t)buffer_len,
                           "digraph G {\n"
                           "\trankdir = TB;\n"
                           "\tbgcolor=\"#1e1e2e\";\n"
                           "\tnode [shape=rectangle, style=filled, fontname=Helvetica, fontsize=12, fillcolor=\"#89b4fa\", color=\"#cba6f7\", fontcolor=\"#1e1e2e\"];\n");

    // Генерация графа
    buffer_len += GenerateGraph(Root, buffer, &buffer_len, DUMP_BUFFER_SIZE);

    buffer_len += snprintf(buffer + buffer_len, DUMP_BUFFER_SIZE - (size_t)buffer_len, "}\n");

    FILE* dump_file = fopen("./GraphDump/dump.dot", "w+");
    if (dump_file == NULL)
    {
        fprintf(stderr, "Failed to open dump.dot\n");
        free(buffer);
        return FILE_NOT_OPEN;
    }

    fprintf(dump_file, "%s", buffer);

    fclose(dump_file);
    free(buffer);

    return OK;
}

int GenerateGraph(Node *node, char* buffer, int* buffer_len, const size_t BUFFER_SIZE)
{
    if (!node) return 0;

    // *buffer_len += snprintf(buffer + *buffer_len, BUFFER_SIZE - (size_t)*buffer_len,
    //                         "\tnode%p [shape=plaintext; style=filled; color=\"#fcf0d2\"; fillcolor=\"#b2d4fc\"; label = <\n"
    //                         "\t\t         <table BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"6\" BGCOLOR=\"#a1c4fd\" COLOR=\"#4f4f4f\">\n"
    //                         "\t\t\t           <tr><td align='center' colspan='2'><FONT COLOR='#3b4252'><b>Node: %p</b></FONT></td></tr>\n"
    //                         "\t\t\t           <tr><td align='center' colspan='2'><FONT COLOR='#2e8b57'>Value: <b> %d </b></FONT></td></tr>\n" // NOTE для добавления вывода какого либо параметра узла снизу, просто добавить данную строку, добавить аргумент и заменить параметр
    //                         "\t\t\t           <tr><td align='center' colspan='2'><FONT COLOR='#2e8b57'>Value: <b> %lg </b></FONT></td></tr>\n"
    //                         "\t\t\t           <tr>\n"
    //                         "\t\t\t               <td WIDTH='150' PORT='left'  align='center'><FONT COLOR='#006400'><b>Left: %p</b></FONT></td>\n"
    //                         "\t\t\t               <td WIDTH='150' PORT='right' align='center'><FONT COLOR='#b94e48'><b>Right: %p</b></FONT></td>\n"
    //                         "\t\t\t           </tr>\n"
    //                         "\t\t         </table> >];\n",
    //                         node, node, node->type, node->value.num, node->left, node->right);

    *buffer_len += snprintf(buffer + *buffer_len, BUFFER_SIZE - (size_t)*buffer_len,
                       "\tnode%p [shape=plaintext; style=filled; color=\"#fcf0d2\"; fillcolor=\"#b2d4fc\"; label = <\n"
                       "\t\t         <table BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"6\" BGCOLOR=\"#a1c4fd\" COLOR=\"#4f4f4f\">\n"
                       "\t\t\t           <tr><td align='center' colspan='2'><FONT COLOR='#3b4252'><b>Node: %p</b></FONT></td></tr>\n"
                       "\t\t\t           <tr><td align='center' colspan='2'><FONT COLOR='#2e8b57'>Value: <b> %d </b></FONT></td></tr>\n",
                       node, node, node->type);

    // Вывод значения в зависимости от типа
    if (node->type == OP) {
        *buffer_len += snprintf(buffer + *buffer_len, BUFFER_SIZE - (size_t)*buffer_len,
                                "\t\t\t           <tr><td align='center' colspan='2'><FONT COLOR='#2e8b57'>Value: <b> %d </b></FONT></td></tr>\n",
                                node->value.op);
    } else {
        *buffer_len += snprintf(buffer + *buffer_len, BUFFER_SIZE - (size_t)*buffer_len,
                                "\t\t\t           <tr><td align='center' colspan='2'><FONT COLOR='#2e8b57'>Value: <b> %lg </b></FONT></td></tr>\n",
                                node->value.num);
    }

    // Продолжение строки
    *buffer_len += snprintf(buffer + *buffer_len, BUFFER_SIZE - (size_t)*buffer_len,
                            "\t\t\t           <tr>\n"
                            "\t\t\t               <td WIDTH='150' PORT='left'  align='center'><FONT COLOR='#006400'><b>Left: %p</b></FONT></td>\n"
                            "\t\t\t               <td WIDTH='150' PORT='right' align='center'><FONT COLOR='#b94e48'><b>Right: %p</b></FONT></td>\n"
                            "\t\t\t           </tr>\n"
                            "\t\t         </table> >];\n",
                            node->left, node->right);

    // Рекурсивно генерируем левое поддерево
    if (node->left)
    {
        *buffer_len += GenerateGraph(node->left, buffer, buffer_len, BUFFER_SIZE);
        *buffer_len += snprintf(buffer + *buffer_len, BUFFER_SIZE - (size_t)*buffer_len,
                                 "\tnode%p:left -> node%p [color=\"#a6e3a1\"]\n", node, node->left);
    }

    // Рекурсивно генерируем правое поддерево
    if (node->right)
    {
        *buffer_len += GenerateGraph(node->right, buffer, buffer_len, BUFFER_SIZE);
        *buffer_len += snprintf(buffer + *buffer_len, BUFFER_SIZE - (size_t)*buffer_len,
                                 "\tnode%p:right -> node%p [color=\"#f9e2af\"]\n", node, node->right);
    }

    return 0;
}
