#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Jan  3 15:37:38 CST 2012
 */

struct cavan_list_node
{
	struct cavan_list_node *prev;
	struct cavan_list_node *next;
};


void cavan_list_head_init(struct cavan_list_node *head);
void cavan_list_insert(struct cavan_list_node *node, struct cavan_list_node *prev, struct cavan_list_node *next);

static inline void cavan_list_append(struct cavan_list_node *head, struct cavan_list_node *node)
{
	cavan_list_insert(node, head->prev, head);
}

static inline void cavan_list_prepend(struct cavan_list_node *head, struct cavan_list_node *node)
{
	cavan_list_insert(node, head, head->next);
}

static inline int cavan_list_is_empty(struct cavan_list_node *head)
{
	return head->next == head;
}

static inline int cavan_list_not_empty(struct cavan_list_node *head)
{
	return head->next != head;
}
