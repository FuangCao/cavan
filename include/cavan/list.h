#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Wed Apr 13 10:40:43 CST 2011

#include <cavan.h>
#include <pthread.h>

struct single_link_node
{
	struct single_link_node *next;

	void (*destroy)(void *pointer);
};

struct single_link
{
	long offset;
	pthread_mutex_t lock;

	struct single_link_node head_node;
};

struct circle_link
{
	long offset;
	pthread_mutex_t lock;

	struct single_link_node head_node;
};

struct double_link_node
{
	struct double_link_node *prev;
	struct double_link_node *next;

	void (*destroy)(void *pointer);
};

struct double_link
{
	long offset;
	pthread_mutex_t lock;

	struct double_link_node head_node;
};

typedef void (*single_link_handler_t)(struct single_link *link, struct single_link_node *node, void *data);
typedef bool (*single_link_matcher_t)(struct single_link *link, struct single_link_node *node, void *data);

typedef void (*circle_link_handler_t)(struct circle_link *link, struct single_link_node *node, void *data);
typedef bool (*circle_link_matcher_t)(struct circle_link *link, struct single_link_node *node, void *data);

typedef void (*double_link_handler_t)(struct double_link *link, struct double_link_node *node, void *data);
typedef bool (*double_link_matcher_t)(struct double_link *link, struct double_link_node *node, void *data);

int single_link_init(struct single_link *link, long offset);
void single_link_deinit(struct single_link *link);
void single_link_free(struct single_link *link);
bool single_link_empty(struct single_link *link);
struct single_link_node *single_link_get_first_node(struct single_link *link);
void single_link_insert(struct single_link *link, struct single_link_node *prev, struct single_link_node *node);
void single_link_append(struct single_link *link, struct single_link_node *node);
bool single_link_remove(struct single_link *link, struct single_link_node *node);
void single_link_push(struct single_link *link, struct single_link_node *node);
struct single_link_node *single_link_pop(struct single_link *link);
void single_link_traversal(struct single_link *link, void *data, single_link_handler_t handler);
struct single_link_node *single_link_find(struct single_link *link, void *data, single_link_matcher_t macher);
bool single_link_has_node(struct single_link *link, struct single_link_node *node);

int circle_link_init(struct circle_link *link, long offset);
void circle_link_deinit(struct circle_link *link);
void circle_link_free(struct circle_link *link);
bool circle_link_empty(struct circle_link *link);
struct single_link_node *circle_link_get_first_node(struct circle_link *link);
void circle_link_append(struct circle_link *link, struct single_link_node *node);
void circle_link_insert(struct circle_link *link, struct single_link_node *prev, struct single_link_node *node);
bool circle_link_remove(struct circle_link *link, struct single_link_node *node);
void circle_link_push(struct circle_link *link, struct single_link_node *node);
struct single_link_node *circle_link_pop(struct circle_link *link);
void circle_link_traversal(struct circle_link *link, void *data, circle_link_handler_t handler);
struct single_link_node *circle_link_find(struct circle_link *link, void *data, circle_link_matcher_t matcher);
bool circle_link_has_node(struct circle_link *link, struct single_link_node *node);

int double_link_init(struct double_link *link, long offset);
void double_link_deinit(struct double_link *link);
void double_link_free(struct double_link *link);
bool double_link_empty(struct double_link *link);
struct double_link_node *double_link_get_first_node(struct double_link *link);
struct double_link_node *double_link_get_last_node(struct double_link *link);
void double_link_insert(struct double_link *link, struct double_link_node *prev, struct double_link_node *node);
void doubel_link_insert2(struct double_link *link, struct double_link_node *next, struct double_link_node *node);
bool double_link_remove(struct double_link *link, struct double_link_node *node);
void double_link_append(struct double_link *link, struct double_link_node *node);
void double_link_push(struct double_link *link, struct double_link_node *node);
struct double_link_node *double_link_pop(struct double_link *link);
void double_link_traversal(struct double_link *link, void *data, double_link_handler_t handler);
void double_link_traversal2(struct double_link *link, void *data, double_link_handler_t handler);
struct double_link_node *double_link_find(struct double_link *link, void *data, double_link_matcher_t matcher);
bool double_link_has_node(struct double_link *link, struct double_link_node *node);
void double_link_cond_insert_append(struct double_link *link, struct double_link_node *node, void *data, double_link_matcher_t matcher);
void double_link_cond_insert_push(struct double_link *link, struct double_link_node *node, void *data, double_link_matcher_t matcher);

bool array_has_element(int element, const int a[], size_t size);

static inline void single_link_node_init(struct single_link_node *node, void (*destroy)(void *pointer))
{
	node->next = NULL;
	node->destroy = destroy;
}

static inline void double_link_node_init(struct double_link_node *node, void (*destroy)(void *pointer))
{
	node->next = NULL;
	node->prev = NULL;
	node->destroy = destroy;
}

static inline void *single_link_get_container(struct single_link *link, struct single_link_node *node)
{
	return POINTER_SUB(node, link->offset);
}

static inline void *circle_link_get_container(struct circle_link *link, struct single_link_node *node)
{
	return POINTER_SUB(node, link->offset);
}

static inline void *double_link_get_container(struct double_link *link, struct double_link_node *node)
{
	return POINTER_SUB(node, link->offset);
}
