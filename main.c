#include "main.h"

int main(int argc, char **argv)
{
    if (argc != 2)
        error("%s: 引数の個数が正しくありません\n", argv[0]);

    user_input = argv[1];
    token = tokenize();
    Node *node = expr();

    codegen(node);
    return 0;
}