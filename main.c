#include <stdio.h>
#include <stdlib.h>
#include "btree.h"
#include "string.h"

int num = 0;

int _btree(BTree *btree)
{
    if (btree == NULL)
    {
        printf("null");
    }
    printf("new:%p\n", btree);
    FILE *f = fopen("dict.txt", "r");
    char line[128];
    int count = 0;
    while (fgets(line, 128, f) != NULL)
    {
        char *find = strchr(line, '\n'); //查找换行符，如果find不为空指针
        if (find)
            *find = '\0';
        if (strlen(line) == 0)
            continue;
        Column *c = (Column *)malloc(sizeof(Column));
        c->id = count; //rand() % 20000;
        strcpy(c->title, line);
        // printf("---开始保存(size:%d) %d: %d---\n", btree->size, count, c->id);
        if (btree_add(btree, c) != 2)
            count++;
        // printf("---已保存(size:%d) %d: %d -> %s\n", btree->size, count, c->id, line);
        // printf("B+树(size:%d, nodes:%d)\n", btree->size, btree_node_count(btree));
        // btree_traverse(btree, print_node);
        // printf("---保存完毕---\n\n");
        if (count >= 100)
            break;
    }
    fclose(f);
    printf("\n--------------\n");
    printf("B+树(nodes:%d leaf=%d) count=%d", btree_node_count(btree), btree_leaf_count(btree), count);
    printf("\n--------------\n");
    btree_traverse(btree, print_node);
    printf("\n------------------\n");
    FILE *resultFile = fopen("my_result.txt", "w");
    int hit = 0;
    f = fopen("string.txt", "r");
    while (fgets(line, 128, f) != NULL)
    {
        char *find = strchr(line, '\n'); //查找换行符，如果find不为空指针
        if (find)
            *find = '\0';
        Column c;
        c.id = 1;
        strcpy(c.title, line);
        printf("%s\n", line);
        if (btree_get_by_value(btree, &c) != NULL)
        {
            fprintf(resultFile, "%s\n", line);
            hit++;
        }
    }
    printf("hits %d\n", hit);
    printf("\n------------------\n");
    printf("btree_clear\n");
    fclose(f);
    fclose(resultFile);
    btree_clear(btree);
}

int _btree1(BTree *btree)
{
    if (btree == NULL)
        printf("null");
    printf("new:%p\n", btree);
    FILE *f = fopen("dict.txt", "r");
    char line[128];
    int count = 0, i = 0;
    for (i = 0; i < 100; i++)
    {
        Column *c = (Column *)malloc(sizeof(Column));
        c->id = rand() % 20000;
        strcpy(c->title, line);
        printf("---开始保存(size:%d) %d: %d---\n", btree->size, count, c->id);
        if (btree_add(btree, c) != 2)
            count++;
    }
    printf("\n--------------\n");
    printf("B+树(nodes:%d leaf=%d) count=%d", btree_node_count(btree), btree_leaf_count(btree), count);
    printf("\n--------------\n");
    Column c;
    c.id = 1;
    strcpy(c.title, line);
    Column *result = btree_get_by_value(btree, &c);
    if (result != NULL)
        printf("get %d\n", result->id);
    printf("\n------------------\n");
    btree_traverse(btree, print_node);
    printf("\n------------------\n");
    printf("btree_clear\n");
    btree_clear(btree);
}

int _btree2(BTree *btree)
{
    if (btree == NULL)
        printf("null");
    printf("new:%p\n", btree);
    FILE *f = fopen("dict.txt", "r");
    char line[128];
    int count = 0, i = 0;
    for (i = 0; i < 100; i++)
    {
        Column *c = (Column *)malloc(sizeof(Column));
        c->id = rand() % 20000;
        strcpy(c->title, line);
        printf("---开始保存(size:%d) %d: %d---\n", btree->size, count, c->id);
        if (btree_add(btree, c) != 2)
            count++;
    }
    printf("\n--------------\n");
    printf("B+树(nodes:%d leaf=%d) count=%d", btree_node_count(btree), btree_leaf_count(btree), count);
    printf("\n--------------\n");
    Column c;
    c.id = 1;
    strcpy(c.title, line);
    Column *result = btree_get_by_value(btree, &c);
    if (result != NULL)
        printf("get %d\n", result->id);
    printf("\n------------------\n");
    btree_traverse(btree, print_node);
    printf("\n------------------\n");
    printf("btree_clear\n");
    btree_clear(btree);
}

int main()
{
    // freopen("Bplus_Tree.log", "w", stdout);
    BTree *btree = (BTree *)malloc(sizeof(BTree));
    btree->root = NULL;
    btree->head = NULL;
    btree->size = 0;
    btree->degree = 2;
    _btree(btree);

    BTree *btree2 = (BTree *)malloc(sizeof(BTree));
    btree2->root = NULL;
    btree2->head = NULL;
    btree2->size = 0;
    btree2->degree = 2;
    _btree(btree);
    return 0;
}
