#include <stdio.h>
#include <stdlib.h>
#include "btree.h"
#include "string.h"

static void btree(void)
{
    freopen("Bplus_Tree2.log", "w", stdout);
    BTree *btree = btree_init(4);
    FILE *f = fopen("0_string.txt", "r");
    char line[128];
    int count=0;
    while (fgets(line, 128, f) != NULL)
    {
        char *find = strchr(line, '\n'); //查找换行符，如果find不为空指针
        if (find)
            *find = '\0';
        if(strlen(line)==0)continue;
        Column c;
        c.id = 1;
        strcpy(c.title, line);
        // printf("%d,%d -> %s\n", count, strlen(line), line);
        count++;
        btree_add(btree, &c);
    }
    printf("\n--------------\n");
    // FILE *resultFile = fopen("my_result.txt", "w");
    // int hit = 0;
    // f = fopen("0_dict.txt", "r");
    // while (fgets(line, 128, f) != NULL)
    // {
    //     char *find = strchr(line, '\n'); //查找换行符，如果find不为空指针
    //     if (find)
    //         *find = '\0';
    //     Column c;
    //     c.id = 1;
    //     strcpy(c.title, line);
    //     Column *result = btree_get_by_value(btree, &c);
    //     if (result != NULL)
    //     {
    //         fprintf(resultFile, "%s\n", line);
    //         hit++;
    //     }
    // }
    // printf("hits%d\n", hit);
    printf("\n------------------\n");
    btree_traverse(btree, print_node);
    printf("\n------------------\n");
    printf("btree_clear\n");
    btree_clear(btree);
}

int main()
{
    btree();

    return 0;
}
