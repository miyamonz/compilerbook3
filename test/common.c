#include <stdio.h>
#include <stdlib.h>

// printfやexitを使うため、このファイルだけgccでコンパイルして、自前のコンパイラの結果とリンクして使う
void assert(int expected, int actual, char *code)
{
  if (expected == actual)
  {
    printf("%s => %d\n", code, actual);
  }
  else
  {
    printf("%s => %d expected but got %d\n", code, expected, actual);
    exit(1);
  }
}