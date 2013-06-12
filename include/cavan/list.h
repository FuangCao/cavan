#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Wed Apr 13 10:40:43 CST 2011

#include <cavan.h>
#include <pthread.h>

#define single_link_foreach(link, entry) \
	do { \
		struct single_link_node *__node, *__next; \
		struct single_link *__link = link; \
		pthread_mutex_lock(&__link->lock); \
		for (__node = __link->head_node.next; __node; __node = __next) \
		{ \
			__next = __node->next; \
			entry = single_link_get_container(__link, __node);

#define end_link_foreach(link) \
		} \
		pthread_mutex_unlock(&(link)->lock); \
	} while (0)

#define link_foreach_return(link, value) \
	do { \
		pthread_mutex_unlock(&(link)->lock); \
		return value; \
	} while (0)

#define circle_link_foreach(link, entry) \
	do { \
		struct single_link_node *__head, *__node, *__next; \
		struct circle_link *__link = link; \
		pthread_mutex_lock(&__link->lock); \
		for (__head = &link->head_node, __node = __head->next; __node != __head; __node = __next) \
		{ \
			__next = __node->next; \
			entry = circle_link_get_container(__link, __node);

#define double_link_foreach(link, entry) \
	do { \
		struct double_link_node *__head, *__node, *__next; \
		struct double_link *__link = link; \
		pthread_mutex_lock(&__link->lock); \
		for (__head = &__link->head_node, __node = __head->next; __node != __head; __node = __next) \
		{ \
			__next = __node->next; \
			entry = double_link_get_container(__link, __node);

struct single_link_node
{
	struct single_link_node *next;

	void (*destroy)(void *addr);
};

struct single_link
{
	int offset;
	pthread_mutex_t lock;

	struct single_link_node head_node;
};

struct circle_link
{
	int offset;
	pthread_mutex_t lock;

	struct single_link_node head_node;
};

struct double_link_node
{
	struct double_link_node *prev;
	struct double_link_node *next;
};

struct double_link
{
	int offset;
	pthread_mutex_t lock;

	struct double_link_node head_node;
};

typedef void (*single_link_handler_t)(struct single_link *link, struct single_link_node *node, void *data);
typedef bool (*single_link_matcher_t)(struct single_link *link, struct single_link_node *node, void *data);

typedef void (*circle_link_handler_t)(struct circle_link *link, struct single_link_node *node, void *data);
typedef bool (*circle_link_matcher_t)(struct circle_link *link, struct single_link_node *node, void *data);

typedef void (*double_link_handler_t)(struct double_link *link, struct double_link_node *node, void *data);
typedef bool (*double_link_matcher_t)(struct double_link *link, struct double_link_node *node, void *data);

int single_link_init(struct single_link *link, int offset);
bool single_link_empty(struct single_link *link);
struct single_link_node *single_link_get_first_node(struct single_link *link);
void single_link_insert(struct single_link *link, struct single_link_node *prev, struct single_link_node *node);
void single_link_append(struct single_link *link, struct single_link_node *node);
bool single_link_remove(struct single_link *link, struct single_link_node *node);
void single_link_push(struct single_link *link, struct single_link_node *node);
struct single_link_node *single_link_pop(struct single_link *link);
void single_link_destroy(struct single_link *link, void *data, single_link_handler_t handler);
void single_link_traversal(struct single_link *link, void *data, single_link_handler_t handler);
struct single_link_node *single_link_find(struct single_link *link, void *data, single_link_matcher_t macher);
bool single_link_has_node(struct single_link *link, struct single_link_node *node);
void single_link_free_all(struct single_link *link);

int circle_link_init(struct circle_link *link, int offset);
bool circle_link_empty(struct circle_link *link);
struct single_link_node *circle_link_get_first_node(struct circle_link *link);
void circle_link_append(struct circle_link *link, struct single_link_node *node);
void circle_link_insert(struct circle_link *link, struct single_link_node *prev, struct single_link_node *node);
bool circle_link_remove(struct circle_link *link, struct single_link_node *node);
void circle_link_push(struct circle_link *link, struct single_link_node *node);
struct single_link_node *circle_link_pop(struct circle_link *link);
void circle_link_destroy(struct circle_link *link, void *data, circle_link_handler_t handler);
void circle_link_traversal(struct circle_link *link, void *data, circle_link_handler_t handler);
struct single_link_node *circle_link_find(struct circle_link *link, void *data, circle_link_matcher_t matcher);
bool circle_link_has_node(struct circle_link *link, struct single_link_node *node);
void circle_link_free_all(struct circle_link *link);

int double_link_init(struct double_link *link, int offset);
bool double_link_empty(struct double_link *link);
struct double_link_node *double_link_get_first_node(struct double_link *link);
struct double_link_node *double_link_get_last_node(struct double_link *link);
void double_link_insert(struct double_link *link, struct double_link_node *next, struct double_link_node *node);
void double_link_insert2(struct double_link *link, struct double_link_node *prev, struct double_link_node *node);
void double_link_remove(struct double_link *link, struct double_link_node *node);
void double_link_append(struct double_link *link, struct double_link_node *node);
void double_link_push(struct double_link *link, struct double_link_node *node);
struct double_link_node *double_link_pop(struct double_link *link);
void double_link_destroy(struct double_link *link, void *data, double_link_handler_t handler);
void double_link_traversal(struct double_link *link, void *data, double_link_handler_t handler);
void double_link_traversal2(struct double_link *link, void *data, double_link_handler_t handler);
struct double_link_node *double_link_find(struct double_link *link, void *data, double_link_matcher_t matcher);
bool double_link_has_node(struct double_link *link, struct double_link_node *node);
struct double_link_node *double_link_get_node_base(struct double_link *link, int index);
struct double_link_node *double_link_get_node(struct double_link *link, int index);
void double_link_cond_insert_append(struct double_link *link, struct double_link_node *node, void *data, double_link_matcher_t matcher);
void double_link_cond_insert_push(struct double_link *link, struct double_link_node *node, void *data, double_link_matcher_t matcher);
void double_link_move(struct double_link *link, struct double_link_node *next, struct double_link_node *node);
void double_link_move2(struct double_link *link, struct double_link_node *prev, struct double_link_node *node);
void double_link_move_to_head(struct double_link *link, struct double_link_node *node);
void double_link_move_to_tail(struct double_link *link, struct double_link_node *node);
void double_link_free_all(struct double_link *link);

bool array_has_element(int element, const int a[], size_t size);

// ================================================================================

static inline void single_link_node_init(struct single_link_node *node)
{
	node->next = NULL;
}

static inline void circle_link_node_init(struct single_link_node *node)
{
	node->next = node;
}

static inline void double_link_node_init(struct double_link_node *node)
{
	node->next = node->prev = node;
}

static inline void single_link_deinit(struct single_link *link)
{
	pthread_mutex_destroy(&link->lock);
}

static inline void circle_link_deinit(struct circle_link *link)
{
	pthread_mutex_destroy(&link->lock);
}

static inline void double_link_deinit(struct double_link *link)
{
	pthread_mutex_destroy(&link->lock);
}

static inline void single_link_insert_base(struct single_link_node *prev, struct single_link_node *node)
{
	node->next = prev->next;
	prev->next = node;
}

static inline void single_link_remove_base(struct single_link_node *prev, struct single_link_node *node)
{
	prev->next = node->next;
	node->next = NULL;
}

static inline void circle_link_remove_base(struct single_link_node *prev, struct single_link_node *node)
{
	prev->next = node->next;
	node->next = node;
}

static inline void double_link_insert_base(struct double_link_node *prev, struct double_link_node *next, struct double_link_node *node)
{
	prev->next = node;
	node->next = next;

	next->prev = node;
	node->prev = prev;
}

static inline void double_link_insert_base2(struct double_link_node *next, struct double_link_node *node)
{
	double_link_insert_base(next->prev, next, node);
}

static inline inline void double_link_insert_base3(struct double_link_node *prev, struct double_link_node *node)
{
	double_link_insert_base(prev, prev->next, node);
}

// ================================================================================

static inline void double_link_remove_only(struct double_link_node *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
}

static inline void double_link_remove_base(struct double_link_node *node)
{
	double_link_remove_only(node);
	node->next = node->prev = node;
}

static inline void double_link_move_base(struct double_link_node *prev, struct double_link_node *next, struct double_link_node *node)
{
	double_link_remove_only(node);
	double_link_insert_base(prev, next, node);
}

static inline void single_link_remove_all(struct single_link *link)
{
	pthread_mutex_lock(&link->lock);
	single_link_node_init(&link->head_node);
	pthread_mutex_unlock(&link->lock);
}

static inline void circle_link_remove_all(struct circle_link *link)
{
	pthread_mutex_lock(&link->lock);
	circle_link_node_init(&link->head_node);
	pthread_mutex_unlock(&link->lock);
}

static inline void double_link_remove_all(struct double_link *link)
{
	pthread_mutex_lock(&link->lock);
	double_link_node_init(&link->head_node);
	pthread_mutex_unlock(&link->lock);
}

// ================================================================================

static inline void *single_link_get_container(struct single_link *link, struct single_link_node *node)
{
	return ADDR_SUB(node, link->offset);
}

static inline void *circle_link_get_container(struct circle_link *link, struct single_link_node *node)
{
	return ADDR_SUB(node, link->offset);
}

static inline void *double_link_get_container(struct double_link *link, struct double_link_node *node)
{
	return ADDR_SUB(node, link->offset);
}

static inline struct double_link_node *double_link_get_to_node(struct double_link *link, void *addr)
{
	return ADDR_ADD(addr, link->offset);
}

// ================================================================================

static inline bool single_link_node_match_equal(struct single_link *link, struct single_link_node *node, void *data)
{
	return node == (struct single_link_node *)data;
}

static inline void single_link_free_handler(struct single_link *link, struct single_link_node *node, void *data)
{
	free(single_link_get_container(link, node));
}

static inline bool circle_link_node_match_equal(struct circle_link *link, struct single_link_node *node, void *data)
{
	return node == (struct single_link_node *)data;
}

static inline void circle_link_free_handler(struct circle_link *link, struct single_link_node *node, void *data)
{
	free(circle_link_get_container(link, node));
}

static inline bool double_link_node_match_equal(struct double_link *link, struct double_link_node *node, void *data)
{
	return node == (struct double_link_node *)data;
}

static inline void double_link_free_handler(struct double_link *link, struct double_link_node *node, void *data)
{
	free(double_link_get_container(link, node));
}

// ================================================================================

static inline void double_link_lock(struct double_link *link)
{
	pthread_mutex_lock(&link->lock);
}

static inline void double_link_unlock(struct double_link *link)
{
	pthread_mutex_unlock(&link->lock);
}
