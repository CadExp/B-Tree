#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1800               // 哈希表大小
#define SPLIT 100            // 哈希成 7 个小的对应文件
#define MAX_STRING_LENGTH 45 // 最大字符串长度
#define FILE_LINE_BUFFER 128 // 读取一行的buffer

// 键值数组结构
typedef struct column
{
  int id; // 关键字
  char title[128];
} Column;

// B树结点
typedef struct bnode
{
  int size;                // 当前关键字数目
  Column **columns;        // 键值数组
  struct bnode **children; // 儿子指针数组
  struct bnode *parent;    //父指针
  struct bnode *next;      //横向顺序查找的链表指针
  unsigned int leaf;       // 是否为叶子 1是 0否
} BNode;

// B树ADT对外接口
typedef struct btree
{
  unsigned int degree; // 度数
  BNode *root;         // 根结点
  BNode *head;         // 叶子链表的头结点
  unsigned int size;   // B树结点大小
} BTree;

// ! 以下方法 return 0失败 1成功

//0不等 1相等
static int column_is_equal(const Column *a, const Column *b)
{
  if (a == NULL)
  {
    printf("a==NULL\n");
    return 0;
  }
  if (b == NULL)
  {
    printf("b==NULL\n");
    return 0;
  }
  if (strcmp(a->title, b->title) == 0)
    return 1;
  return 0;
}
//a>b则1，否则0
static int column_a_greater_than_b(const Column *a, const Column *b)
{
  if (a == NULL)
  {
    printf("a==NULL\n");
    return 0;
  }
  if (b == NULL)
  {
    printf("b==NULL\n");
    return 0;
  }
  if (strcmp(a->title, b->title) > 0)
    return 1;
  return 0;
}
//a>=b则1，否则0
static int column_a_greater_or_equal_b(const Column *a, const Column *b)
{
  if (a == NULL)
  {
    printf("a==NULL\n");
    return 0;
  }
  if (b == NULL)
  {
    printf("b==NULL\n");
    return 0;
  }
  int r = strcmp(a->title, b->title);
  if (r > 0 || r == 0)
    return 1;
  return 0;
}

// 顺序查找结点关键字
// 每个结点最多关键字为2t-1，时间复杂度为O(2t-1)，即O(t)
static int seq_search(const Column *array[], const int len, const Column *value)
{
  int i = 0;
  while (i <= len - 1 && column_a_greater_than_b(value, array[i]))
    i++;
  return i;
}

// 二分法查找结点上相同的关键字、确定儿子访问位置
// 每个结点关键字数为2t-1，时间复杂度为O(log(2t-1))，即O(log t)
static int binary_search(Column *array[], const int len, const Column *value)
{
  int start = 0, end = len - 1, index = 0, center = (start + end) / 2;
  while (start <= end)
  {
    if (column_is_equal(value, array[center]) == 1)
    {
      index = center;
      break;
    }
    else if (column_a_greater_than_b(value, array[center]) == 1)
    {
      index = center + 1;
      center += 1;
      start = center;
    }
    else
    {
      index = center;
      center -= 1;
      end = center;
    }
    center = (start + end) / 2;
  }
  return index;
}

//初始化
BTree *btree_init(unsigned int degree)
{
  BTree *btree = (BTree *)malloc(sizeof(BTree));
  if (!btree)
  {
    perror("init b tree error.");
    return NULL;
  }
  btree->root = NULL;
  btree->head = NULL;
  btree->size = 0;
  btree->degree = degree;
  return btree;
}
//空
int btree_is_empty(BTree *btree)
{
  if (btree == NULL)
    return 1;
  return btree->size == 0;
}
//结点构造器
//按BTree的设置（m叉），生成一个结点
//还没插入树中
static BNode *new_node(BTree *btree)
{
  BNode *node = (BNode *)malloc(sizeof(BNode));
  if (!node)
    return NULL;
  node->columns = (Column **)malloc(sizeof(Column *) * (2 * btree->degree - 1)); //2t-1个关键字
  node->children = (BNode **)malloc(sizeof(BNode *) * (2 * btree->degree));      // 2t棵子树
  node->next = NULL;
  node->parent = NULL;
  if (!node->columns)
  {
    free(node);
    return NULL;
  }
  if (!node->children)
  {
    free(node->columns);
    free(node);
    return NULL;
  }
  return node;
}

//将数据 column 存储为 node 的第 index 个关键字
// index 从 0 开始
static int replace_data_at_node(BNode *node, int index, Column *column)
{
  Column *c = (Column *)malloc(sizeof(Column));
  if (!c)
    return 0;
  c->id = column->id;
  strcpy(c->title, column->title);
  node->columns[index] = c;
  return 1;
}

//将数据 column 存储为 node 的第 index 个关键字
// index 从 0 开始
static int save_data_to_node(BNode *node, int index, Column *column)
{
  replace_data_at_node(node, index, column);
  node->size++;
  return 1;
}

static void print_node_data(BNode *node)
{
  if (node == NULL)
  {
    printf("null");
    return;
  }
  printf("[");
  int i;
  for (i = 0; i < node->size - 1; i++)
  {
    printf("%s, ", node->columns[i]->title);
  }
  printf("%s]", node->columns[node->size - 1]->title);
}
static void print_node(BNode *node)
{
  print_node_data(node);
  printf(", size: %d", node->size);
  printf(", parent:");
  print_node_data(node->parent);
  printf(", next:");
  print_node_data(node->next);
  printf("\n");
}
// 最大值
static Column *node_max(BNode *root)
{
  while (!root->leaf)
    root = root->children[root->size - 1];
  return root->columns[root->size - 1];
}
// A、子树最大值
Column *btree_max(BTree *btree)
{
  if (btree == NULL || btree->size == 0)
    return NULL;
  return node_max(btree->root);
}
// B、子树最小值
static Column *node_min(BNode *root)
{
  while (!root->leaf)
    root = root->children[0];
  return root->columns[0];
}

Column *btree_min(BTree *btree)
{
  if (btree == NULL || btree->size == 0)
    return NULL;
  return node_min(btree->root);
}
// 按 id 在子树中查找
static Column *btree_get(BNode *root, Column *c)
{
  int index = binary_search(root->columns, root->size, c);
  if (index < root->size && column_is_equal(root->columns[index], c) == 1)
    return root->columns[index];
  else if (root->leaf)
    return NULL;
  else
    return btree_get(root->children[index], c);
}

Column *btree_get_by_value(BTree *btree, Column *c)
{
  if (btree == NULL || btree->size == 0)
    return NULL;
  return btree_get(btree->root, c);
}

//中序遍历
// 0. 插入5个
// [1, 2, 3, 4, 5]
//
// 1. 插入6个
// [3]
// |--[1, 2]
// |--[4, 5, 6]
//
// 2. 插入30个
// [9, 18]
// |--[3, 6]
// |--|--[1, 2]
// |--|--[4, 5]
// |--|--[7, 8]
// |--[12, 15]
// |--|--[10, 11]
// |--|--[13, 14]
// |--|--[16, 17]
// |--[21, 24, 27]
// |--|--[19, 20]
// |--|--[22, 23]
// |--|--[25, 26]
// |--|--[28, 29, 30]
static void traverse_tree(BNode *root, int depth, char *prefix, void (*traverse)(BNode *))
{
  //1.输出当前结点的信息
  printf("%s", prefix);
  traverse(root);
  //2.依次遍历子树
  char *prefix_plus = malloc(sizeof(char) * (depth * 3));
  int i;
  for (i = 0; i < depth * 3; i++)
  {
    prefix_plus[i] = ' ';
  }
  // strcpy(prefix_plus, prefix);

  prefix_plus[depth * 3] = '|';
  prefix_plus[depth * 3 + 1] = '-';
  prefix_plus[depth * 3 + 2] = '-';
  if (!root->leaf)
  {
    for (i = 0; i < root->size; ++i)
    {
      traverse_tree(root->children[i], depth + 1, prefix_plus, traverse);
    }
  }
  //printf("遍历完成 %d, %s\n", depth, root->columns[0]->title);
}

static void traverse_node(BNode *root)
{
  traverse_tree(root, 0, "", print_node);
}
void btree_traverse(BTree *btree, void (*traverse)(BNode *))
{
  if (btree == NULL || btree->size == 0)
    return;
  traverse_tree(btree->root, 0, "", traverse);
}

//分割满结点，返回分割后的子树根结点
//root是一个满结点
//[1,2,3]
//分裂为
//[2,3]
//|--[1,2]
//|--[3]
static BNode *split_full_node(BTree *btree, BNode *root, int index)
{
  int i, j, k;
  // printf("结点 ");
  // print_node_data(root);
  if (root->parent != NULL)
  {
    //结点有父节点
    // printf(" 有父结点: ");
    BNode *parent = root->parent;
    // print_node_data(parent);
    // printf("\n");
    BNode *left = root;
    BNode *that_parent = parent;
    if (parent->size == 2 * btree->degree - 1)
    {
      //父结点也满了，需要递归分裂
      // printf("  父结点也满了，需要递归分裂\n");
      // printf("  ---递归中---\n");
      parent = split_full_node(btree, parent, index);
      // printf("  left:\n");
      // print_node(left);
      // printf("  left parent:\n");
      // print_node(left->parent);
      // printf("  split_parent:\n");
      // print_node(parent);
      that_parent = parent;
      // printf("  that_parent:\n");
      // print_node(that_parent);
      // printf("  ---递归完成---\n");
      if (parent == NULL)
      {
        printf("  分裂失败\n");
        return NULL;
      }
    }
    else
    {
      //父结点有空闲
      // printf("  父结点有空闲\n");
      that_parent = parent;
    }
    // printf("  创建一个兄弟结点\n");
    BNode *right = new_node(btree);
    if (!right)
      return 0;
    int split_index = btree->degree;
    for (i = split_index; i < left->size; i++)
    {
      right->columns[i - split_index] = left->columns[i];
      right->children[i - split_index] = left->children[i];
    }
    right->leaf = left->leaf;
    left->size = split_index;
    right->size = split_index - 1;
    // printf("  分裂为两个子树，左子树 ");
    // print_node_data(left);
    // printf("，右子树 ");
    // print_node_data(right);
    // printf("\n");
    left->next = right;
    //that_parent一定非满
    Column *left_max_id = node_max(left);
    for (i = 0; i < that_parent->size; i++)
    {
      Column *id = that_parent->columns[i];
      if (column_a_greater_or_equal_b(id, left_max_id) == 1)
      {
        for (j = that_parent->size - 1; j > i; j--)
        {
          that_parent->columns[j] = that_parent->columns[j - 1];
          that_parent->children[j] = that_parent->children[j - 1];
        }
        that_parent->columns[i] = node_max(left);
        that_parent->children[i] = left;
        that_parent->columns[i + 1] = node_max(right);
        that_parent->children[i + 1] = right;
        left->parent = that_parent;
        right->parent = that_parent;
        that_parent->size++;
        btree->size++;
        // traverse_node(that_parent);
        break;
      }
    }
    // btree_traverse(btree, print_node);

    return right;
  }
  else
  {
    //结点没父节点
    // printf(" 没父节点\n");
    //1.创建一个父结点
    //2.创建一个兄弟结点，根据index看这个兄弟结点是作为左子树还是右子树
    // printf("  创建一个兄弟结点\n");
    BNode *right = new_node(btree);
    if (!right)
      return 0;
    BNode *left = root;
    int split_index = btree->degree;
    int right_index;
    for (i = split_index; i < left->size; i++)
    {
      right_index = i - split_index; //左结点的第i棵子树复制为右结点的第right_index棵子树
      right->columns[right_index] = left->columns[i];
      right->children[right_index] = left->children[i];
      if (right->children[right_index] != NULL)
      {
        right->children[right_index]->parent = right;
      }
    }
    right->leaf = left->leaf;
    left->size = split_index;
    right->size = split_index - 1;

    // printf("  创建一个父结点\n");
    BNode *parent = new_node(btree);
    if (!parent)
      return 0;
    parent->size = 2;
    parent->children[0] = left;
    parent->children[1] = right;
    parent->columns[0] = left->columns[split_index - 1];
    parent->columns[1] = right->columns[split_index - 2];
    parent->leaf = 0;

    left->parent = parent;
    right->parent = parent;
    left->next = right;

    btree->root = parent;
    btree->size += 2;
    // btree_traverse(btree, print_node);
    return right;
  }
}

//替换中间结点的最大值 last_column 为 column
static int replace_max_of_intrenal_node(BTree *btree, BNode *root, Column *last_column, Column *column)
{
  // printf("替换中间结点 ");
  // print_node_data(root);
  // printf(" 的最大值 %d 为 %d\n", last_column->id, column->id);
  int i;
  if (root->leaf)
  {
    return 0;
  }
  else
  {
    for (i = 0; i < root->size; i++)
    {
      if (column_is_equal(root->columns[i], last_column) == 1)
      {
        // replace_data_at_node(root, i, column);
        root->columns[i] = column;
        if (root->parent != NULL)
        {
          replace_max_of_intrenal_node(btree, root->parent, last_column, column);
        }
        break;
      }
    }
    return 1;
  }
}

//在BTree的定义下，将数据column插入子树root中
//btree是上下文，不参与结构构建
//root一定非空
static int add_node(BTree *btree, BNode *root, Column *column)
{
  if (root == NULL)
  {
    printf("树为空\n");
    return 0;
  }
  int i;
  // printf("树不为空 ");
  // print_node_data(root);
  // printf("\n");
  //树不空时，先搜索到叶子，如果叶子满了，再递归分裂，直到一个非满的子树，插入到那个子树
  //二分法搜索最佳插入点
  int index = binary_search(root->columns, root->size, column);
  // printf("二分法搜索最佳插入点, index=%d\n", index);
  if (root->leaf)
  {
    // printf("结点是叶子\n");
    //root结点是叶子
    if (index < root->size && column_is_equal(root->columns[index], column)==1)
    {
      printf("已经存在该数据了，直接覆盖数据内容而不是另外分配空间\n");
      //已经存在该数据了，直接覆盖数据内容而不是另外分配空间
      strcpy(root->columns[index]->title, column->title);
      return 1;
    }
    //这是新数据
    //root结点没有该数据，需要分配空间
    // printf("结点没有该数据，需要分配空间\n");
    if (root->size == 2 * btree->degree - 1)
    {
      // printf("叶子满了\n");
      //需要递归分裂
      // print_node_data(root->parent);
      // printf("\n");
      // btree_traverse(btree, print_node);
      BNode *parent = split_full_node(btree, root, index);
      if (parent == NULL)
      {
        //分裂失败
        printf("分裂失败\n");
        return 0;
      }
      return add_node(btree, btree->root, column);
    }
    else
    {
      // printf("叶子没满\n");
      //将 index 右边的结点往右挪一位
      for (i = root->size; i > index; --i)
      {
        root->columns[i] = root->columns[i - 1];
      }
      //保存到 index
      // printf("保存到 %d 到 index:%d\n", column->id, index);
      save_data_to_node(root, index, column);
      if (index == root->size - 1 && root->parent != NULL)
      {
        // printf("插入后是最大的，需要递归修改父结点\n");
        return replace_max_of_intrenal_node(btree, root->parent, root->columns[index - 1], column);
      }
      return 1;
    }
  }
  else
  {
    // printf("结点不是叶子\n");
    if (index == root->size)
    {
      index--;
    }
    return add_node(btree, root->children[index], column);
  }
}

//将数据colum插入B+树中
int btree_add(BTree *btree, Column *column)
{
  if (btree == NULL || column == NULL)
    return 0;
  if (!btree->root)
  {
    printf("树为空，将插入的结点当作根结点\n");
    BNode *root = new_node(btree);
    if (!root)
      return 0;
    printf("保存到结点\n");
    save_data_to_node(root, 0, column);
    printf("保存完成\n");
    root->leaf = 1;

    btree->root = root;
    btree->head = root;
    btree->size = 1;
    return 1;
  }
  // printf("保存到结点\n");
  return add_node(btree, btree->root, column);
}

// 释放空间
static int clear_node(BNode *root)
{
  if (!root)
    return 0;
  int i;
  if (root->leaf)
  {
    // printf("---释放叶子---");
    // print_node_data(root);
    // printf("\n");
    for (i = 0; i < root->size; ++i)
    {
      free(root->columns[i]);
    }
    free(root->columns);
    return 1;
  }
  // printf("//释放中间结点---");
  // print_node_data(root);
  // printf("\n");

  for (i = 0; i < root->size; ++i)
  {
    clear_node(root->children[i]);
  }
  free(root->columns);
  free(root);
  // printf("\\\\释放中间结点结束---\n");
  return 1;
}

int btree_clear(BTree *btree)
{
  if (btree == NULL)
    return 0;
  clear_node(btree->root);
  free(btree);
  return 1;
}

char *line;
int key = -1;
char *itoa(int num, char *str, int radix)
{
  char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"; //索引表
  unsigned unum;                                         //存放要转换的整数的绝对值,转换的整数可能是负数
  int i = 0, j, k;                                       //i用来指示设置字符串相应位，转换之后i其实就是字符串的长度；转换后顺序是逆序的，有正负的情况，k用来指示调整顺序的开始位置;j用来指示调整顺序时的交换。

  //获取要转换的整数的绝对值
  if (radix == 10 && num < 0) //要转换成十进制数并且是负数
  {
    unum = (unsigned)-num; //将num的绝对值赋给unum
    str[i++] = '-';        //在字符串最前面设置为'-'号，并且索引加1
  }
  else
    unum = (unsigned)num; //若是num为正，直接赋值给unum

  //转换部分，注意转换后是逆序的
  do
  {
    str[i++] = index[unum % (unsigned)radix]; //取unum的最后一位，并设置为str对应位，指示索引加1
    unum /= radix;                            //unum去掉最后一位

  } while (unum); //直至unum为0退出循环

  str[i] = '\0'; //在字符串最后添加'\0'字符，c语言字符串以'\0'结束。

  //将顺序调整过来
  if (str[0] == '-')
    k = 1; //如果是负数，符号不用调整，从符号后面开始调整
  else
    k = 0; //不是负数，全部都要调整

  char temp;                         //临时变量，交换两个值时用到
  for (j = k; j <= (i - 1) / 2; j++) //头尾一一对称交换，i其实就是字符串的长度，索引最大值比长度少1
  {
    temp = str[j];               //头部赋值给临时变量
    str[j] = str[i - 1 + k - j]; //尾部赋值给头部
    str[i - 1 + k - j] = temp;   //将临时变量的值(其实就是之前的头部值)赋给尾部
  }

  return str; //返回转换后的字符串
}

// ELF Hash Function
unsigned int ELFHash(char *str)
{
  unsigned int hash = 0;
  unsigned int x = 0;

  while (*str)
  {
    hash = (hash << 4) + (*str++);
    if ((x = hash & 0xF0000000L) != 0)
    {
      hash ^= (x >> 24);
      hash &= ~x;
    }
  }

  return (hash & 0x7FFFFFFF);
}

unsigned int hash(char *str)
{
  return ELFHash(str);
}

/**
 * dictFile: 字典文件
 * wordFile: 待搜索文件
 * resultFile: 结果输出
 * m : m阶B+树
 */
static int search(FILE *dictFile, FILE *wordFile, FILE *resultFile, int m)
{
  //构建哈希字典
  int k = (m + 1) / 2;
  printf("building B+ tree, m=%d, k=%d\n", m, k);
  BTree *btree = btree_init(k);
  int counter = 0;
  while (fgets(line, FILE_LINE_BUFFER, dictFile) != NULL)
  {
    char *find = strchr(line, '\n'); //查找换行符，如果find不为空指针
    Column c;
    c.id = 1;
    strcpy(c.title, line);
    btree_add(btree, &c);
    counter++;
    if(counter >= 10)break;
  }
  printf("\n---btree_traverse begin---%d\n\n", btree->size);
  btree_traverse(btree, print_node);
  printf("---btree_traverse end---\n");

  //搜索
  int hit = 0;
  while (fgets(line, FILE_LINE_BUFFER, wordFile) != NULL)
  {
    char *find = strchr(line, '\n'); //查找换行符，如果find不为空指针
    if (find)
      *find = '\0';
    if (line[0] == '\n' || line[0] == '\0')
    {
      printf("skip an empty line\n");
      continue;
    }

    printf("%s\n", line);
    Column *c = (Column *)malloc(sizeof(Column));
    c->id = 0; //hash(line);
    strcpy(c->title, line);
    Column *result = btree_get_by_value(btree, c);
    if (result != NULL)
    {
      fprintf(resultFile, "%s\n", line);
      hit++;
    }
  }
  fclose(wordFile);
  fclose(dictFile);
  printf("hit %d\n", hit);
  btree_clear(btree);
  return hit;
}

char *key2string(int key, char *suffix, char *buffer)
{
  char *keyString = itoa(key, buffer, 10);
  return strcat(keyString, suffix);
}

/*
dict.txt   词典串127万个
string.txt 待匹配的1.7万个字符串
result.txt 实验结果，所有查找到的串，一行一个

gcc bplus_search.c -o bplus_search.exe && ./bplus_search.exe dict.txt string.txt
*/
int main(int argc, char *argv[])
{
  // freopen("Bplus_Tree.log", "w", stdout);
  char *dictFilename = "dict.txt";
  char *wordFilename = "string.txt";
  int m = 5;

  // 命令行参数处理
  if (argc != 3)
  {
    printf("you can call me like this: \n\n    bplus_search m dict.txt string.txt\n\n");
  }
  else
  {
    m = atoi(argv[1]);
    dictFilename = argv[2];
    wordFilename = argv[3];
  }
  printf("dict: %s, word: %s\n", dictFilename, wordFilename);

  // 统计运行时间
  clock_t start_t = clock();
  printf("--- start\n");
  int i = 0;
  line = (char *)malloc(FILE_LINE_BUFFER * sizeof(char)); // 读取一行字符串的 buffer
  // 哈希成小的文件对
  char filenameBuffer[12] = {0};
  FILE *file = NULL, *dictOutput[SPLIT], *wordOutput[SPLIT];
  for (i = 0; i < SPLIT; i++)
  {
    dictOutput[i] = NULL;
    wordOutput[i] = NULL;
  }
  //构建哈希字典
  if ((file = fopen(dictFilename, "r")) == NULL)
  {
    printf("fail to open file %s\n", dictFilename);
    return 0;
  }
  printf("open %s\n", dictFilename);
  while (fgets(line, 63, file) != NULL)
  {
    char *find = strchr(line, '\n'); //查找换行符
    if (find)                        //如果find不为空指针
      *find = '\0';                  //就把一个空字符放在这里
    key = abs(hash(line) % SPLIT);
    if (dictOutput[key] == NULL)
    {
      char *targetFilename = key2string(key, "_dict.txt", filenameBuffer);
      // printf("open %s\n", targetFilename);
      if ((dictOutput[key] = fopen(targetFilename, "w+")) == NULL)
      {
        printf("fail to open file %s\n", targetFilename);
        return 0;
      }
    }
    fprintf(dictOutput[key], "%s\n", line);
  }
  fclose(file);

  if ((file = fopen(wordFilename, "r")) == NULL)
  {
    printf("fail to open file %s\n", wordFilename);
    return 0;
  }
  printf("open %s\n", wordFilename);
  while (fgets(line, 63, file) != NULL)
  {
    char *find = strchr(line, '\n'); //查找换行符，如果find不为空指针
    if (find)
      *find = '\0';
    key = abs(hash(line) % SPLIT);
    if (wordOutput[key] == NULL)
    {
      char *targetFilename = key2string(key, "_string.txt", filenameBuffer);
      // printf("open %s\n", targetFilename);
      if ((wordOutput[key] = fopen(targetFilename, "w+")) == NULL)
      {
        printf("fail to open file %s\n", targetFilename);
        return 0;
      }
    }
    fprintf(wordOutput[key], "%s\n", line);
  }
  fclose(file);

  printf("--- split using time : %f s\n", (double)((double)(clock() - start_t) / (double)(CLOCKS_PER_SEC)));

  // 生成结果
  FILE *resultFile = NULL;
  if ((resultFile = fopen("bupt_14.txt", "w+")) == NULL)
  {
    printf("fail to open file result.txt\n");
    return 0;
  }
  printf("open result.txt\n");
  int total_hits = 0;
  for (i = 0; i < SPLIT; i++)
  {
    FILE *dictFile = dictOutput[i];
    FILE *wordFile = wordOutput[i];
    rewind(dictFile);
    rewind(wordFile);
    //构建哈希字典
  int k = (m + 1) / 2;
  printf("building B+ tree, m=%d, k=%d\n", m, k);
  BTree *btree = btree_init(k);
  int counter = 0;
  while (fgets(line, FILE_LINE_BUFFER, dictFile) != NULL)
  {
    char *find = strchr(line, '\n'); //查找换行符，如果find不为空指针
    Column c;
    c.id = 1;
    strcpy(c.title, line);
    btree_add(btree, &c);
    counter++;
    if(counter >= 10)break;
  }
  printf("\n---btree_traverse begin---%d\n\n", btree->size);
  btree_traverse(btree, print_node);
  printf("---btree_traverse end---\n");

  //搜索
  int hit = 0;
  while (fgets(line, FILE_LINE_BUFFER, wordFile) != NULL)
  {
    char *find = strchr(line, '\n'); //查找换行符，如果find不为空指针
    if (find)
      *find = '\0';
    if (line[0] == '\n' || line[0] == '\0')
    {
      printf("skip an empty line\n");
      continue;
    }

    printf("%s\n", line);
    Column *c = (Column *)malloc(sizeof(Column));
    c->id = 0; //hash(line);
    strcpy(c->title, line);
    Column *result = btree_get_by_value(btree, c);
    if (result != NULL)
    {
      fprintf(resultFile, "%s\n", line);
      hit++;
    }
  }
  fclose(wordFile);
  fclose(dictFile);
  printf("hit %d\n", hit);
  btree_clear(btree);
    total_hits += hit;//search(dictFile, wordFile, resultFile, m);
    remove(key2string(i, "_dict.txt", filenameBuffer));
    remove(key2string(i, "_string.txt", filenameBuffer));
  }
  printf("total hits: %d\n", total_hits);
  fclose(resultFile);
  free(line);
  printf("runtime: %f s, string match: %d\n", (double)((double)(clock() - start_t) / (double)(CLOCKS_PER_SEC)), total_hits);
  printf("--- total using time : %f s\n", (double)((double)(clock() - start_t) / (double)(CLOCKS_PER_SEC)));
  sleep(3);
  return 0;
}