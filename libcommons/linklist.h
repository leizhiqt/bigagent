/********************************** 
* @author	leizhifesker@gmail.com
* @date	2017/02/16
* Last update:	2017/02/16
* License:	LGPL
* 双向通用链表,只处理逻辑，不载体数据
* 
**********************************/

#ifndef _LINKLIST_
#define _LINKLIST_

//version 1.1

//计算结构体(struct type)中成员(stuct type {member})的相对偏移量
//(type*)0 指向0的指针 (type*)0 == (type*)NULL
//(struct_t *)0是一个指向struct_t类型的指针，其指针值为 0
//#define offsetof(type, member) (size_t)(&((type*)0)->member)

//根据member的地址获取type的起始地址
#define container_of(ptr, type, member)						\
({															\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );		\
})

//链表节点
typedef struct _link_node link_node_t;
struct _link_node{
	link_node_t* prev;	//指向上一个节点
	link_node_t* next;	//指向下一个节点
};

//链表
typedef struct _link_list{
	link_node_t *head;	//头指针
	link_node_t *last;	//末尾指针
	uint size;
}link_list_t;

//根据指针 (link_list_t *ptr) 初始化链表
int link_list_init(link_list_t *link_list);

//根据指针 (link_list_t *ptr) 加入链表首节点 link_node_t
int link_list_add_first(link_list_t *link_list,link_node_t *link_node);

//根据指针 (link_list_t *ptr) 加入链表尾节点 link_node_t
int link_list_add_last(link_list_t *link_list,link_node_t *link_node);

//链表入栈
int link_list_push(link_list_t *link_list,link_node_t *link_node);

//根据指针 (link_list_t *ptr) 返回链表元素个数
int link_list_size(link_list_t *link_list);

//link_list_remove_first根据指针 type类型 移出
//ptr link_node_t *
#define link_list_remove_first(ptr, type) 				\
		({												\
			if((ptr)->head != NULL && (ptr)->size>0 )	\
			{											\
				link_node_t *p = (ptr)->head;			\
				if((ptr)->head->next!=NULL)				\
				{										\
					(ptr)->head->next->prev = NULL;		\
					(ptr)->head = (ptr)->head->next;	\
				}else{									\
					(ptr)->head = NULL;					\
					(ptr)->last = NULL;					\
				}										\
				type *dptr=container_of(p,type,node);	\
				dptr->node.prev = NULL;					\
				dptr->node.next = NULL;					\
				if(dptr!=NULL)							\
				{										\
					free(dptr);							\
					dptr=NULL;							\
				}										\
				(ptr)->size--;							\
			}											\
		})

//link_list_remove_last根据指针 type类型 移出
//ptr link_node_t *
#define link_list_remove_last(ptr, type) 				\
		({												\
			if((ptr)->last != NULL && (ptr)->size>0 )	\
			{											\
				link_node_t *p = (ptr)->last;			\
				if((ptr)->last->prev != NULL)			\
				{										\
					(ptr)->last->prev->next = NULL;		\
					(ptr)->last = (ptr)->last->prev;	\
				}else{									\
					(ptr)->head = NULL;					\
					(ptr)->last = NULL;					\
				}										\
				type *dptr=container_of(p,type,node);	\
				dptr->node.prev = NULL;					\
				dptr->node.next = NULL;					\
				if(dptr!=NULL)							\
				{										\
					free(dptr);							\
					dptr=NULL;							\
				}										\
				(ptr)->size--;							\
			}											\
		})

//根据指针 type类型 释放内存
#define link_list_destroy(ptr,type) 					\
		({												\
			link_node_t *p = (ptr)->head;				\
			link_node_t *cur = NULL;					\
			type *type_ptr;								\
			while(p!=NULL)								\
			{											\
				cur=p->next;							\
				type_ptr = container_of(p,type,node);	\
				type_ptr->node.prev = NULL;				\
				type_ptr->node.next = NULL;				\
				if(type_ptr!=NULL)						\
				{										\
					free(type_ptr);						\
					type_ptr=NULL;						\
				}										\
				p = cur;								\
			}											\
			(ptr)->head = NULL;							\
			(ptr)->last = NULL;							\
			(ptr)->size = 0;							\
		})

//链表出栈
#define link_list_pop(ptr,type)		link_list_remove_last(ptr, type)

 #endif
