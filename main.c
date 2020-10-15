#include <stdio.h>
#include <stdlib.h>
#include "btree.h"
#include "string.h"

static void btree(void)
{
    // freopen("Bplus_Tree2.log", "w", stdout);
    BTree *btree = btree_init(2);
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
        c->id = count;//rand() % 20000;
        strcpy(c->title, line);
        // printf("---开始保存(size:%d) %d: %d---\n", btree->size, count, c->id);
        if (btree_add(btree, c) != 2)
        {
            count++;
        }
        // printf("---已保存(size:%d) %d: %d -> %s\n", btree->size, count, c->id, line);
        // printf("B+树(size:%d, nodes:%d)\n", btree->size, btree_node_count(btree));
        // btree_traverse(btree, print_node);
        // printf("---保存完毕---\n\n");
        // if (count >= 150)
        // {
        //     break;
        // }
    }
    printf("\n--------------\n");
    printf("B+树(nodes:%d leaf=%d) count=%d", btree_node_count(btree), btree_leaf_count(btree), count);
    printf("\n--------------\n");
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
        Column *result = btree_get_by_value(btree, &c);
        if (result != NULL)
        {
            fprintf(resultFile, "%s\n", line);
            hit++;
        }
    }
    printf("hits %d\n", hit);
    printf("\n------------------\n");
    // btree_traverse(btree, print_node);
    printf("\n------------------\n");
    printf("btree_clear\n");
    btree_clear(btree);
}

int main()
{
    btree();

    return 0;
}
