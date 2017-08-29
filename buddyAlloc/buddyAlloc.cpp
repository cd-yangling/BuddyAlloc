// buddyAlloc.cpp : 定义控制台应用程序的入口点。
//伙伴系统

#include "stdafx.h"
#include <cstdlib>
#include <cmath>

#define MAX_SIZE	64//最大空间
#define LIST_LEN	7//表头数组长度
#define MAX_INDEX	6//表头数组最大下标
struct BuddyNode{
	BuddyNode * preLink;
	BuddyNode * nextLink;
	int k;//只能分配2 的k次幂
	int tag;//0 空闲, 1 占用
};

typedef struct BuddyHead{
	BuddyNode * head;
	int nodeSize;
}FreeList[LIST_LEN];

static BuddyNode * freeSpace = NULL;//为了释放内存
static BuddyNode * userSpace[MAX_SIZE] = {NULL};
static int userCount = 0;

void initFreeList(FreeList list){
	for (int i = 0; i < LIST_LEN; i++){
		list[i].head = NULL;
		list[i].nodeSize = (int)pow(2.0,i);
	}
	//分配最大空间
	BuddyNode * p = (BuddyNode*)malloc(sizeof(BuddyNode) * MAX_SIZE);
	if (p != NULL){
		p->k = MAX_INDEX;
		p->tag = 0;
		p->nextLink = p->preLink = p;//双向循环链表
	}
	freeSpace = list[MAX_INDEX].head = p;
}

void destoryFreeList(FreeList list){
	for (int i = 0; i < LIST_LEN; i++){
		list[i].head = NULL;
	}
	free(freeSpace);
}

BuddyNode * buddyAlloc(FreeList list,int size){
	bool isFirst = true;
	int k = -1;//分配空间的k值
	int needK = -1;//需要分配空间的k值.
	//查找第一个满足条件的空间.
	for (int i = 0; i < LIST_LEN; i++){
		BuddyHead head = list[i];
		if (head.nodeSize >= size){
			if (isFirst){
				needK = i;
				isFirst = false;
			}
			if (head.head != NULL){//找到可分配的空间了
				k = i;
				break;
			}
		}
	}
	BuddyNode * freeNode = NULL;
	if (k == -1){
		return NULL;//找不到满足条件的空间了..
	}
	else{
		//设置这个空间 被占用，并且 更换 表头元素..
		freeNode = list[k].head;
		freeNode->k = needK;
		freeNode->tag = 1;//设置 成 已占用..
		list[k].head = freeNode->nextLink;//重新设置表头..
		if (freeNode->preLink == freeNode->nextLink){//删除这个空间后，成为空表.
			list[k].head = NULL;
		}
		else{//删除这个节点.
			freeNode->preLink->nextLink = freeNode->nextLink;
			freeNode->nextLink->preLink = freeNode->preLink;
		}
		//剩余空间 依次 插在 needK 和 k-1 表里.
		for (int i = needK; i < k ; i++){//从高位开始分配..
			int index = (int)pow(2.0,i);
			BuddyNode * p = freeNode + index;
			p->k = i;
			p->tag = 0;
			BuddyNode * head = list[i].head;
			if (head != NULL){
				p->preLink = head->preLink;
				p->nextLink = head;
				p->preLink->nextLink = head->preLink = p;
			}
			else{
				p->preLink = p->nextLink = p;
			}
			list[i].head = p;//重新设置表头..
		}
	}
	userSpace[userCount++] = freeNode;
	return freeNode;
}

void userSpaceFree(BuddyNode * node){
	for (int i = 0; i < userCount; i++){
		if (userSpace[i] == node){
			userSpace[i] = NULL;
		}
	}
}

//伙伴算法 回收...
void buddyReclaim(FreeList list,BuddyNode * node){
	userSpaceFree(node);
	while (node->k < MAX_INDEX){//循环查找伙伴，k值达到 MAX_INDEX 不需要 寻找...
		int sub = node - freeSpace;//计算出 具体坐标位置
		BuddyNode * buddy = NULL;
		int i = (int)pow(2.0,node->k + 1);
		bool isNext = true;//伙伴是不是在后
		if(sub % i == 0){//伙伴在后
			buddy = node + i/2;
		}
		else{//伙伴在前.
			buddy = node - i/2;
			isNext = false;
		}
		if (buddy->tag == 0 ){//伙伴空闲，合并
			//首先从列表里释放 buddy
			if (buddy->preLink == buddy->nextLink){//表中 只有一个节点，释放后，成为空表
				list[buddy->k].head = NULL;
			}
			else{//删除节点
				buddy->preLink->nextLink = buddy->nextLink;
				buddy->nextLink->preLink = buddy->preLink;
				list[buddy->k].head = buddy->nextLink;
			}
			if (isNext == false){
				node = buddy;
			}
			node->k ++;
		}
		else{//伙伴空间被占用..
			break;
		}
	}
	BuddyNode * head = list[node->k].head;
	node->tag = 0;
	if (head == NULL){//表头为空
			node->preLink = node->nextLink = node;
	}
	else{
		node->nextLink = head;
		node->preLink = head->preLink;
		node->preLink->nextLink = head->preLink = node;
	}
	list[node->k].head = node;
}

void printList(FreeList list){
	printf("------------------------打印伙伴列表-------------------\n");
	for (int i = 0; i < LIST_LEN; i++){
		BuddyNode * head = list[i].head;
		if (head){
			printf("首地址空间为：%0x,表的前驱：%0x,后继：%0x,表空间大小是2的%d次方\n",head,head->preLink,head->nextLink,head->k);
		}
	}
	printf("------------------------用户空间-------------------\n");
	for (int i = 0; i < userCount; i++){
		BuddyNode * us = userSpace[i];
		if (us != NULL){
			printf("首地址空间为：%0x,表空间大小是2的%d次方\n",us,us->k);
		}
	}
	printf("\n");
}



int _tmain(int argc, _TCHAR* argv[])
{
	FreeList list;
	initFreeList(list);
	printList(list);
	printf("---------------分配一个1空间之后--------------\n");
	BuddyNode * s1_1 = buddyAlloc(list,1);
	printList(list);
	printf("---------------分配一个1空间之后--------------\n");
	BuddyNode * s1_2 = buddyAlloc(list,1);
	printList(list);
	printf("---------------分配一个29空间之后--------------\n");
	BuddyNode * s29 = buddyAlloc(list,29);
	printList(list);
	printf("---------------释放一个1空间之后--------------\n");
	buddyReclaim(list,s1_2);
	printList(list);
	printf("---------------释放一个1空间之后--------------\n");
	buddyReclaim(list,s1_1);
	printList(list);
	destoryFreeList(list);
	return 0;
}

