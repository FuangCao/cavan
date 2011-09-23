#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Wed Apr 13 10:40:43 CST 2011

struct single_link_node
{
	struct single_link_node *next;
};

struct single_link
{
	struct single_link_node *head;
};

struct circle_link
{
	struct single_link_node head_node;
};

struct double_link_node
{
	struct double_link_node *pre;
	struct double_link_node *next;
};

struct double_link
{
	struct double_link_node head_node;
};

void single_link_init(struct single_link *link);
void single_link_free(struct single_link *link, off_t offset);
void single_link_insert_back(struct single_link_node *pre, struct single_link_node *node);
void single_link_insert_head(struct single_link *link, struct single_link_node *node);
void single_link_append(struct single_link *link, struct single_link_node *node);
void single_link_traversal(struct single_link *link, void (*handle)(struct single_link_node *node));
struct single_link_node *single_link_find(struct single_link *link, void *data, int (*match)(struct single_link_node *node, void *data));
void single_link_delete_node(struct single_link *link, struct single_link_node *node);

void circle_link_init(struct circle_link *link);
void circle_link_free(struct circle_link *link, off_t offset);
void circle_link_append(struct circle_link *link, struct single_link_node *node);
void circle_link_insert_head(struct circle_link *link, struct single_link_node *node);
void circle_link_delete_node(struct single_link_node *node);
void circle_link_traversal(struct circle_link *link, void (*handle)(struct single_link_node *node));
struct single_link_node *circle_link_find(struct circle_link *link, void *data, int (*match)(struct single_link_node *node, void *data));

void double_link_init(struct double_link *link);
void double_link_free(struct double_link *link, off_t offset);
void double_link_insert_simple(struct double_link_node *pre, struct double_link_node *next, struct double_link_node *node);
void double_link_insert_back(struct double_link_node *pre, struct double_link_node *node);
void doubel_link_insert_front(struct double_link_node *next, struct double_link_node *node);
void double_link_insert_head(struct double_link *link, struct double_link_node *node);
void double_link_append(struct double_link *link, struct double_link_node *node);
void double_link_traversal(struct double_link *link, void (*handle)(struct double_link_node *node));
struct double_link_node *double_link_find(struct double_link *link, void *data, int (*match)(struct double_link_node *node, void *data));
void double_link_delete_node(struct double_link_node *node);

