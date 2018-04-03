#ifndef _FPTREE_H
#define _FPTREE_H
#include "tract.h"

#define MAX_SUPPORT 2147483647              /* maximum integer in 32 bits Linux */
extern int NumberOfFrequentSets;
typedef struct _FpNode         /*node trong cây FP tree*/
{
	char           *field;     /*tên item*/
	int            support;    /*support count*/
	struct _FpNode *parent;    /*node cha*/
	struct _FpNode *eldest;    /*node con đầu tiên (các node con tiếp theo sẽ được liên kết bởi sibling của node con đầu)*/
	struct _FpNode *sibling;   /*node có cùng cha*/
	struct _FpNode *same;      /*node có cùng tên item (cùng field)*/
} FpNode, *FpNodeLink, *Pattern, *PatternBase;

/*PatternBase có thể hiểu là prefix path*/

typedef struct             /*node liên kết các item cùng tên (mỗi node được xem là một danh sách item cùng tên)*/
{
	char       *field;     /*tên item*/
	int        support;    /*support count tổng cộng của các item cùng tên trong FP tree*/
	FpNodeLink head;       /*head node của một loại item, từ head này có thể duyệt tất cả item cùng tên trong FP tree*/
} FpList, *FpListLink;

typedef struct
{                         /*headTable: bảng chứa danh sách liên kết các item cùng tên (đại diện cho mỗi danh sách là head)*/
	FpListLink *lists;    /*mảng các danh sách liên kết các item cùng tên*/
	int        count;     /*số danh sách*/
} FpHeadTable;

typedef struct
{                              /*FP tree*/
	FpHeadTable *headTable;    /*headTable của FP tree đó*/
	FpNode      *root;         /*root: node này ko chứa item mà chỉ để liên kết các nhánh của FP tree*/
} FpTree;

static FpNode* _matched(FpNode *node, ItemLink transactionItem);// x
static void _initTreeRoot(FpNode **root);// x
static void _insertTree(ItemLink transactionItem, FpNode **parent, FpHeadTable *headTable);// x
static void _linkSameNode(FpNode **node, FpHeadTable *headTable);// x
extern void createHeadTable(NodeLink *hashTable, int hashTableSize, FpHeadTable *headTable, int numExceeded);// x
extern void _sortHeadTable(FpHeadTable *headTable);// x
extern void createFpTree(FpTree *fptree, FpHeadTable *headTable, ItemLink *transaction, int transNum);// x
static void _buildCondFpTree(FpTree *cfptree, FpTree *fptree, int idx, int support);//
static void _buildCondPatternBase(PatternBase pbh, FpTree *fptree, int idx);// x
static void _buildHeadTable(FpHeadTable *ht, PatternBase pbh, int support);// x
static int  _assistUtil(NodeLink *list, PatternBase pbh, int support);// x
static void _prunePatternBase(PatternBase pbh, NodeLink list, int n);// x
static void _insertCfpTree(Pattern pt, FpNode *node, FpHeadTable *headTable);//
static FpNode* _matchedCfp(FpNode *q, Pattern p);//
extern void fpgrowth(FpTree *fptree, FpNode *node, int support, FILE *fp);
static int  _singlePath(FpTree *fptree);// x
static void _generatePatterns(FpNode *a, FpTree *fptree, FILE *fp);
static void _generateOnePattern(FpListLink ai, FpNode *a, FILE *fp);
static void _copyFpNode(FpNodeLink *newNode, const FpNode node);//
extern void showFpTree(FpTree *fptree);// x
static void _showPatternBase(PatternBase pb);// x
static void _showAssistList(NodeLink list);// x
static void _showHeadTable(FpHeadTable ht);// x
void freeHeadTable(FpHeadTable *headTable);// x
void freeFpTree(FpTree *fptree);// x
void freeFpNode(FpNode *node);// x
void freeListHead(NodeLink listHead);// x
void freePatternBase(PatternBase pbHead);// x
void freeSuffixPattern(Pattern suffix);// x
#endif
