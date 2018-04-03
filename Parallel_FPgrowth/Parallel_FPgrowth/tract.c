/*File: tract.c*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <omp.h>
#include "tract.h"

/*loadTransactionFile:
Input:
	file: file dữ liệu
	transaction: mảng các ItemLink, mỗi ItemLink là một giao dịch
	hashTable: mảng các NodeLink, mỗi NodeLink là mảng các item có cùng giá trị hash
	support: ngưỡng support count
	itemsNum: tổng số các loại item (các loại mặt hàng)
Output:
	ko có
Chức năng:
	đọc file dữ liệu chứa các giao dịch, mỗi dòng của file là một giao dịch, sau đó ghi các giao dịch vào transaction
	và tạo hashTable*/

void loadTransactionFile(FILE *file, ItemLink *transaction, NodeLink *hashTable, int support, int transNum, int itemsNum)
{
	int idx = 0;    /*số thứ tự của các transaction*/
	size_t read = 0;
	char *transBuf = NULL;
	transBuf = (char *)malloc(BUF_SIZE*sizeof(char));    /*buffer chứa nội dung của từng transaction*/
	if(!transBuf)
	{
		perror("out of memory!!!");
		exit(-1);
	}
	read = BUF_SIZE;    /*buffer size khởi tạo*/
	assert(file);
	while((read = getline(&transBuf, &read, file)) != -1)    /*đọc từng giao dịch, ghi vào transaction và hashTable*/
	{
		readTransaction(transaction, idx++, hashTable, support, transBuf, itemsNum);
	}                                         
	if(transBuf)
		free(transBuf);   /* free transaction buffer */
}

/*readTransaction: 
Input:
	transaction: mảng các ItemLink, mỗi ItemLink là một giao dịch,
	mỗi phần tử của transaction (transaction[idx]) là item đầu của một giao dịch
	idx: số thứ tự của các giao dịch
	hashTable: mảng các NodeLink, mỗi NodeLink là mảng các item có cùng giá trị hash
	support: ngưỡng support count
	transBuf: buffer chứa nội dung của 1 giao dịch
	itemsNum: tổng số các loại item (các loại mặt hàng)
Output:
	ko có
Chức năng:
	thực hiện việc ghi từng giao dịch vào mảng transaction */

void readTransaction(ItemLink *transaction, int idx, NodeLink *hashTable, int support, char *transBuf, int itemsNum)
{
	int hashSize = itemsNum;
	char *p;
	char seperators[3] = {DMN_SPRT, RCD_SPRT, '\0'};    /*khoảng cách giữa các item và các giao dịch*/
	const char *delim = seperators;
	p = strtok(transBuf, delim);	/* transaction id */
	//p = strtok(NULL, delim);    	/* items number */
	//p = strtok(NULL, delim);    	/* first item */
	while(p)
	{
		createTransaction(transaction, idx, p);    /*ghi từng giao dịch có số thứ tự idx vào mảng transaction*/
		createHashTable(hashTable, p, support, hashSize);    /*cập nhật hashTable*/
		p = strtok(NULL, delim);    /*đọc từng item từ transBuf*/
	}
}

/*createTransaction:
Input:
	transaction: mảng các ItemLink, mỗi ItemLink là một giao dịch
	idx: số thứ tự của các giao dịch
	field: tên item
Output:
	ko có
Chức năng:	
	hàm thực hiện ghi từng item vào giao dịch của nó, transaction[i] là item đầu tiên của giao dịch thứ i,
	transaction[i] có con trỏ next tới các item tiếp theo*/

void createTransaction(ItemLink *transaction, int idx, char *field)
{
	Item *i = (Item *)malloc(sizeof(Item));    /*tạo item để lưu vào transaction*/
	if(!i)
	{
		perror("out of memory!!!");exit(-1);
	}
	i->field = (char *)malloc(strlen(field) + 1);
	if(!i->field)
	{
		perror("out of memory!!!");exit(-1);
	}
	strcpy(i->field, field);
	i->next = transaction[idx];   /*liên kết item vừa được thêm với các item trước của giao dịch*/
	transaction[idx] = i;   /*item mới được thêm trở thành item đầu của giao dịch*/
}

/*deleteUnsupport:
Input:
	transaction: mảng các ItemLink, mỗi ItemLink là một giao dịch
	transNum: tổng số giao dịch
	support: ngưỡng support count
	hashTable: mảng các NodeLink, mỗi NodeLink là mảng các item có cùng giá trị hash
	hashTableSize = itemsNum: kích thước của hashTable = số item
Output:
	ko có
Chức năng:
	xét từng giao dịch và xóa các item trong giao dịch mà ko thỏa ngưỡng support count*/

void deleteUnsupport(ItemLink *transaction, int transNum, int support, NodeLink *hashTable, int hashTableSize)
{
	int i;
	#pragma omp parallel for \
		shared(transaction, transNum, support, hashTable, hashTableSize) \
		private(i) \
		schedule(static)
	for(i = 0; i < transNum; ++i)
	{
		ItemLink p, q = transaction[i];
		
		/*vòng lặp while dưới tiến hành xóa các item đầu tiên của từng giao dịch để giữ được một item có support thỏa
		yêu cầu làm transactionHead cho giao dịch đó*/
		while(q && getSupport(hashTable, hashTableSize, q->field) < support)
		{
			transaction[i] = q->next;
			free(q->field);
			free(q);
			q = transaction[i];
		}
		
		/*sau khi đã tìm được transactionHead cho mỗi giao dịch, tiến hành xét các item sau đó*/
		if(!q || !q->next)continue;
		p = q->next;       
		while(p)
		{
			if(getSupport(hashTable, hashTableSize, p->field) < support)
			{
				q->next = p->next;
				free(p->field);
				free(p);
				p = q->next;
				continue;
			}
			p = p->next;
			q = q->next;
		}
	}
}


/*sortEveryTransaction:
Input:
	transaction: mảng các ItemLink, mỗi ItemLink là một giao dịch
	transNum: tổng số giao dịch
	hashTable: mảng các NodeLink, mỗi NodeLink là mảng các item có cùng giá trị hash
	hashTableSize = itemsNum: kích thước của hashTable = số item
	support: ngưỡng support count
Output:
	ko có
Chức năng:
	sắp xếp các item trong các giao dịch theo thứ tự giảm dần support count*/

void sortEveryTransaction(ItemLink *transaction, int transNum, NodeLink *hashTable, int hashTableSize, int support)
{
	int i;
	//printf("Xuat cac giao dich sau khi loai cac item ko thoa support count va sap xep giam dan:\n");
	for(i = 0; i < transNum; ++i)
	{
		selectSort(&transaction[i], hashTable, hashTableSize, support);
		//ItemLink p = transaction[i];
		//for ( ; p; p = p->next)
		//	printf("%s	", p->field);
		//printf("\n");
	}
}


/*selectSort:
Input:
	transactionHead = transaction[i]: item đầu tiên của mỗi giao dịch
	hashTable: mảng các NodeLink, mỗi NodeLink là mảng các item có cùng giá trị hash
	hashTableSize = itemsNum: kích thước của hashTable = số item
	support: ngưỡng support count
Output:
	ko có
Chức năng:
	sắp xếp item trong giao dịch dựa theo support count tổng cộng của item đó trong tập dữ liệu theo thứ tự giảm dần,
nếu support bằng nhau thì xếp theo alphabe*/

void selectSort(ItemLink *transactionHead, NodeLink *hashTable, int hashTableSize, int support)
{                                           
	ItemLink p, q, t, s, h;
	h = (ItemLink)malloc(sizeof(Item));
	if(!h)
	{
		perror("out of memory!!!");exit(-1);
	}
	h->next = (*transactionHead);
	p = h;
	while(p->next && p->next->next != NULL)
	{
		for(s = p, q = p->next; q->next != NULL; q = q->next)
		{
			int spt_q = getSupport(hashTable, hashTableSize, q->next->field);
			int spt_s = getSupport(hashTable, hashTableSize, s->next->field);
			if(spt_q > spt_s || (spt_q == spt_s &&
				 strcmp(q->next->field, s->next->field) < 0))
				s = q;
		}
		if(s != q)
		{
			t = s->next;
			s->next = t->next;
			t->next = p->next;
			p->next = t;
		}
		p = p->next;
	}
	(*transactionHead) = h->next;
	free(h);
}

/*hash: 
Input:
	field: tên item
	hashTableSize: kích thước bảng hashTable
Output:
	giá trị hash của item
Chức năng:
	tính giá trị hash cho item để ghi vào hashTable*/

int hash(char *field, int hashTableSize)    /*dùng BKDR Hash Function*/
{                                           
	unsigned int seed = 5;    /*namely seed is 31*/
	unsigned int h = 0;
	char *p = field;
	while(*p)
	{
		h = (h << seed) - h + (*p++);    /*h = h*31 + (*p++)*/
	}
	return h % hashTableSize;
}

/*createHashTable:
Input:
	hashTable: mảng các NodeLink, mỗi NodeLink là một tập các item cùng giá trị hash,
	support count của các item trong hashTable = số lần xuất hiện item đó trong tất cả các đơn hàng
	field: tên item
	support: ngưỡng support count
	hashTableSize = itemsNum: kích thước bảng hashTable
Output:
	ko có
Chức năng:
	thêm item vào hashTable*/

void createHashTable(NodeLink *hashTable, char *field, int support, int hashTableSize)
{
	int h = hash(field, hashTableSize);    /*tìm giá trị hash của item, giá trị này cũng là vị trí nodeLink chứa item đó*/
	NodeLink p;
	for(p = hashTable[h]; p; p=p->next)
	{    /*tra nodeLink vị trí [h] xem có tồn tại item trong mảng đó chưa*/
		if(strcmp(p->field, field) == 0)
		{    /*nếu có, tăng support count và kiểm tra thỏa ngưỡng support chưa*/
			p->support++;
			if(p->support >= support) p->exceeded = 'Y';
			return;                               
		}
	}
	p = (NodeLink)malloc(sizeof(Node));    /*nếu chưa có, tạo thêm 1 node và liên kết nó với các node đã có của nodeLink[h]*/
	if(!p)
	{
		perror("out of memory!!!");exit(-1);
	}
	p->field = (char *)malloc(strlen(field) + 1);
	if(!p->field)
	{
		perror("out of memory!!!");exit(-1);
	}
	strcpy(p->field, field);
	p->support = 1;
	if(support > 1)	p->exceeded = 'N';        /*giá trị mặc định*/
	else p->exceeded = 'Y';
	p->next = hashTable[h];                   /*liên kết với node trước đó*/
	hashTable[h] = p;                         /*item mới được thêm vào trở thành item đầu của nodeLink[h]*/
}

/*getNumOfExceeded:
Input:
	hashTable: bảng hash
	hashTableSize: kích thước bảng hash
Output:
	số item thỏa ngưỡng support count
Chức năng:
	tính có bao nhiêu item thỏa ngưỡng support count*/

int getNumOfExceeded(NodeLink *hashTable, int hashTableSize)
{                                           /* get the number of items whose support has exceeded the threshold */
	int i;
	int num = 0;
	//#pragma omp parallel for \
		shared(hashTable, hashTableSize, num) \
		private(i) \
		schedule(static)
	for(i = 0; i < hashTableSize; ++i)
	{
		NodeLink p = hashTable[i];
		for(; p; p = p->next)
		{
			if(p->exceeded == 'Y')
				//#pragma omp atomic
				++num;
		}
	}
	return num;
}

/*getSupport:
Input:
	hashTable: bảng hash
	hashTableSize: kích thước bảng hash
	field: tên item
Output:
	giá trị support count của item
Chức năng:
	trả về giá trị support count của item nào đó trong hashTable*/

int getSupport(NodeLink *hashTable, int hashTableSize, char *field)
{
	int h = hash(field, hashTableSize);
	NodeLink p;
	for(p = hashTable[h]; p; p=p->next)
	{
		if(strcmp(p->field, field) == 0)
			return p->support;
	}
	printf("input error, can not find the key[%s]\n", field);
	return -1;
}

void freeTransaction(ItemLink *transaction, int transNum)
{
	int i;
	
	#pragma omp parallel for \
		shared(transaction, transNum) \
		private(i) \
		schedule(static)
	for (i = 0; i < transNum; ++i)
	{
		ItemLink q = transaction[i];
		while (q)
		{
			transaction[i] = q->next;
			free(q->field);
			free(q);
			q = transaction[i];
		}
	}
	
	free(transaction);
}

void freeHashTable(NodeLink *hashTable, int hashTableSize)
{
	int i;
	
	//#pragma omp parallel for \
		shared(hashTable, hashTableSize) \
		private(i) \
		schedule(static)
	for (i = 0; i < hashTableSize; ++i)
	{
		NodeLink p = hashTable[i];
		while (p)
		{
			hashTable[i] = p->next;
			free(p->field);
			free(p);
			p = hashTable[i];
		}
	}
	
	free(hashTable);
}
