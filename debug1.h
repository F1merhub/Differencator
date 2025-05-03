#include "diff.h"

const size_t BUFFER_SIZE = 100;

Node* NewNode(NodeType type, NodeValue value, Node* left, Node* right)
{
    Node* node = (Node*)calloc(1, sizeof(Node));
    if (!node) return nullptr;

    node->type = type;
    node->value = value;

    node->left = left;
    node->right = right;

    if (left)  left->parent  = node;
    if (right) right->parent = node;

    return node;
}


// void ParseMathExpr(Node **node, char **buffer, Node *parent)
// {
//     assert(node != nullptr);
//     assert(buffer != nullptr);
//     if (*buffer == nullptr) return;
//
//     SkipSpaces(buffer);
//
//     if (**buffer != '(')
//     {
//         return;
//     }
//
//     (*buffer)++;
//     SkipSpaces(buffer);
//
//     char value_buf[BUFFER_SIZE] = "";
//     int offset = 0;
//     if (sscanf(*buffer, "%[^ )]%n", value_buf, &offset) != 1)
//     {
//         return;
//     }
//
//     *buffer += offset;
//     SkipSpaces(buffer);
//
//     NodeType type = DetectNodeType(value_buf);
//     CodeError err = CreateNode(node, value_buf, parent);
//     if (err != OK) return;
//
//     if (type == NUM || type == VAR)
//     {
//         if (**buffer != ')')
//         {
//             return;
//         }
//
//         (*buffer)++;
//         return;
//     }
//
//     ParseMathExpr(&((*node)->left), buffer, *node);
//     ParseMathExpr(&((*node)->right), buffer, *node);
//
//     while (isspace(**buffer)) (*buffer)++;
//
//     if (**buffer != ')')
//     {
//         FreeTree(node);
//         return;
//     }
//
//     (*buffer)++;
// }
