#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <omp.h>
#include "fptree.h"
#include "tract.h"

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
}

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
}

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
}

void _sortHeadTable(FpHeadTable *headTable)
{  
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
}

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
				//printf("----%d\n", omp_get_num_threads());
				FpListLink aiLink = ht->lists[i];	/*duyệt từ item có support count thấp nhất, tức các node dưới cùng của fp tree*/
				_generateOnePattern(aiLink, a, fp);   /* output frequent item sets at first */
				Pattern b = (Pattern)malloc(sizeof(FpNode));/* and then generate the required pattern b */
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

void _buildHeadTable(FpHeadTable *ht, PatternBase pbh, int support)
{
	NodeLink p, listHead = (NodeLink)malloc(sizeof(Node));/* put all p.b. into a link list for compute number and support of the same items, simple idea */
	if(!listHead){
		perror("can not create link list head ");exit(-1);
	}
	int size = _assistUtil(&listHead, pbh, support);/* build a list, which has a head node */
	//printf("Done assistUtil\n");
	ht->count = 0;                            /* initialize the number of lists */
	
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
}

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

	for (i = 0; i < n; ++i)
	{
		free(items[i].field);
	}
}

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

void _showAssistList(NodeLink list)
{
	NodeLink p = list->next;
	printf("number of nodes in list:[%d]\n", list->support);
	while(p){
		printf("[%s:%d][%c]\n", p->field, p->support, p->exceeded);
		p = p->next;
	}
}

void _showHeadTable(FpHeadTable ht)
{
	printf("number of lists in head table:[%d]\n", ht.count);
	int i;
	for(i = 0; i < ht.count; ++i){
		printf("[%s:%d]\n", ht.lists[i]->field, ht.lists[i]->support);
	}
}

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
}

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
}

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

void freeFpNode(FpNode *node)
{
	if (node == NULL)
		return;
	
	freeFpNode(node->eldest);
	freeFpNode(node->sibling);
	
	free(node->field);
	free(node);
}

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

void freePatternBase(PatternBase pbHead)
{
	if (pbHead == NULL)
		return;
		
	freeFpNode(pbHead->eldest);
	free(pbHead);
}

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
	
	
	
	
	
	