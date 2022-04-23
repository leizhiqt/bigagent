/********************************** 
 * @author	leizhifesker@gmail.com
 * @date	2017/02/16
 * Last update:	2017/02/16
 * License:	LGPL
 * 
**********************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>

#include "ulog.h"
#include "dtype.h"
#include "linklist.h"

//根据指针 (link_list_t *ptr) 初始化链表
int link_list_init(link_list_t *link_list)
{
	link_list->head = NULL;
	link_list->last = NULL;
	link_list->size = 0;
	return 0;
}

//根据指针 (link_list_t *ptr) 加入链表首节点 link_node_t
int link_list_add_first(link_list_t *link_list,link_node_t *link_node)
{
	if(link_list->head == NULL && link_list->last == NULL){
		link_list->head = link_node;
		link_list->last = link_node;

		link_node->prev = NULL;
	}else{
		link_list->last->next = link_node;
		link_node->prev = link_list->last;
		link_list->last = link_node;
	}
	link_node->next = NULL;
	link_list->size++;
	return 0;
}

//根据指针 (link_list_t *ptr) 加入链表尾节点 link_node_t
int link_list_add_last(link_list_t *link_list,link_node_t *link_node)
{
	if(link_list->head == NULL && link_list->last == NULL){
		link_list->head = link_node;
		link_list->last = link_node;
		link_node->prev = NULL;
	}else{
		link_list->last->next = link_node;
		link_node->prev = link_list->last;
		link_list->last = link_node;
	}
	link_node->next = NULL;
	link_list->size++;
	return 0;
}

//链表入栈
int link_list_push(link_list_t *link_list,link_node_t *link_node)
{
	return link_list_add_last(link_list,link_node);
}

//根据指针 (link_list_t *ptr) 返回链表元素个数
int link_list_size(link_list_t *link_list)
{
	if(link_list==NULL) return -1;

	return link_list->size;
}
