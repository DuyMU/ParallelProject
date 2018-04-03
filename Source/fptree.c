#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <omp.h>
#include "fptree.h"
#include "tract.h"

/*_initTreeRoot:
Input:
	root: root của FP tree
Output:
	ko có
Chức năng:
	khởi tạo root của FP tree*/

void _initTreeRoot(FpNode **root)
{
	(*root) = (FpNode *)malloc(sizeof(FpNode));
	if(!(*root))
	{
		perror("out of memory!!!");
		exit(-1);
	}
	(*root)->field = NULL;
	(*root)->support = 0;
	(*root)->parent = NULL;
	(*root)->eldest = NULL;
	(*root)->sibling = NULL;
	(*root)->same = NULL;
}

/*createFpTree:
Input:
	fptree: cây FP tree
	headTable: bảng headTable của FP tree
	transaction: mảng chứa các giao dịch
	transNum: tổng số giao dịch
Output:
	ko có
Chức năng:
	tạo FP tree dựa vào các giao dịch chứa trong transaction,
	đồng thời cập nhật headTable để kết nối các item giống nhau nằm khác nhánh trong FP tree*/ 

void createFpTree(FpTree *fptree, FpHeadTable *headTable, ItemLink *transaction, int transNum)
{
	int i;
	fptree->headTable = headTable;
	_initTreeRoot(&fptree->root);    /*khởi tạo root*/
	for(i = 0; i < transNum; ++i)    /*ghi từng giao dịch vào FP tree*/
	{
		ItemLink p = transaction[i];
		_insertTree(p, &fptree->root, fptree->headTable);    /*ghi từng item của mỗi giao dịch*/
	}
	
	//printf("Xuat bang headTable sau khi doc xong cac giao dich:\n");
	//_showHeadTable(*(fptree->headTable));
	//printf("Xuat cay fp sau khi doc xong cac giao dich:\n");
	//showFpTree(fptree);
}

/*_matchedCfp:
Input:
	q: node trong cây conditional FP tree
	p: node trong prefix path
Output:
	node có field giống với p, NULL nếu ko có
Chức năng:
	kiểm tra q và các anh em cùng cha của nó trong conditional FP tree có node nào giống field với p ko,
	nếu giống thì tăng support count của node đó lên và trả về node đó*/

FpNode* _matchedCfp(FpNode *q, Pattern p)
{                             /* if matching, node's support++ and return the matched child(true), otherwise NULL(false) */
	for(; q; q = q->sibling){                 /* when using large data set, here may cause some problems, e.g. segemental fault, I don't know why */																							
		if((q->field != NULL) && (p->field != NULL) && (strcmp(q->field, p->field)==0)){
			q->support += p->support;			        /* support increase */
			return q;                             /* find it */
		}
	}
	return NULL;                              /* not matched */
}

/*_insertCfpTree:
Input:
	pt: các node của prefix path
	node: root hiện tại của conditional fp tree
	headTable: dùng để liên kết các item giống nhau nằm khác nhánh
Output:
	ko có
Chức năng:
	thực hiện ghi các nhánh prefix path (đã loại các item ko thỏa ngưỡng support count) vào conditional FP tree,
	nếu có các item trùng với các nhánh khác thì chỉ tăng support count của item có sẵn chứ ko tạo thêm nhánh mới,
	tới khi nào có item khác thì mới rẽ nhánh*/

void _insertCfpTree(Pattern pt, FpNode *node, FpHeadTable *headTable)
{
	if(!pt || pt->field == NULL)return;
	if(node == NULL){
		printf("error parameter, node is null.\n");
		return;
	}
	FpNode *child = node->eldest;
	if((child != NULL) && (child = _matchedCfp(child, pt)));/*if node has first child and match, support++ and get the matched child */
	else{                                     /* else node does not have a child at all or no child(ren) can be matched */
		child = (FpNode *)malloc(sizeof(FpNode));
		if(!child){
			perror("out of memory!!!");exit(-1);
		}
		child->field = (char *)malloc(strlen(pt->field) + 1);
		if(!child->field){
			perror("out of memory!!!");
			exit(-1);
		}
		strcpy(child->field, pt->field);
		child->support = pt->support;           /* its support must be one */
		child->parent = node;                   /* as a child of node */
		child->eldest = NULL;                   /* there is not a child */
		child->sibling = node->eldest;          /* become a sibling of node's child */
		node->eldest = child;                   /* and as the first child (like a stack) */
		_linkSameNode(&child, headTable);       /* link together the nodes which have the same name to the item head table */
	}
	_insertCfpTree(pt->sibling, child, headTable);/* insert the tree recursively */
}

/*_matched:
Input:
	node: một node trong cây
	transactionItem: một item trong giao dịch
Output:
	node có field giống với field của transactionItem (nếu ko có trả về NULL)
Chức năng:
	kiếm xem một node và các anh em cùng cha của nó có node nào giống field với transactionItem ko,
	nếu giống thì tăng support count của node đó lên và trả về node đó*/

FpNode* _matched(FpNode *node, ItemLink transactionItem)
{
	for(; node; node = node->sibling)    /*kiểm tra node và các anh em của nó*/
	{
		if(!strcmp(node->field, transactionItem->field))
		{
			node->support++;    /*nếu có thì tăng support count của node và trả về node đó*/
			return node;
		}
	}
	return NULL; 
}

/*_insertTree:
Input:
	transactionItem: item trong giao dịch được ghi
	parent: node cha của item được thêm vào
	headTable: dùng để liên kết các item giống nhau nằm khác nhánh
Output:
	ko có
Chức năng:
	thực hiện ghi từng item của mỗi giao dịch vào FP tree,
	nếu giao dịch có các item trùng với giao dịch khác thì chỉ tăng support count của item có sẵn chứ ko tạo thêm nhánh mới,
	tới khi nào có item khác thì mới rẽ nhánh*/

void _insertTree(ItemLink transactionItem, FpNode **parent, FpHeadTable *headTable)
{
	int i;
	if(!transactionItem || transactionItem->field == NULL)
		return;
	if((*parent) == NULL)
	{
		printf("error parameter, parent is null.\n");
		return;
	}
	FpNode *child = (*parent)->eldest;
	if((child != NULL) && (child = _matched(child, transactionItem)));
		/*nếu parent có node con và có field khớp với item được thêm, support++ node khớp và trả về node đó*/
	else
	{    /*nếu parent ko có node con hoặc ko có node con khớp thì tạo thêm node con mới*/
		child = (FpNode *)malloc(sizeof(FpNode));
		if(!child)
		{
			perror("out of memory!!!");exit(-1);
		}
		child->field = (char *)malloc(strlen(transactionItem->field) + 1);
		if(!child->field)
		{
			perror("out of memory!!!");exit(-1);
		}
		strcpy(child->field, transactionItem->field);
		child->support = 1;                    /*support count = 1*/
		child->parent = *parent;               /*cha của node mới là parent*/
		child->eldest = NULL;
		child->sibling = (*parent)->eldest;    /*liên kết với con node con của parent (sibling)*/
		(*parent)->eldest = child;             /*node mới trở thành con đầu của parent (eldest)*/
		_linkSameNode(&child, headTable);      /*liên kết node mới với các node khác nhánh cùng field trong FP tree*/
	}
	_insertTree(transactionItem->next, &child, headTable);
	/*tiếp tục thêm item tiếp theo của giao dịch vào cây, parent của item này là item trước đó*/
}

/*_linkSameNode:
Input:
	node: node được liên kết với các node cùng item field
	headTable: headTable của FP tree
Output:
	ko có
Chức năng:
	liên kết node mới với các node có cùng item field trong headTable*/

void _linkSameNode(FpNode **node, FpHeadTable *headTable)
{
	int i;
	for(i = 0; i < headTable->count; ++i)
	{
		if(!strcmp((*node)->field, headTable->lists[i]->field))    /*tìm head có cùng item field với node mới*/
		{
			(*node)->same = headTable->lists[i]->head;    /*liên kết node mới với head*/
			headTable->lists[i]->head = (*node);          /*node mới trở thành head*/
			break;
		}
	}
}

/*createHeadTable:
Input:
	hashTable: mảng các NodeLink, mỗi NodeLink là mảng các item có cùng giá trị hash,
	mỗi node trong nodeLink đại diện cho một loại item kèm support count tổng của item đó trong toàn bộ giao dịch
	hashTableSize = itemsNum: kích thước bảng hash = tổng số item
	headTable:  dùng để liên kết các item giống nhau nằm khác nhánh
	numExceeded: số item thỏa ngưỡng support count
Output:
	ko có
Chức năng:
	tạo bảng head của các item thỏa ngưỡng support count,
	mỗi head sẽ đại diện cho một loại item kèm support count tổng của item đó,
	head sẽ liên kết các item giống nhau trong FP tree*/

void createHeadTable(NodeLink *hashTable, int hashTableSize, FpHeadTable *headTable, int numExceeded)
{
	int i;
	headTable->count = 0;
	headTable->lists = (FpListLink *)calloc(numExceeded, sizeof(FpListLink));
	for(i = 0; i < hashTableSize; ++i)    /*duyệt toàn bộ các nodeLink của bảng hash*/
	{
		NodeLink p = hashTable[i];
		for(; p; p = p->next)   /*duyệt toàn bộ node trong mỗi nodeLink*/
		{
			if(p->exceeded == 'N')continue;    /*nếu node ko thỏa ngưỡng support count thì bỏ qua*/
			FpListLink q = (FpListLink)malloc(sizeof(FpList));   /*nếu thỏa, thì tạo head của item đó trong headTable*/
			if(!q)
			{
				perror("out of memory!!!");exit(-1);
			}
			q->field = (char *)malloc(strlen(p->field) + 1);
			if(!q->field)
			{
				perror("out of memory!!!");exit(-1);
			}
			strcpy(q->field, p->field);    /*copy nội dung item từ hashTable*/
			q->support = p->support;
			q->head = NULL;                
				/*head khởi tạo là NULL, về sau sẽ được dùng để liên kết các item cùng field trong FP tree*/
			headTable->lists[headTable->count++] = q;
		}
	}
	_sortHeadTable(headTable);
	//_showHeadTable(*headTable);
}
	

/*_sortHeadTable:
Input:
	headTable: headTable
Output:
	ko có
Chức năng:
	sau khi tạo headTable thì tiến hành sắp xếp theo chiều giảm dần support count,
	headTable->list[0] là danh sách liên kết của item có support count lớn nhất,
	nếu hai item bằng support count thì xếp theo tăng dần alphabe*/

void _sortHeadTable(FpHeadTable *headTable)
{   
	/*quick sort
	int i = left, j = right;
	FpListLink temp;
	FpListLink pivot = headTable->lists[(left + right) / 2];
	while (i <= j)
	{
		while (headTable->lists[i]->support > pivot->support)
			i++;
		while (headTable->lists[j]->support < pivot->support)
			j--;
		if (i <= j)
		{
			temp = headTable->lists[i];
			headTable->lists[i] = headTable->lists[j];
			headTable->lists[j] = temp;
			i++;
			j--;
		}
	}
	
	// recursion
	if (left < j)
		_sortHeadTable(headTable, left, j);
	if (i < right)
		_sortHeadTable(headTable, i, right);*/
	
	/* select sort */
	int i, j, max;
	FpListLink temp;
	for (i = 0; i < headTable->count; ++i)
	{
		max = i;
		for (j = i + 1; j < headTable->count; ++j)
		{
			if ((headTable->lists[max]->support < headTable->lists[j]->support) 
				|| ((headTable->lists[max]->support == headTable->lists[j]->support) && (strcmp(headTable->lists[max]->field, headTable->lists[j]->field) > 0)))
			{
				max = j;
			}
		}
		
		if (max != i)
		{
			temp = headTable->lists[max];
			headTable->lists[max] = headTable->lists[i];
			headTable->lists[i] = temp;
		}
	}
	
	/*Binary sort
	int i, j;
	int low, high, m;
	for(i = 1; i < headTable->count; ++i)
	{
		FpListLink p = headTable->lists[i];
		low = 0; high = i-1;
		while(low <= high)
		{
			m = (low + high)/2;
			if(p->support > headTable->lists[m]->support 
					|| (p->support == headTable->lists[m]->support && strcmp(p->field, headTable->lists[m]->field) < 0)
				)
				high = m - 1;
			else low = m + 1;
		}
		for(j = i - 1; j >= high; --j)
			headTable->lists[j+1] = headTable->lists[j];
		headTable->lists[high+1] = p;
	}*/
}

/*showFpTree:
Input:
	fptree: cây FP tree
Output:
	ko có
Chức năng:
	xuất cây FP*/

void showFpTree(FpTree *fptree)
{
	FpHeadTable *ht = fptree->headTable;
	FpNode *r = fptree->root;
	FpListLink *lists = ht->lists;
	int i, c = ht->count;
	for(i = 0; i < c; ++i){
		printf("---[%d]---:\n", i);
		FpListLink t = lists[i];
		FpNodeLink p = t->head;
		while(p){
			printf("field=[%s:%d],parentfield=[%s:%d],eldest=[%s:%d],sibling=[%s:%d],same=[%s:%d]\n",
									p->field, p->support, p->parent->field,p->parent->support, 
									p->eldest==NULL?"-":p->eldest->field,p->eldest==NULL?0:p->eldest->support, 
									p->sibling==NULL?"-":p->sibling->field, p->sibling==NULL?0:p->sibling->support,
									p->same == NULL?"-":p->same->field,p->same == NULL?0:p->same->support);
			p = p->same;
		}
	}
}

/*_singlePath:
Input:
	fptree: cây FP tree
Output:
	trả về 1 nếu cây chỉ có 1 nhánh, trả về 0 nếu ngược lại
Chức năng:
	hàm kiểm tra xem FP tree chỉ có 1 đường đi duy nhất hay ko (tức cây ko rẻ nhánh)*/

int _singlePath(FpTree *fptree)
{
	assert(fptree);
	FpHeadTable *ht = fptree->headTable;
	FpNode *p = fptree->root;
	if(ht->count == 1)return 1;    /*chỉ có 1 node*/
	while(p)
	{
		if(p->eldest && p->eldest->sibling)    /*có ít nhất 2 node con*/
			return 0;
		p = p->eldest;
	}
	return 1;
}

/*_copyFpNode:
Input:
	newNode: node mới
	node: node được sao chép
Output:
	ko có
Chức năng:
	sao chép nội dung của một FpNode sang một FpNode khác,
	chỉ sao chép tên và support count,
	ko sao chép link tới cha, con, anh em...*/

void _copyFpNode(FpNodeLink *newNode, const FpNode node)
{
	(*newNode)->field = (char *)malloc(strlen(node.field) + 1);
	if(!(*newNode)->field)
	{
		perror("out of memory!!!");exit(-1);
	}
	strcpy((*newNode)->field, node.field);
	(*newNode)->support = node.support;
	(*newNode)->parent = NULL;
	(*newNode)->eldest = NULL;
	(*newNode)->sibling = NULL;
	(*newNode)->same = NULL;
}

/*fpgrowth: 
Input:
Output:
Chức năng:
	thực hiện giải thuật fp growth sinh các frequent itemsets,
	hàm này sẽ chia cây fp tree ra thành các conditional fp tree nhỏ hơn để duyệt tìm frequent itemsets,
	chạy đệ qui để chia cây*/

void fpgrowth(FpTree *fptree, FpNode *a, int support, FILE *fp)
{
	int i;
	if(!fptree)return;
	
	FpHeadTable *ht = fptree->headTable;
	#pragma omp parallel shared(fp, fptree, support, a, ht, NumberOfFrequentSets)
	{
		if(_singlePath(fptree))
		{ /*nếu fp tree chỉ có 1 đường thì chắc chắn các itemset trên đường đó đều là frequent,
									do trước đó đã loại các infrequent 1-itemset*/
			_generatePatterns(a, fptree, fp);       /* output frequent item sets */
		} 
		else 
		{				 
			#pragma omp for private(i) schedule(dynamic, 1)
			for(i = ht->count - 1; i >= 0; --i)
			{
				FpListLink aiLink = ht->lists[i];	/*duyệt từ item có support count thấp nhất, tức các node dưới cùng của fp tree*/
				_generateOnePattern(aiLink, a, fp);   /* output frequent item sets at first */
				Pattern b = (Pattern)malloc(sizeof(FpNode));/* and then generate the required pattern b */
				/* b là chuỗi suffix của cây đang xét
				VD: nếu itemset d - e là frequent, tiếp tục xét prefix path của d - e, thì d - e là chuỗi suffix của cây đang xét
				Nếu root của cây cfptree đang xét = NULL thì free b*/
				if(!b)
				{
					printf("*** out of memory! *** failed to create pattern b \n");
					exit(-1);
				}
				_copyFpNode(&b, *(aiLink->head));     /* b is ai now */
				b->eldest = a;                        /* b = (ai U a) TODO check if we can use a, with assuming ok */
				FpTree *cfptree = (FpTree *)malloc(sizeof(FpTree));
				if(!cfptree)
				{
					printf("*** out of memory! *** failed to create conditional fptree \n");
					exit(-1);
				}
				/* we can use ai rather than b to create c.f.p. tree, 
					 b, containing (ai U a), will be used in fpgrowth recursively. 
					 i is ai's index in headTable */
				_buildCondFpTree(cfptree, fptree, i, support);
				/*printf("-------------[%d] fptree\n", i);
				showFpTree(cfptree);
				_generatePatterns(NULL, cfptree, stdout);printf("----test---\n");//return;*/
				if(cfptree->root){
					fpgrowth(cfptree, b, support, fp);
				}
				free(b->field);
				free(b);
			}
		}
	}
	freeFpTree(fptree);
}

/*_buildCondFpTree:
Input:
	cfptree: conditional FP tree cho item ở vị trí idx trong headTable
	fptree: cây FP tree đang xét
	idx: vị trí của item đang xét trong headTable
	support: ngưỡng support count
Output:
	ko có
Chức năng:
	tạo conditional FP tree cho item ở vị trí idx trong headTable*/

void _buildCondFpTree(FpTree *cfptree, FpTree *fptree, int idx, int support)
{
	PatternBase pbHead = (PatternBase)malloc(sizeof(FpNode));    /* every suffix pattern has its own p.b. */
	if(!pbHead){
		perror("can not create PatternBase");
		exit(-1);
	}
	pbHead->field = NULL;
	pbHead->support = 0;
	pbHead->parent = NULL;
	pbHead->eldest = NULL;
	pbHead->sibling = NULL;
	pbHead->same = NULL;

	if(NULL == (cfptree->headTable = (FpHeadTable *)malloc(sizeof(FpHeadTable)))){
		perror("can not create head table ");
		exit(-1);
	}
	cfptree->headTable->lists = NULL;
	_buildCondPatternBase(pbHead, fptree, idx);    /*tạo prefix path cho item đang xét*/
	//printf("-------------[%d]---\n", idx);   /*for test*/
	//_showPatternBase(pbHead);    /*for test*/
	//printf("Done buildCondPatternBase\n");
	_buildHeadTable(cfptree->headTable, pbHead, support);    /*tạo headTable cho conditional FP tree*/
	//_showHeadTable(*(cfptree->headTable));
	//printf("Done buildHeadTable\n");
	_initTreeRoot(&cfptree->root);
	PatternBase pbh = pbHead;
	for(pbh = pbh->eldest; pbh; pbh = pbh->eldest){
		_insertCfpTree(pbh->sibling, cfptree->root, cfptree->headTable); 
		/*insert các node của prefix path vào conditional
		FP tree, bỏ node eldest vì đây là node ta đang xét prefix path của nó*/
	}
	
	freePatternBase(pbHead);
	//showFpTree(cfptree);
	//_showPatternBase(pbHead);
	//TODO free the memory!!!
}

/*_buildHeadTable: 
Input:
	ht: headTable của conditional FP tree (đã loại các item ko thỏa ngưỡng support count trong prefix path),
	các item trong headTable xếp theo thứ tự giảm dần support count
	pbh: root của prefix path để liên kết các itemset trong prefix path
	support: ngưỡng support count
Output:
	ko có
Chức năng:
	tạo headTable cho conditional FP tree*/

void _buildHeadTable(FpHeadTable *ht, PatternBase pbh, int support)
{
	NodeLink p, listHead = (NodeLink)malloc(sizeof(Node));/* put all p.b. into a link list for compute number and support of the same items, simple idea */
	if(!listHead){
		perror("can not create link list head ");exit(-1);
	}
	int size = _assistUtil(&listHead, pbh, support);/* build a list, which has a head node */
	//printf("Done assistUtil\n");
	ht->count = 0;                            /* initialize the number of lists */
	
	//_showAssistList(listHead);/* for test */
	//printf("Kiem tra prefix path sau khi bo bot item ko thoa:\n");
	//_showPatternBase(pbh);// for test
	
	if(size <= 0)
	{
		freeListHead(listHead);
		return;                      /* need not to create head table any more */
	}
	
	ht->lists = (FpListLink *)calloc(size, sizeof(FpListLink));
	if(ht->lists == NULL){
		perror("can not create head table lists ");exit(-1);
	}
	for(p = listHead->next; p; p = p->next){  /* I'm NOT sure whether the link list is descend order or not */
		if(p->exceeded == 'N')continue;         /* so, traverse it then sort the head table again, ignoring the unsupport one */
		FpListLink q = (FpListLink)malloc(sizeof(FpList));/* else create a node head */
		if(!q){
			perror("out of memory!!!");exit(-1);
		}
		q->field = (char *)malloc(strlen(p->field) + 1);/* item name */
		if(!q->field){
			perror("out of memory!!!");exit(-1);
		}
		strcpy(q->field, p->field);             /* copy from hash table */
		q->support = p->support;                /* and support */
		q->head = NULL;                         /* initial point to null, ready to link nodes of freqent pattern tree */
		ht->lists[ht->count++] = q;             /* assign to node table pointer */
	}
	_sortHeadTable(ht);                       /* here, we sort it again, time consuming :( */
	freeListHead(listHead);
	//_showHeadTable(*ht);// for test
}

/*_assistUtil:
Input:
	list = listHead: danh sách liệt kê các item trong prefix path
	(bản thân con trỏ listHead ko chứa item nào mà list->next mới là item đầu tiên)
	pbh: root của prefix path để liên kết các itemset trong prefix path
	support: ngưỡng support count
Output:
	số item thỏa ngưỡng support count trong prefix path (numOfExceeded)
Chức năng:
	sau khi tạo được prefix path từ hàm _buildCondPatternBase,
	ta tạo một danh sách liệt kê các item có trong prefix path,
	rồi kiểm tra xem item nào ko thỏa ngưỡng support,
	sau đó dựa vào numOfExceeded (số item thỏa ngưỡng support) và listHead->support (số item trong prefix path),
	nếu có item nào ko thỏa (numOfExceeded < listHead->support) thì xóa item đó bằng hàm _prunePatternBase*/

int _assistUtil(NodeLink *list, PatternBase pbh, int support)
{/* build a assistant list for building the head table,
		prune the p.b., w.r.t., delete the unsupport nodes int p.b.,
		return the number of nodes exceeding the support as well */
	int numOfExceeded = 0;
	NodeLink listHead = (*list);                 /* initialize the link list head */
	listHead->field = NULL;
	listHead->support = 0;                    /* regard as counter of the distinct nodes in the list */
	listHead->exceeded = 'A';                 /* nothing, just initialize it */
	listHead->next = NULL;
	PatternBase q = pbh->eldest;
	PatternBase p = NULL;
	//printf("Start assistUtil\n");
	while(q){
		p = q->sibling;                         /* sth segemental fault may occur here */
		while(p){
			NodeLink s = listHead->next;
			NodeLink r = listHead;
			for(; s; s = s->next, r = r->next){   /* r->next is s */
				if(!strcmp(p->field, s->field)){
					s->support += p->support;
					if(s->exceeded == 'N' && support <= s->support){/* if someone comes up to the threshold */
						s->exceeded = 'Y';              /* mark it */
						++numOfExceeded;                /* and increase the number */
					}
					break;
				}
			}
			if(!s){                               /* there is not a same node */
				listHead->support++;                /* regard head's support as counter of the list */
				if(NULL == (r->next = (NodeLink)malloc(sizeof(Node)))){
					perror("can not create Node ");
					exit(-1);
				}
				r->next->field = (char *)malloc(strlen(p->field) + 1);
				if(!r->next->field){
					perror("out of memory!!!");exit(-1);
				}
				strcpy(r->next->field, p->field);
				r->next->support = p->support;
				r->next->next = NULL;
				if(support <= p->support){
					r->next->exceeded = 'Y';
					++numOfExceeded;                  /* number of nodes exceeding the support */
				} else {
					r->next->exceeded = 'N';
				}
			}
			p = p->sibling;
		}
		q = q->eldest;                          /* next set */
	}
	//printf("Start prunePatternBase\n");
	if(numOfExceeded < listHead->support){    /* support nodes is less than total ones */
		_prunePatternBase(pbh, listHead, listHead->support - numOfExceeded);
	}
	
	return numOfExceeded;
}

/*_prunePatternBase:
Input:
	pbh: root của prefix path để liên kết các itemset trong prefix path
	list = listHead: danh sách liệt kê các item trong prefix path
	(bản thân con trỏ listHead ko chứa item nào mà list->next mới là item đầu tiên)
	n: số item ko thỏa ngưỡng support count trong prefix path
Output:
	ko có
Chức năng:
	xóa các item ko thỏa ngưỡng support count trong prefix path*/

void _prunePatternBase(PatternBase pbh, NodeLink list, int n)
{                                           /* delete n unsupport node(s) in the p.b. */
	Item items[n];
	NodeLink p = list->next;
	int i = 0;
	
	/*lưu tên các item ko thỏa vào mảng items[n] để sau đó xóa các item này trong prefix path*/
	for(; p; p = p->next)
	{
		if(p->exceeded == 'Y')
		{
			continue;
		}
		items[i].field = (char *)malloc(strlen(p->field) + 1);
		if(!items[i].field)
		{
			perror("out of memory!!!");exit(-1);
		}
		strcpy(items[i++].field, p->field);
	}
	
	/*xóa các item ko thỏa ngưỡng support count trong prefix path, ở đây sẽ ko xét các eldest
	vì đây chính là item đang xét conditional FP tree của nó nên chắc chắn thỏa ngưỡng support count*/
	PatternBase r = pbh->eldest;
	int del = 0;
	for(; r; r = r->eldest)
	{
		PatternBase s = r->sibling;
		PatternBase x = r;
		while (s)
		{
			for(i = 0; i < n; ++i)
			{
				if(!strcmp(items[i].field, s->field))
				{
					//printf("Xoa %s\n", s->field);
					del = 1;
					break;
				}
			}
			
			if (del)
			{
				x->sibling = s->sibling;
				free(s->field);
				free(s);
				s = x->sibling;
				del = 0;
			}
			else
			{
				s = s->sibling;
				x = x->sibling;
			}
		}
	}
	
	// có thể song song
	//Xóa các phần tử field của mảng item
	for (i = 0; i < n; ++i)
	{
		free(items[i].field);
	}
}

/*_showPatternBase:
Input:
	pbHead: root của prefix path để liên kết các itemset trong prefix path
Output:
	ko có
Chức năng:
	hàm xuất conditional fp tree hoặc prefix path để kiểm tra,
	ở đây khi xuất conditional fp tree, hàm này bỏ qua ko xuất item đầu tiên của mỗi itemset do nó chính là item đang xét,
	nên khi tìm conditional fp tree thì sẽ ko quan tâm item này*/

void _showPatternBase(PatternBase pbHead)
{
	printf("------------\n");
	PatternBase q = pbHead;
	for(q = q->eldest; q; q = q->eldest){
		PatternBase p = q->sibling;
		printf("***\n");
		while(p){
			printf("field=[%s:%d],sibling=[%s],eldest=[%s]\n",
									p->field, p->support, 
									p->sibling==NULL?"-":p->sibling->field, 
									p->eldest==NULL?"-":p->eldest->field
									);
			p = p->sibling;
		}
	}
}

/*_showAssistList: 
Input:
	list = listHead: danh sách liệt kê các item trong prefix path
Output:
	ko có
Chức năng:
	xuất listHead để kiểm tra*/

void _showAssistList(NodeLink list)
{
	NodeLink p = list->next;
	printf("number of nodes in list:[%d]\n", list->support);
	while(p){
		printf("[%s:%d][%c]\n", p->field, p->support, p->exceeded);
		p = p->next;
	}
}

/*_showHeadTable: 
Input:
	ht = headTable: bảng headTable
Output:
	ko có
Chức năng:
	xuất headTable để kiểm tra*/

void _showHeadTable(FpHeadTable ht)
{
	printf("number of lists in head table:[%d]\n", ht.count);
	int i;
	for(i = 0; i < ht.count; ++i){
		printf("[%s:%d]\n", ht.lists[i]->field, ht.lists[i]->support);
	}
}

/*_buildCondPatternBase:
Input:
	pbHead: root của prefix path, node này ko chứa nội dung gì
	fptree: cây FP tree đang xét
	idx: vị trí của item đang xét trong headTable
Output:
	ko có
Chức năng:
	sinh prefix path cho item có vị trí idx trong headTable
	Qui tắc của prefix path:
	eldest của root sẽ chứa item đầu của 1 itemset chứa item ở vị trí idx (thực chất item đầu luôn là item ở vị trí idx),
	từ eldest ta duyệt các sibling sẽ được các item còn lại trong set đó,
	eldest của eldest sẽ chứa item đầu của một set khác có chứa item ở vị trí idx, cứ thế đến hết*/

void _buildCondPatternBase(PatternBase pbHead, FpTree *fptree, int idx)
{/* use FpNode to create nodes of pattern bases, 
		->eldest as linking the next set, while 
		->sibling as next pattern base in the same set,
		so, pattern base pb need have a head node with null field
		*/
	FpNodeLink p = fptree->headTable->lists[idx]->head;    /* the first node pointered by idxth head table node in fptree */
	PatternBase q = pbHead;
	while(p)    /* traverse the chain of node-links with same item name */
	{
		FpNodeLink r = (FpNode *)malloc(sizeof(FpNode));
		if(!r)
		{
			printf("*** out of memory! *** can not create a set link node \n");
			exit(-1);
		}
		_copyFpNode(&r, *p);         /* r's support is assigned absolutely p's support */
		r->eldest = q->eldest;    /* link the sets with ->eldest */
		q->eldest = r;
		FpNodeLink s = p;
		s = s->parent;               /* create conditional pattern base starts from it's parent */
		while(s && s->field)    /* traverse the path along with ->parent, no root(->field is null)*/
		{
			FpNodeLink t = (FpNode *)malloc(sizeof(FpNode));
			if(!t){
				printf("*** out of memory! *** can not create a same node \n");
				exit(-1);
			}
			_copyFpNode(&t, *s);
			t->support = r->support;    /* must be the support of node in chain of node-links */
			t->sibling = r->sibling;    /* link to the same set with ->sibling */
			r->sibling = t;
			s = s->parent;
		}
		p = p->same;
	}
}

/*_generatePatterns: sinh ra các frequent itemsets và xuất ra,
duyệt từ trên xuống, VD: với ngưỡng support là 10% và tập giao dịch sau: a b c, a b, a b, a
ta được kết quả các frequent itemsets theo thứ tự sau: a - a b - a b c - b - b c - c*/

void _generatePatterns(FpNode *a, FpTree *fptree, FILE *fp)
{
	int support = MAX_SUPPORT;
	int cnt = 0;                              /* for combination */
	int i, j;
	int n = fptree->headTable->count;         /* number of nodes in the single path */
	FpNode *path = fptree->root;
	FpNode *cur = path;
	FpNode *b;
	FpNode *b_cur;
	FpNode *a_cur;
	
	char header[] = "Output: ";
	char output[200];
	while(n--)
	{	// traverse whole path which has at least one node
		for(i = 0, b = cur->eldest; i <= n; ++i)
		{	// for each combination (denoted as b) of nodes in the path
			b_cur = b;
			strcpy(output, header);
			for(support = MAX_SUPPORT, j = 0; j <= i && b_cur; ++j, b_cur = b_cur->eldest)
			{	// traverse one combination
				if (support > b_cur->support)
					support = b_cur->support;	// minimum support count of nodes in b
				strcat(output, b_cur->field);
				strcat(output, " ");
			}
			a_cur = a;
			while(a_cur)
			{	// traverse the suffix pattern a
				strcat(output, a_cur->field);
				strcat(output, " ");
				a_cur = a_cur->eldest;
			}
			#pragma omp critical
			{
				fprintf(fp, "%s[%d]\n", output, support);
				++NumberOfFrequentSets;
			}
		}
		cur = cur->eldest;	// cur and n have synchronous change
	}
	
	
	/*#pragma omp critical
	{
		while(n--)
		{	// traverse whole path which has at least one node
			for(i = 0, b = cur->eldest; i <= n; ++i)
			{	// for each combination (denoted as b) of nodes in the path
				FpNode *b_cur = b;
				fprintf(fp, "Output: ");
				for(support = MAX_SUPPORT, j = 0; j <= i && b_cur; ++j, b_cur = b_cur->eldest)
				{	// traverse one combination
					if(support > b_cur->support)support = b_cur->support;	// minimum support count of nodes in b
					fprintf(fp, "%s ", b_cur->field);
				}
				FpNode *a_cur = a;
				while(a_cur)
				{	// traverse the suffix pattern a
					fprintf(fp, "%s ", a_cur->field);
					a_cur = a_cur->eldest;
				}
				fprintf(fp, "[%d]\n", support);
				//#pragma omp atomic
				++NumberOfFrequentSets;
			}
			cur = cur->eldest;	// cur and n have synchronous change
		}
	}*/
}

/*_generateOnePattern:
Input:
Output:
Chức năng:
	sinh một frequent itemset*/

void _generateOnePattern(FpListLink ai, FpNode *a, FILE *fp)
{
	FpNode *p = a;
	char output[200] = "Output: ";
	strcat(output, ai->field);
	while(p)
	{
		strcat(output, " ");
		strcat(output, p->field);
		p = p->eldest;
	}
	#pragma omp critical
	{
		fprintf(fp, "%s [%d]\n", output, ai->support);
		++NumberOfFrequentSets;
	}
	
	/*#pragma omp critical
	{
		FpNode *p = a;
		fprintf(fp, "Output: %s ", ai->field);
		while(p)
		{
			fprintf(fp, "%s ", p->field);
			p = p->eldest;
		}
		fprintf(fp, "[%d]\n", ai->support);
		//#pragma omp atomic
		++NumberOfFrequentSets;
	}*/
}

/*freeHeadTable:
Input:
	headTable
Output:
	ko có
Chúc năng:
	giải phóng headTable

*/
void freeHeadTable(FpHeadTable *headTable)
{
	if (headTable == NULL)
		return;
		
	// có thể song song
	int i;
	for (i = 0; i < headTable->count; ++i)
	{
		free(headTable->lists[i]->field);
		free(headTable->lists[i]);
	}
	
	free(headTable->lists);
	free(headTable);
}

/*freeFpTree:
Input:
	FpTree
Output:
	ko có
Chúc năng:
	giải phóng FpTree kèm headTable của nó*/

//C1: dùng đệ qui
/*void freeFpTree(FpTree *fptree)
{
	if (fptree == NULL)
		return;
	freeFpNode(fptree->root->eldest);
	free(root);
	freeHeadTable(fptree->headTable);
	free(fptree);
}*/

//C2: ko dùng đệ qui
void freeFpTree(FpTree *fptree)
{
	if (fptree == NULL)
		return;
		
	int i;
	FpNode *p, *del;
	FpHeadTable *ht = fptree->headTable;
	
	// có thể song song
	for (i = 0; i < ht->count; ++i)
	{
		p = ht->lists[i]->head;
		while (p)
		{
			ht->lists[i]->head = p->same;
			free(p->field);
			free(p);
			p = ht->lists[i]->head;
		}
		free(ht->lists[i]->field);
		free(ht->lists[i]);
	}
	
	if (fptree->root != NULL)
	{
		free(fptree->root);
	}
	
	if (ht->lists != NULL)
	{
		free(ht->lists);
	}
	
	free(ht);
	free(fptree);
}

/*freeFpNode:
Input:
	node
Output:
	ko có
Chức năng:
	dùng trong hàm freeFpTree đệ qui, freePatternBase*/
	
void freeFpNode(FpNode *node)
{
	if (node == NULL)
		return;
	
	freeFpNode(node->eldest);
	freeFpNode(node->sibling);
	
	free(node->field);
	free(node);
}


/*freeListHead:
Input:
	listHead
Output:
	ko có
Chức năng:
	giải phóng listHead dùng trong hàm tạo headTable cho conditional FPtree*/

void freeListHead(NodeLink listHead)
{
	if (listHead == NULL)
		return;
		
	Node *p = listHead->next;
	Node *del = NULL;
	while (p)
	{
		del = p;
		p = p->next;
		free(del->field);
		free(del);
	}
	
	free(listHead);
}

/*freePatternBase:
Input:
	pbHead
Output:
	ko có
Chức năng:
	giải phóng prefix path*/

void freePatternBase(PatternBase pbHead)
{
	if (pbHead == NULL)
		return;
		
	freeFpNode(pbHead->eldest);
	free(pbHead);
}

/*freeSuffixPattern:
Input:
	suffix
Output:
	ko có
Chức năng:
	giải phóng chuỗi suffix khi xuất frequent itemset*/

void freeSuffixPattern(Pattern suffix)
{
	Pattern p = suffix;
	Pattern del;
	while (p)
	{
		del = p;
		p = p->eldest;
		free(del->field);
		free(del);
	}
}
	
	
	
	
	
	