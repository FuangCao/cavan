// Fuang.Cao <cavan.cfa@gmail.com> Wed Apr 13 10:40:43 CST 2011

#include <cavan.h>
#include <cavan/list.h>

int single_link_init(struct single_link *link, int offset)
{
	link->offset = offset;
	single_link_node_init(&link->head_node);

	return pthread_mutex_init(&link->lock, NULL);
}

bool single_link_empty(struct single_link *link)
{
	bool res;

	pthread_mutex_lock(&link->lock);
	res = link->head_node.next == NULL;
	pthread_mutex_unlock(&link->lock);

	return res;
}

struct single_link_node *single_link_get_first_node(struct single_link *link)
{
	struct single_link_node *node;

	pthread_mutex_lock(&link->lock);
	node = link->head_node.next;
	pthread_mutex_unlock(&link->lock);

	return node;
}

void single_link_insert(struct single_link *link, struct single_link_node *prev, struct single_link_node *node)
{
	pthread_mutex_lock(&link->lock);
	single_link_insert_base(prev, node);
	pthread_mutex_unlock(&link->lock);
}

void single_link_append(struct single_link *link, struct single_link_node *node)
{
	struct single_link_node *prev;

	pthread_mutex_lock(&link->lock);

	for (prev = &link->head_node; prev->next; prev = prev->next);

	single_link_insert_base(prev, node);

	pthread_mutex_unlock(&link->lock);
}

bool single_link_remove(struct single_link *link, struct single_link_node *node)
{
	struct single_link_node *prev;

	pthread_mutex_lock(&link->lock);

	for (prev = &link->head_node; prev; prev = prev->next)
	{
		if (prev->next == node)
		{
			single_link_remove_base(prev, node);
			pthread_mutex_unlock(&link->lock);
			return true;
		}
	}

	pthread_mutex_unlock(&link->lock);

	return false;
}

void single_link_push(struct single_link *link, struct single_link_node *node)
{
	pthread_mutex_lock(&link->lock);
	single_link_insert_base(&link->head_node, node);
	pthread_mutex_unlock(&link->lock);
}

struct single_link_node *single_link_pop(struct single_link *link)
{
	struct single_link_node *node;

	pthread_mutex_lock(&link->lock);

	node = link->head_node.next;
	if (node)
	{
		single_link_remove_base(&link->head_node, node);
	}

	pthread_mutex_unlock(&link->lock);

	return node;
}

void single_link_destroy(struct single_link *link, void *data, single_link_handler_t handler)
{
	struct single_link_node *node, *next;

	pthread_mutex_lock(&link->lock);

	node = link->head_node.next;
	while (node)
	{
		next = node->next;
		handler(link, node, data);
		node = next;
	}

	single_link_node_init(&link->head_node);

	pthread_mutex_unlock(&link->lock);
}

void single_link_traversal(struct single_link *link, void *data, single_link_handler_t handler)
{
	struct single_link_node *node;

	pthread_mutex_lock(&link->lock);

	for (node = link->head_node.next; node; node = node->next)
	{
		handler(link, node, data);
	}

	pthread_mutex_unlock(&link->lock);
}

struct single_link_node *single_link_find(struct single_link *link, void *data, single_link_matcher_t matcher)
{
	struct single_link_node *node;

	pthread_mutex_lock(&link->lock);

	for (node = link->head_node.next; node; node = node->next)
	{
		if (matcher(link, node, data))
		{
			pthread_mutex_unlock(&link->lock);

			return node;
		}
	}

	pthread_mutex_unlock(&link->lock);

	return NULL;
}

bool single_link_has_node(struct single_link *link, struct single_link_node *node)
{
	return single_link_find(link, node, single_link_node_match_equal) != NULL;
}

void single_link_free_all(struct single_link *link)
{
	single_link_destroy(link, NULL, single_link_free_handler);
}

// ================================================================================

int circle_link_init(struct circle_link *link, int offset)
{
	link->offset = offset;
	circle_link_node_init(&link->head_node);

	return pthread_mutex_init(&link->lock, NULL);
}

bool circle_link_empty(struct circle_link *link)
{
	bool res;

	pthread_mutex_lock(&link->lock);
	res = link->head_node.next == &link->head_node;
	pthread_mutex_unlock(&link->lock);

	return res;
}

struct single_link_node *circle_link_get_first_node(struct circle_link *link)
{
	struct single_link_node *node;

	pthread_mutex_lock(&link->lock);

	node = link->head_node.next;
	if (node == &link->head_node)
	{
		node = NULL;
	}

	pthread_mutex_unlock(&link->lock);

	return node;
}

void circle_link_append(struct circle_link *link, struct single_link_node *node)
{
	struct single_link_node *head, *prev;

	pthread_mutex_lock(&link->lock);

	for (prev = head = &link->head_node; prev->next != head; prev = prev->next);

	single_link_insert_base(prev, node);

	pthread_mutex_unlock(&link->lock);
}

void circle_link_insert(struct circle_link *link, struct single_link_node *prev, struct single_link_node *node)
{
	pthread_mutex_lock(&link->lock);
	single_link_insert_base(prev, node);
	pthread_mutex_unlock(&link->lock);
}

bool circle_link_remove(struct circle_link *link, struct single_link_node *node)
{
	struct single_link_node *head, *prev;

	pthread_mutex_lock(&link->lock);

	for (prev = head = &link->head_node; prev->next != head; prev = prev->next)
	{
		if (prev->next == node)
		{
			circle_link_remove_base(prev, node);
			pthread_mutex_unlock(&link->lock);
			return true;
		}
	}

	pthread_mutex_unlock(&link->lock);

	return false;
}

void circle_link_push(struct circle_link *link, struct single_link_node *node)
{
	pthread_mutex_lock(&link->lock);
	single_link_insert_base(&link->head_node, node);
	pthread_mutex_unlock(&link->lock);
}

struct single_link_node *circle_link_pop(struct circle_link *link)
{
	struct single_link_node *node;

	pthread_mutex_lock(&link->lock);

	node = link->head_node.next;
	if (node == &link->head_node)
	{
		node = NULL;
	}
	else
	{
		single_link_remove_base(&link->head_node, node);
	}

	pthread_mutex_unlock(&link->lock);

	return node;
}

void circle_link_destroy(struct circle_link *link, void *data, circle_link_handler_t handler)
{
	struct single_link_node *head, *node, *next;

	pthread_mutex_lock(&link->lock);

	head = &link->head_node;
	node = head->next;

	while (node != head)
	{
		next = node->next;
		handler(link, node, data);
		node = next;
	}

	circle_link_node_init(&link->head_node);

	pthread_mutex_unlock(&link->lock);
}

void circle_link_traversal(struct circle_link *link, void *data, circle_link_handler_t handler)
{
	struct single_link_node *head, *node;

	pthread_mutex_lock(&link->lock);

	for (head = &link->head_node, node = head->next; node != head; node = node->next)
	{
		handler(link, node, data);
	}

	pthread_mutex_unlock(&link->lock);
}

struct single_link_node *circle_link_find(struct circle_link *link, void *data, circle_link_matcher_t matcher)
{
	struct single_link_node *head, *node;

	pthread_mutex_lock(&link->lock);

	for (head = &link->head_node, node = head->next; node != head; node = node->next)
	{
		if (matcher(link, node, data))
		{
			pthread_mutex_unlock(&link->lock);

			return node;
		}
	}

	pthread_mutex_unlock(&link->lock);

	return NULL;
}

bool circle_link_has_node(struct circle_link *link, struct single_link_node *node)
{
	return circle_link_find(link, node, circle_link_node_match_equal) != NULL;
}

void circle_link_free_all(struct circle_link *link)
{
	circle_link_destroy(link, NULL, circle_link_free_handler);
}

// ================================================================================

int double_link_init(struct double_link *link, int offset)
{
	link->offset = offset;
	double_link_node_init(&link->head_node);

	return pthread_mutex_init(&link->lock, NULL);
}

bool double_link_empty(struct double_link *link)
{
	bool res;

	pthread_mutex_lock(&link->lock);
	res = link->head_node.next == &link->head_node;
	pthread_mutex_unlock(&link->lock);

	return res;
}

struct double_link_node *double_link_get_first_node(struct double_link *link)
{
	struct double_link_node *node;

	pthread_mutex_lock(&link->lock);

	node = link->head_node.next;
	if (node == &link->head_node)
	{
		node = NULL;
	}

	pthread_mutex_unlock(&link->lock);

	return node;
}

struct double_link_node *double_link_get_last_node(struct double_link *link)
{
	struct double_link_node *node;

	pthread_mutex_lock(&link->lock);

	node = link->head_node.prev;
	if (node == &link->head_node)
	{
		node = NULL;
	}

	pthread_mutex_unlock(&link->lock);

	return node;
}

void double_link_insert(struct double_link *link, struct double_link_node *next, struct double_link_node *node)
{
	pthread_mutex_lock(&link->lock);
	double_link_insert_base(next->prev, next, node);
	pthread_mutex_unlock(&link->lock);
}

void double_link_insert2(struct double_link *link, struct double_link_node *prev, struct double_link_node *node)
{
	pthread_mutex_lock(&link->lock);
	double_link_insert_base(prev, prev->next, node);
	pthread_mutex_unlock(&link->lock);
}

void double_link_append(struct double_link *link, struct double_link_node *node)
{
	pthread_mutex_lock(&link->lock);
	double_link_insert_base2(&link->head_node, node);
	pthread_mutex_unlock(&link->lock);
}

void double_link_push(struct double_link *link, struct double_link_node *node)
{
	pthread_mutex_lock(&link->lock);
	double_link_insert_base3(&link->head_node, node);
	pthread_mutex_unlock(&link->lock);
}

void double_link_remove(struct double_link *link, struct double_link_node *node)
{
	pthread_mutex_lock(&link->lock);
	double_link_remove_base(node);
	pthread_mutex_unlock(&link->lock);
}

struct double_link_node *double_link_pop(struct double_link *link)
{
	struct double_link_node *node;

	pthread_mutex_lock(&link->lock);

	node = link->head_node.next;
	if (node == &link->head_node)
	{
		node = NULL;
	}
	else
	{
		double_link_remove_base(node);
	}

	pthread_mutex_unlock(&link->lock);

	return node;
}

void double_link_destroy(struct double_link *link, void *data, double_link_handler_t handler)
{
	struct double_link_node *head, *node, *next;

	pthread_mutex_lock(&link->lock);

	head = &link->head_node;
	node = head->next;
	while (node != head)
	{
		next = node->next;
		handler(link, node, data);
		node = next;
	}

	double_link_node_init(&link->head_node);

	pthread_mutex_unlock(&link->lock);
}

void double_link_traversal(struct double_link *link, void *data, double_link_handler_t handler)
{
	struct double_link_node *head, *node;

	pthread_mutex_lock(&link->lock);

	for (head = &link->head_node, node = head->next; node != head; node = node->next)
	{
		handler(link, node, data);
	}

	pthread_mutex_unlock(&link->lock);
}

void double_link_traversal2(struct double_link *link, void *data, double_link_handler_t handler)
{
	struct double_link_node *head, *node;

	pthread_mutex_lock(&link->lock);

	for (head = &link->head_node, node = head->prev; node != head; node = node->prev)
	{
		handler(link, node, data);
	}

	pthread_mutex_unlock(&link->lock);
}

static struct double_link_node *double_link_find_base(struct double_link *link, void *data, double_link_matcher_t matcher)
{
	struct double_link_node *head, *node;

	for (head = &link->head_node, node = head->next; node != head; node = node->next)
	{
		if (matcher(link, node, data))
		{
			return node;
		}
	}

	return NULL;
}

struct double_link_node *double_link_find(struct double_link *link, void *data, double_link_matcher_t matcher)
{
	struct double_link_node *node;

	pthread_mutex_lock(&link->lock);
	node = double_link_find_base(link, data, matcher);
	pthread_mutex_unlock(&link->lock);

	return node;
}

bool double_link_has_node(struct double_link *link, struct double_link_node *node)
{
	return double_link_find(link, node, double_link_node_match_equal) != NULL;
}

struct double_link_node *double_link_get_node_base(struct double_link *link, int index)
{
	struct double_link_node *head, *node;

	for (head = &link->head_node, node = head->next; node != head; node = node->next, index--)
	{
		if (index == 0)
		{
			return node;
		}
	}

	return NULL;
}

struct double_link_node *double_link_get_node(struct double_link *link, int index)
{
	struct double_link_node *node;

	pthread_mutex_lock(&link->lock);
	node = double_link_get_node_base(link, index);
	pthread_mutex_unlock(&link->lock);

	return node;
}

void double_link_cond_insert_append(struct double_link *link, struct double_link_node *node, void *data, double_link_matcher_t matcher)
{
	struct double_link_node *next;

	pthread_mutex_lock(&link->lock);

	next = double_link_find_base(link, data, matcher);
	if (next)
	{
		double_link_insert_base(next->prev, next, node);
	}
	else
	{
		double_link_insert_base2(&link->head_node, node);
	}

	pthread_mutex_unlock(&link->lock);
}

void double_link_cond_insert_push(struct double_link *link, struct double_link_node *node, void *data, double_link_matcher_t matcher)
{
	struct double_link_node *next;

	pthread_mutex_lock(&link->lock);

	next = double_link_find_base(link, data, matcher);
	if (next)
	{
		double_link_insert_base(next->prev, next, node);
	}
	else
	{
		double_link_insert_base3(&link->head_node, node);
	}

	pthread_mutex_unlock(&link->lock);
}

void double_link_move(struct double_link *link, struct double_link_node *next, struct double_link_node *node)
{
	pthread_mutex_lock(&link->lock);

	if (node->next != next)
	{
		double_link_move_base(next->prev, next, node);
	}

	pthread_mutex_unlock(&link->lock);
}

void double_link_move2(struct double_link *link, struct double_link_node *prev, struct double_link_node *node)
{
	pthread_mutex_lock(&link->lock);

	if (node->prev != prev)
	{
		double_link_move_base(prev, prev->next, node);
	}

	pthread_mutex_unlock(&link->lock);
}

void double_link_move_to_head(struct double_link *link, struct double_link_node *node)
{
	struct double_link_node *head;

	pthread_mutex_lock(&link->lock);

	head = &link->head_node;
	if (head->next != node)
	{
		double_link_move_base(head, head->next, node);
	}

	pthread_mutex_unlock(&link->lock);
}

void double_link_move_to_tail(struct double_link *link, struct double_link_node *node)
{
	struct double_link_node *head;

	pthread_mutex_lock(&link->lock);

	head = &link->head_node;
	if (node->next != head)
	{
		double_link_move_base(head->prev, head, node);
	}

	pthread_mutex_unlock(&link->lock);
}

void double_link_free_all(struct double_link *link)
{
	double_link_destroy(link, NULL, double_link_free_handler);
}

// ================================================================================

bool array_has_element(int element, const int a[], size_t size)
{
	const int *a_end;

	for (a_end = a + size; a < a_end; a++)
	{
		if (*a == element)
		{
			return true;
		}
	}

	return false;
}
