#ifndef _DSL_H
#define _DSL_H

#include "diff.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DIF_LEFT Diff(node->left)

#define DIF_RIGHT Diff(node->right)

#define COPY_LEFT CopyTree(node->left)

#define COPY_RIGHT CopyTree(node->right)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
