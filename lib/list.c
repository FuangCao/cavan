// Fuang.Cao <cavan.cfa@gmail.com> Wed Apr 13 10:40:43 CST 2011

#include <cavan.h>
#include <cavan/list.h>

static void single_link_init_base(struct single_link *link)
{
	link->head_node.next = NULL;
}

static void circle_link_init_base(struct circle_link *link)
{
	link->head_node.next = &link->head_node;
}

static void double_link_init_base(struct double_link *link)
{
	link->head_node.next = &link->head_node;
	link->head_node.prev = &link->head_node;
}

static void single_link_insert_base(struct single_link_node *prev, struct single_link_node *node)
{
	node->next = prev->next;
	prev->next = node;
}

static void single_link_remove_base(struct single_link_node *prev, struct single_link_node *node)
{
	prev->next = node->next;
	node->next = NULL;
}

static void double_link_insert_base(struct double_link_node *prev, struct double_link_node *node)
{
	node->next = prev->next;
	prev->next = node;

	node->prev = prev;
	node->next->prev = node;
}

static bool double_link_remove_base(struct double_link_node *node)
{
	bool res = false;
	struct double_link_node *prev = node->prev;
	struct double_link_node *next = node->next;

	if (prev && next)
	{
		res = true;
		prev->next = node->next;
		next->prev = node->prev;
	}

	node->next = node->prev = NULL;

	return res;
}

static bool single_link_node_match_equal(struct single_link *link, struct single_link_node *node, void *data)
{
	return node == (struct single_link_node *)data;
}

static bool circle_link_node_match_equal(struct circle_link *link, struct single_link_node *node, void *data)
{
	return node == (struct single_link_node *)data;
}

static bool double_link_node_match_equal(struct double_link *link, struct double_link_node *node, void *data)
{
	return node == (struct double_link_node *)data;
}

// ================================================================================

int single_link_init(struct single_link *link, off_t offset)
{
	link->offset = offset;
	link->head_node.destroy = NULL;

	single_link_init_base(link);

	return pthread_mutex_init(&link->lock, NULL);
}

void single_link_deinit(struct single_link *link)
{
	single_link_free(link);

	pthread_mutex_destroy(&link->lock);
}

void single_link_free(struct single_link *link)
{
	struct single_link_node *node;
	off_t offset = link->offset;

	pthread_mutex_lock(&link->lock);

	node = link->head_node.next;

	while (node)
	{
		struct single_link_node *next = node->next;

		if (node->destroy)
		{
			node->destroy(POINTER_SUB(node, offset));
		}

		node = next;
	}

	single_link_init_base(link);

	pthread_mutex_unlock(&link->lock);
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
	bool res = false;
	struct single_link_node *prev;

	pthread_mutex_lock(&link->lock);

	for (prev = &link->head_node; prev; prev = prev->next)
	{
		if (prev->next == node)
		{
			res = true;
			single_link_remove_base(prev, node);
			break;
		}
	}

	pthread_mutex_unlock(&link->lock);

	return res;
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

void single_link_traversal(struct single_link *link, void *data, void (*handle)(struct single_link *link, struct single_link_node *node, void *data))
{
	struct single_link_node *node;

	pthread_mutex_lock(&link->lock);

	for (node = link->head_node.next; node; node = node->next)
	{
		handle(link, node, data);
	}

	pthread_mutex_unlock(&link->lock);
}

struct single_link_node *single_link_find(struct single_link *link, void *data, bool (*match)(struct single_link *link, struct single_link_node *node, void *data))
{
	struct single_link_node *node;

	pthread_mutex_lock(&link->lock);

	for (node = link->head_node.next; node; node = node->next)
	{
		if (match(link, node, data))
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

// ================================================================================

int circle_link_init(struct circle_link *link, off_t offset)
{
	link->offset = offset;
	link->head_node.destroy = NULL;

	circle_link_init_base(link);

	return pthread_mutex_init(&link->lock, NULL);
}

void circle_link_deinit(struct circle_link *link)
{
	circle_link_free(link);

	pthread_mutex_destroy(&link->lock);
}

void circle_link_free(struct circle_link *link)
{
	struct single_link_node *head, *node;
	off_t offset = link->offset;

	pthread_mutex_lock(&link->lock);

	head = &link->head_node;
	node = head->next;

	while (node != head)
	{
		struct single_link_node *next = node->next;

		if (node->destroy)
		{
			node->destroy(POINTER_SUB(node, offset));
		}

		node = next;
	}

	circle_link_init_base(link);

	pthread_mutex_unlock(&link->lock);
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
	bool res = false;
	struct single_link_node *head, *prev;

	pthread_mutex_lock(&link->lock);

	for (prev = head = &link->head_node; prev->next != head; prev = prev->next)
	{
		if (prev->next == node)
		{
			res = true;
			single_link_remove_base(prev, node);
			break;
		}
	}

	pthread_mutex_unlock(&link->lock);

	return res;
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

void circle_link_traversal(struct circle_link *link, void *data, void (*handle)(struct circle_link *link, struct single_link_node *node, void *data))
{
	struct single_link_node *head, *node;

	pthread_mutex_lock(&link->lock);

	for (head = &link->head_node, node = head->next; node != head; node = node->next)
	{
		handle(link, node, data);
	}

	pthread_mutex_unlock(&link->lock);
}

struct single_link_node *circle_link_find(struct circle_link *link, void *data, bool (*match)(struct circle_link *link, struct single_link_node *node, void *data))
{
	struct single_link_node *head, *node;

	pthread_mutex_lock(&link->lock);

	for (head = &link->head_node, node = head->next; node != head; node = node->next)
	{
		if (match(link, node, data))
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

// ================================================================================

int double_link_init(struct double_link *link, off_t offset)
{
	link->offset = offset;
	link->head_node.destroy = NULL;

	double_link_init_base(link);

	return pthread_mutex_init(&link->lock, NULL);
}

void double_link_deinit(struct double_link *link)
{
	double_link_free(link);

	pthread_mutex_destroy(&link->lock);
}

void double_link_free(struct double_link *link)
{
	struct double_link_node *head, *node;
	off_t offset = link->offset;

	pthread_mutex_lock(&link->lock);

	head = &link->head_node;
	node = head->next;

	while (node != head)
	{
		struct double_link_node *next = node->next;

		if (node->destroy)
		{
			node->destroy(POINTER_SUB(node, offset));
		}

		node = next;
	}

	double_link_init_base(link);

	pthread_mutex_unlock(&link->lock);
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

void double_link_insert(struct double_link *link, struct double_link_node *prev, struct double_link_node *node)
{
	pthread_mutex_lock(&link->lock);
	double_link_insert_base(prev, node);
	pthread_mutex_unlock(&link->lock);
}

void doubel_link_insert2(struct double_link *link, struct double_link_node *next, struct double_link_node *node)
{
	pthread_mutex_lock(&link->lock);
	double_link_insert_base(next->prev, node);
	pthread_mutex_unlock(&link->lock);
}

bool double_link_remove(struct double_link *link, struct double_link_node *node)
{
	bool res;

	pthread_mutex_lock(&link->lock);
	res = double_link_remove_base(node);
	pthread_mutex_unlock(&link->lock);

	return res;
}

void double_link_append(struct double_link *link, struct double_link_node *node)
{
	pthread_mutex_lock(&link->lock);
	double_link_insert_base(link->head_node.prev, node);
	pthread_mutex_unlock(&link->lock);
}

void double_link_push(struct double_link *link, struct double_link_node *node)
{
	pthread_mutex_lock(&link->lock);
	double_link_insert_base(&link->head_node, node);
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

void double_link_traversal(struct double_link *link, void *data, void (*handle)(struct double_link *link, struct double_link_node *node, void *data))
{
	struct double_link_node *head, *node;

	pthread_mutex_lock(&link->lock);

	for (head = &link->head_node, node = head->next; node != head; node = node->next)
	{
		handle(link, node, data);
	}

	pthread_mutex_unlock(&link->lock);
}

void double_link_traversal2(struct double_link *link, void *data, void (*handle)(struct double_link *link, struct double_link_node *node, void *data))
{
	struct double_link_node *head, *node;

	pthread_mutex_lock(&link->lock);

	for (head = &link->head_node, node = head->prev; node != head; node = node->prev)
	{
		handle(link, node, data);
	}

	pthread_mutex_unlock(&link->lock);
}

struct double_link_node *double_link_find(struct double_link *link, void *data, bool (*match)(struct double_link *link, struct double_link_node *node, void *data))
{
	struct double_link_node *head, *node;

	pthread_mutex_lock(&link->lock);

	for (head = &link->head_node, node = head->next; node != head; node = node->next)
	{
		if (match(link, node, data))
		{
			pthread_mutex_unlock(&link->lock);
			return node;
		}
	}

	pthread_mutex_unlock(&link->lock);

	return NULL;
}

bool double_link_has_node(struct double_link *link, struct double_link_node *node)
{
	return double_link_find(link, node, double_link_node_match_equal) != NULL;
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
