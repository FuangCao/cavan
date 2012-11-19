// Fuang.Cao <cavan.cfa@gmail.com> Wed Apr 13 10:40:43 CST 2011

#include <cavan.h>
#include <cavan/list.h>

void single_link_init(struct single_link *link)
{
	link->head = NULL;
}

void single_link_free(struct single_link *link, off_t offset)
{
	struct single_link_node *node, *next;

	node = link->head;

	while (node)
	{
		next = node->next;
		free(((char *)node) - offset);
		node = next;
	}

	single_link_init(link);
}

void single_link_insert_back(struct single_link_node *pre, struct single_link_node *node)
{
	node->next = pre->next;
	pre->next = node;
}

void single_link_insert_head(struct single_link *link, struct single_link_node *node)
{
	node->next = link->head;
	link->head = node;
}

void single_link_append(struct single_link *link, struct single_link_node *node)
{
	struct single_link_node *pre;

	node->next = NULL;

	if (link->head == NULL)
	{
		link->head = node;
		return;
	}

	pre = link->head;

	while (pre->next)
	{
		pre = pre->next;
	}

	pre->next = node;
}

void single_link_traversal(struct single_link *link, void (*handle)(struct single_link_node *node))
{
	struct single_link_node *node = link->head;

	while (node)
	{
		handle(node);
		node = node->next;
	}
}

struct single_link_node *single_link_find(struct single_link *link, void *data, int (*match)(struct single_link_node *node, void *data))
{
	struct single_link_node *node = link->head;

	while (node)
	{
		if (match(node, data))
		{
			return node;
		}
		node = node->next;
	}

	return NULL;
}

void single_link_delete_node(struct single_link *link, struct single_link_node *node)
{
	struct single_link_node *pre;

	if (node == link->head)
	{
		link->head = link->head->next;
	}

	pre = link->head;

	while (pre->next && pre->next != node)
	{
		pre = pre->next;
	}

	if (pre->next)
	{
		pre->next = pre->next->next;
	}
}

void circle_link_init(struct circle_link *link)
{
	link->head_node.next = &link->head_node;
}

void circle_link_free(struct circle_link *link, off_t offset)
{
	struct single_link_node *head = &link->head_node;
	struct single_link_node *node = head->next;
	struct single_link_node *temp;

	while (node != head)
	{
		temp = node->next;
		free(((char *)node) - offset);
		node = temp;
	}

	circle_link_init(link);
}

void circle_link_append(struct circle_link *link, struct single_link_node *node)
{
	struct single_link_node *head = &link->head_node;
	struct single_link_node *pre = head;

	while (pre->next != head)
	{
		pre = pre->next;
	}

	single_link_insert_back(pre, node);
}

void circle_link_insert_head(struct circle_link *link, struct single_link_node *node)
{
	node->next = link->head_node.next;
	link->head_node.next = node;
}

void circle_link_delete_node(struct single_link_node *node)
{
	struct single_link_node *pre = node;

	while (pre->next != node)
	{
		pre = pre->next;
	}

	pre->next = node->next;
}

void circle_link_traversal(struct circle_link *link, void (*handle)(struct single_link_node *node))
{
	struct single_link_node *head = &link->head_node;
	struct single_link_node *node = head->next;

	while (node != head)
	{
		handle(node);
		node = node->next;
	}
}

struct single_link_node *circle_link_find(struct circle_link *link, void *data, int (*match)(struct single_link_node *node, void *data))
{
	struct single_link_node *head = &link->head_node;
	struct single_link_node *node = head->next;

	while (node != head)
	{
		if (match(node, data))
		{
			return node;
		}
		node = node->next;
	}

	return NULL;
}

void double_link_init(struct double_link *link)
{
	link->head_node.next = &link->head_node;
	link->head_node.pre = &link->head_node;
}

void double_link_free(struct double_link *link, off_t offset)
{
	struct double_link_node *head = &link->head_node;
	struct double_link_node *node = head->next;
	struct double_link_node *temp;

	while (node != head)
	{
		temp = node->next;
		free(((char *)node) - offset);
		node = temp;
	}

	double_link_init(link);
}

void double_link_insert_simple(struct double_link_node *pre, struct double_link_node *next, struct double_link_node *node)
{
	pre->next = node;
	node->pre = pre;

	next->pre = node;
	node->next = next;
}

void double_link_insert_back(struct double_link_node *pre, struct double_link_node *node)
{
	double_link_insert_simple(pre, pre->next, node);
}

void doubel_link_insert_front(struct double_link_node *next, struct double_link_node *node)
{
	double_link_insert_simple(next->pre, next, node);
}

void double_link_insert_head(struct double_link *link, struct double_link_node *node)
{
	double_link_insert_simple(&link->head_node, link->head_node.next, node);
}

void double_link_append(struct double_link *link, struct double_link_node *node)
{
	double_link_insert_simple(link->head_node.pre, &link->head_node, node);
}

void double_link_traversal(struct double_link *link, void (*handle)(struct double_link_node *node))
{
	struct double_link_node *head = &link->head_node;
	struct double_link_node *node = head->next;

	while (node != head)
	{
		handle(node);
		node = node->next;
	}
}

void double_link_delete_node(struct double_link_node *node)
{
	node->pre->next = node->next;
	node->next->pre = node->pre;
}

struct double_link_node *double_link_find(struct double_link *link, void *data, int (*match)(struct double_link_node *node, void *data))
{
	struct double_link_node *head = &link->head_node;
	struct double_link_node *node = head->next;

	while (node != head)
	{
		if (match(node, data))
		{
			return node;
		}
		node = node->next;
	}

	return NULL;
}

int array_has_element(int element, const int a[], size_t size)
{
	const int *a_end;

	for (a_end = a + size; a < a_end; a++)
	{
		if (*a == element)
		{
			return 1;
		}
	}

	return 0;
}
