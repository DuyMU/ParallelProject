#ifndef _TRACT_H
#define _TRACT_H

#define BUF_SIZE 1024        /* khởi tạo 1k buffer cho getline() */
#define DMN_SPRT ' '        /* khoảng cách giữa các item */
#define RCD_SPRT '\n'        /* khoảng cách giữa các đơn hàng */

typedef struct Node
{
    char        exceeded;    /* exceeded = 'Y' nếu thỏa ngưỡng support count, = 'N' nếu ngược lại */
    char        *field;      /* tên node (tên item) */
    int         support;     /* support count*/
    struct Node *next;       /* con trỏ tới node tiếp theo */
} Node, *NodeLink;           /* node trong itemset */

typedef struct Item
{                        
    char        *field;      /* tên item */
    struct Item *next;       /* con trỏ tới item tiếp theo */
} Item, *ItemLink;           /* item */ 

extern void loadTransactionFile(FILE *file, ItemLink *transaction, NodeLink *hashTable, int support, int transNum, int itemsNum);
extern int  hash(char *field, int hashTableSize);
extern void readTransaction(ItemLink *transaction, int idx,  NodeLink *hashTable, int support, char *transBuf, int itemsNum);
extern void createTransaction(ItemLink *transaction, int idx, char *field);
extern void deleteUnsupport(ItemLink *transaction, int transNum, int support, NodeLink *hashTable, int hashTableSize);
extern void sortEveryTransaction(ItemLink *transaction, int transNum, NodeLink *hashTable, int hashTableSize, int support);
extern void selectSort(ItemLink *transactionHead, NodeLink *hashTable, int hashTableSize, int support);
extern void createHashTable(NodeLink *hashTable, char *field, int support, int hashTableSize);
extern int getNumOfExceeded(NodeLink *hashTable, int hashTableSize);
extern int getSupport(NodeLink *hashTable, int hashTableSize, char *field);
void freeTransaction(ItemLink *transaction, int transNum);
void freeHashTable(NodeLink *hashTable, int hashTableSize);
#endif
