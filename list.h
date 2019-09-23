#ifndef LIST_H_
#define LIST_H_
#include <stdlib.h>
#include <assert.h>
/*��ʼ������list������Ϊ��ͷ����˫��ѭ������*/
#define List_Init(list,list_node_t) {					\
		list=(list_node_t*)malloc(sizeof(list_node_t)); \
		(list)->next=(list)->prev=list;					\
	}

//�ͷ�����list���������ݽ�㡣list Ϊ����ͷָ�룬list_node_tΪ����������
#define List_Free(list,list_node_t) {			\
		assert(NULL!=list);						\
		list_node_t *tmpPtr;					\
		(list)->prev->next=NULL; 				\
		while(NULL!=(tmpPtr=(list)->next)){ 	\
			(list)->next=tmpPtr->next;			\
			free(tmpPtr);						\
		}										\
		(list)->next=(list)->prev=list;			\
	}

//��������list���ͷ��������ݽ�㼰ͷ��㡣 listΪ����ͷָ�룬tmpPtrΪ��������ʱָ�����
#define List_Destroy(list,list_node_t) {		\
		assert(NULL!=list);						\
		List_Free(list, list_node_t);			\
		free(list);								\
		(list)=NULL;							\
	}

//����ͷ�巨��listΪͷָ�룬newΪ�½ڵ�
#define List_AddHead(list,newNode) {			\
		(newNode)->next=(list)->next;		 	\
		(list)->next->prev=newNode;			 	\
		(newNode)->prev=(list);				 	\
		(list)->next=newNode;				 	\
	}

//����β�巨��listΪͷָ�룬newΪ�½ڵ�
#define List_AddTail(list,newNode) {			\
		(newNode)->prev=(list)->prev; 		 	\
		(list)->prev->next=newNode;			 	\
		(newNode)->next=list;				 	\
		(list)->prev=newNode;				 	\
	}

//���½ڵ�newNode���뵽node֮ǰ
#define List_InsertBefore(node,newNode) {		\
		(newNode)->prev=(node)->prev; 		 	\
		(newNode)->next=node;			 		\
		(newNode)->prev->next=newNode;			\
		(newNode)->next->prev=newNode;			\
	}

//���½ڵ�newNode���뵽node֮��
#define List_InsertAfter(node, newNode) {		\
		(newNode)->next=node->next;			 	\
		(newNode)->prev=node; 				 	\
		(newNode)->next->prev=newNode;			\
		(newNode)->prev->next=newNode;			\
	}

//�ж������Ƿ�Ϊ�գ�listΪͷָ��
#define List_IsEmpty(list)  ((list != NULL)	\
	&& ((list)->next == list)				\
	&& (list == (list)->prev))

//��ɾ��������node��
#define List_DelNode(node) {\
			assert(NULL!=node && node!=(node)->next && node!=(node)->prev);				\
			(node)->prev->next=(node)->next; 	\
			(node)->next->prev=(node)->prev;	\
	}

//��������ɾ�����ͷŽ��node
#define List_FreeNode(node) {	\
		List_DelNode(node);		\
		free(node);				\
	}


//ʹ��ָ��curPos���α�������list
#define List_ForEach(list, curPos) 		\
	 for (   curPos = (list)->next;  	\
		  	  	  curPos != list;       \
		  	  	  curPos=curPos->next	\
	    )



// void AddToSortedList(name_list_t list,name_node_t *p)
// {
//     name_node_t *cur;
//     if(!list) {List_AddTail(list,p);}
//     else{
//               cur = list->next;
//               while(cur != list){
//                   if(p->data.age < cur->data.age) break;
//                   cur = cur->next;
//               }
//               List_InsertBefore(cur,p);
//     }
//     return;
// }
// void  SortByNum(name_list_t list) {
//     int flag = 1;
//     name_list_t listLeft;
//     List_Init(listLeft,name_node_t);
//    if(!list)    flag = 0;
//     if(flag){
//    list->prev->next     =   NULL;//将循环链表最后一个节点断开
//    listLeft     =   list->next;  //listleft指向第一个数据节点
//    list->next   =     list->prev = list;//将list链表置为空
//    while(listLeft != NULL){
//        name_list_t p;
//        p = listLeft;
//        listLeft = listLeft->next;
//        AddToSortedList(list,p);
//    }}
// }

#endif
