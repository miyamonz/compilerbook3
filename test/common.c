#include <stdio.h>
#include <stdlib.h>

// printfやexitを使うため、このファイルだけgccでコンパイルして、自前のコンパイラの結果とリンクして使う
void assert(long expected, long actual, char *code)
{
  if (expected == actual)
  {
    // printf("%s => %ld\n", code, actual);
  }
  else
  {
    printf("%s => %ld expected but got %ld\n", code, expected, actual);
    exit(1);
  }
}