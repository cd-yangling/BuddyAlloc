// buddyAlloc.cpp : �������̨Ӧ�ó������ڵ㡣
//���ϵͳ

#include "stdafx.h"
#include <cstdlib>
#include <cmath>

#define MAX_SIZE	64//���ռ�
#define LIST_LEN	7//��ͷ���鳤��
#define MAX_INDEX	6//��ͷ��������±�
struct BuddyNode{
	BuddyNode * preLink;
	BuddyNode * nextLink;
	int k;//ֻ�ܷ���2 ��k����
	int tag;//0 ����, 1 ռ��
};

typedef struct BuddyHead{
	BuddyNode * head;
	int nodeSize;
}FreeList[LIST_LEN];

static BuddyNode * freeSpace = NULL;//Ϊ���ͷ��ڴ�
static BuddyNode * userSpace[MAX_SIZE] = {NULL};
static int userCount = 0;

void initFreeList(FreeList list){
	for (int i = 0; i < LIST_LEN; i++){
		list[i].head = NULL;
		list[i].nodeSize = (int)pow(2.0,i);
	}
	//�������ռ�
	BuddyNode * p = (BuddyNode*)malloc(sizeof(BuddyNode) * MAX_SIZE);
	if (p != NULL){
		p->k = MAX_INDEX;
		p->tag = 0;
		p->nextLink = p->preLink = p;//˫��ѭ������
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
	int k = -1;//����ռ��kֵ
	int needK = -1;//��Ҫ����ռ��kֵ.
	//���ҵ�һ�����������Ŀռ�.
	for (int i = 0; i < LIST_LEN; i++){
		BuddyHead head = list[i];
		if (head.nodeSize >= size){
			if (isFirst){
				needK = i;
				isFirst = false;
			}
			if (head.head != NULL){//�ҵ��ɷ���Ŀռ���
				k = i;
				break;
			}
		}
	}
	BuddyNode * freeNode = NULL;
	if (k == -1){
		return NULL;//�Ҳ������������Ŀռ���..
	}
	else{
		//��������ռ� ��ռ�ã����� ���� ��ͷԪ��..
		freeNode = list[k].head;
		freeNode->k = needK;
		freeNode->tag = 1;//���� �� ��ռ��..
		list[k].head = freeNode->nextLink;//�������ñ�ͷ..
		if (freeNode->preLink == freeNode->nextLink){//ɾ������ռ�󣬳�Ϊ�ձ�.
			list[k].head = NULL;
		}
		else{//ɾ������ڵ�.
			freeNode->preLink->nextLink = freeNode->nextLink;
			freeNode->nextLink->preLink = freeNode->preLink;
		}
		//ʣ��ռ� ���� ���� needK �� k-1 ����.
		for (int i = needK; i < k ; i++){//�Ӹ�λ��ʼ����..
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
			list[i].head = p;//�������ñ�ͷ..
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

//����㷨 ����...
void buddyReclaim(FreeList list,BuddyNode * node){
	userSpaceFree(node);
	while (node->k < MAX_INDEX){//ѭ�����һ�飬kֵ�ﵽ MAX_INDEX ����Ҫ Ѱ��...
		int sub = node - freeSpace;//����� ��������λ��
		BuddyNode * buddy = NULL;
		int i = (int)pow(2.0,node->k + 1);
		bool isNext = true;//����ǲ����ں�
		if(sub % i == 0){//����ں�
			buddy = node + i/2;
		}
		else{//�����ǰ.
			buddy = node - i/2;
			isNext = false;
		}
		if (buddy->tag == 0 ){//�����У��ϲ�
			//���ȴ��б����ͷ� buddy
			if (buddy->preLink == buddy->nextLink){//���� ֻ��һ���ڵ㣬�ͷź󣬳�Ϊ�ձ�
				list[buddy->k].head = NULL;
			}
			else{//ɾ���ڵ�
				buddy->preLink->nextLink = buddy->nextLink;
				buddy->nextLink->preLink = buddy->preLink;
				list[buddy->k].head = buddy->nextLink;
			}
			if (isNext == false){
				node = buddy;
			}
			node->k ++;
		}
		else{//���ռ䱻ռ��..
			break;
		}
	}
	BuddyNode * head = list[node->k].head;
	node->tag = 0;
	if (head == NULL){//��ͷΪ��
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
	printf("------------------------��ӡ����б�-------------------\n");
	for (int i = 0; i < LIST_LEN; i++){
		BuddyNode * head = list[i].head;
		if (head){
			printf("�׵�ַ�ռ�Ϊ��%0x,���ǰ����%0x,��̣�%0x,��ռ��С��2��%d�η�\n",head,head->preLink,head->nextLink,head->k);
		}
	}
	printf("------------------------�û��ռ�-------------------\n");
	for (int i = 0; i < userCount; i++){
		BuddyNode * us = userSpace[i];
		if (us != NULL){
			printf("�׵�ַ�ռ�Ϊ��%0x,��ռ��С��2��%d�η�\n",us,us->k);
		}
	}
	printf("\n");
}



int _tmain(int argc, _TCHAR* argv[])
{
	FreeList list;
	initFreeList(list);
	printList(list);
	printf("---------------����һ��1�ռ�֮��--------------\n");
	BuddyNode * s1_1 = buddyAlloc(list,1);
	printList(list);
	printf("---------------����һ��1�ռ�֮��--------------\n");
	BuddyNode * s1_2 = buddyAlloc(list,1);
	printList(list);
	printf("---------------����һ��29�ռ�֮��--------------\n");
	BuddyNode * s29 = buddyAlloc(list,29);
	printList(list);
	printf("---------------�ͷ�һ��1�ռ�֮��--------------\n");
	buddyReclaim(list,s1_2);
	printList(list);
	printf("---------------�ͷ�һ��1�ռ�֮��--------------\n");
	buddyReclaim(list,s1_1);
	printList(list);
	destoryFreeList(list);
	return 0;
}

