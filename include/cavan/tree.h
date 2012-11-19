#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> 2011-12-02 10:35:54

#define TREE_MAX_DEPTH		100
#define TREE_BUFFER_SIZE	100

struct binary_tree_node
{
	void *data;

	struct binary_tree_node *left;
	struct binary_tree_node *right;
};

struct binary_tree_cache
{
	struct binary_tree_node node;
	int flags;
};

struct binary_tree_descriptor
{
	struct binary_tree_node *root;
	struct binary_tree_cache buff[TREE_BUFFER_SIZE], *buff_end;
	size_t count;
	int (*compare)(const void *data1, const void *data2);
	void (*traversal)(void *data);
};

int binary_tree_init(struct binary_tree_descriptor *desc);
struct binary_tree_node *binary_tree_malloc_node(struct binary_tree_descriptor *desc, void *data, struct binary_tree_node *left, struct binary_tree_node *right);
int binary_tree_insert(struct binary_tree_descriptor *desc, void *data);

int binary_tree_traversal_first(struct binary_tree_descriptor *desc);
int binary_tree_traversal_middle(struct binary_tree_descriptor *desc);
int binary_tree_traversal_later(struct binary_tree_descriptor *desc);
int binary_tree_traversal_level(struct binary_tree_descriptor *desc);

int binary_tree_traversal_first_recursion(struct binary_tree_descriptor *desc);
int binary_tree_traversal_middle_recursion(struct binary_tree_descriptor *desc);
int binary_tree_traversal_later_recursion(struct binary_tree_descriptor *desc);

