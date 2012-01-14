/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Jan  3 15:37:38 CST 2012
 */

#include "list.h"

void cavan_list_head_init(struct cavan_list_node *head)
{
	head->next = head;
	head->prev = head;
}

void cavan_list_insert(struct cavan_list_node *node, struct cavan_list_node *prev, struct cavan_list_node *next)
{
	node->prev = prev;
	prev->next = node;

	node->next = next;
	next->prev = node;
}
