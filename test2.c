#include <stdio.h>
#include <stdlib.h>
#include "btree.h"
#include "string.h"

int main()
{
  BTree *btree= (BTree *)malloc(sizeof(BTree));
  printf("%p\n", btree);

  BTree *btree1= (BTree *)malloc(sizeof(BTree));
  printf("%p\n", btree1);
  free(btree1);
  free(btree);
  int *i =  (int *)malloc(sizeof(int));
  printf("%p\n", i);
  free(i);
  int *i1 =  (int *)malloc(sizeof(int));
  printf("%p\n", i1);
  free(i1);
  return 0;
}