#include <cavan.h>
#include <cavan/tree.h>
#include <cavan/stack.h>
#include <cavan/queue.h>

// Fuang.Cao <cavan.cfa@gmail.com> 2011-12-02 10:35:54

int binary_tree_init(struct binary_tree_descriptor *desc)
{
	struct binary_tree_cache *p, *p_end;

	desc->root = NULL;
	desc->compare = NULL;
	desc->traversal = NULL;

	for (p = desc->buff, p_end = p + ARRAY_SIZE(desc->buff); p < p_end ; p++)
	{
		p->flags = 0;
	}

	desc->count = 0;
	desc->buff_end = p_end;

	return 0;
}

struct binary_tree_node *binary_tree_malloc_node(struct binary_tree_descriptor *desc, void *data, struct binary_tree_node *left, struct binary_tree_node *right)
{
	struct binary_tree_node *p;

	if (desc->count < ARRAY_SIZE(desc->buff))
	{
		struct binary_tree_cache *q;

		for (q = desc->buff; q->flags; q++);

		p = &q->node;
		q->flags = 1;
	}
	else
	{
		p = malloc(sizeof(*p));
		if (p == NULL)
		{
			return NULL;
		}
	}

	p->data = data;
	p->left = left;
	p->right = right;

	return p;
}

void binary_tree_free_node(struct binary_tree_descriptor *desc, struct binary_tree_node *node)
{
	if ((struct binary_tree_cache *)node >= desc->buff && (struct binary_tree_cache *) node < desc->buff_end)
	{
		((struct binary_tree_cache *)node)->flags = 0;
	}
	else
	{
		free(node);
	}
}

int binary_tree_insert(struct binary_tree_descriptor *desc, void *data)
{
	int (*compare)(const void *, const void *);
	struct binary_tree_node **pp, *p;

	compare = desc->compare;
	if (compare == NULL)
	{
		error_msg("compare function is NULL");
		return -ENODATA;
	}

	pp = &desc->root;

	while ((p = *pp))
	{
		if (compare(data, p->data) < 0)
		{
			pp = &p->left;
		}
		else
		{
			pp = &p->right;
		}
	}

	p = binary_tree_malloc_node(desc, data, NULL, NULL);
	if (p == NULL)
	{
		error_msg("binary_tree_malloc_node");
		return -ENOMEM;
	}

	*pp = p;

	return 0;
}

int binary_tree_traversal_first(struct binary_tree_descriptor *desc)
{
	int ret;
	void (*traversal)(void *);
	struct general_stack stack;
	struct binary_tree_node *p;

	traversal = desc->traversal;
	if (traversal == NULL)
	{
		return -ENODATA;
	}

	ret = general_stack_init_ed(&stack, TREE_MAX_DEPTH);
	if (ret < 0)
	{
		return ret;
	}

	p = desc->root;

	while (1)
	{
		while (p)
		{
			traversal(p->data);

			ret = general_stack_push_ed(&stack, p);
			if (ret < 0)
			{
				goto out_free_stack;
			}

			p = p->left;
		}

		p = general_stack_pop_ed(&stack);
		if (p == NULL)
		{
			break;
		}

		p = p->right;
	}

	ret = 0;

out_free_stack:
	general_stack_free(&stack);

	return ret;
}

int binary_tree_traversal_middle(struct binary_tree_descriptor *desc)
{
	int ret;
	void (*traversal)(void *);
	struct general_stack stack;
	struct binary_tree_node *p;

	traversal = desc->traversal;
	if (traversal == NULL)
	{
		return -ENODATA;
	}

	ret = general_stack_init_ed(&stack, TREE_MAX_DEPTH);
	if (ret < 0)
	{
		return ret;
	}

	p = desc->root;

	while (1)
	{
		while (p)
		{
			ret = general_stack_push_ed(&stack, p);
			if (ret < 0)
			{
				goto out_free_stack;
			}

			p = p->left;
		}

		p = general_stack_pop_ed(&stack);
		if (p == NULL)
		{
			break;
		}

		traversal(p->data);

		p = p->right;
	}

	ret = 0;

out_free_stack:
	general_stack_free(&stack);

	return ret;
}

int binary_tree_traversal_later(struct binary_tree_descriptor *desc)
{
	int ret;
	void (*traversal)(void *);
	struct general_stack stack, right_stack;
	struct binary_tree_node *p;

	traversal = desc->traversal;
	if (traversal == NULL)
	{
		return -ENODATA;
	}

	ret = general_stack_init_ed(&stack, TREE_MAX_DEPTH);
	if (ret < 0)
	{
		return ret;
	}

	ret = general_stack_init_ed(&right_stack, TREE_MAX_DEPTH);
	if (ret < 0)
	{
		goto out_free_stack;
	}

	p = desc->root;

	while (1)
	{
		while (p)
		{
			ret = general_stack_push_ed(&stack, p);
			if (ret < 0)
			{
				goto out_free_right_stack;
			}

			p = p->left;
		}

		p = general_stack_get_top_ed(&stack);
		if (p == NULL)
		{
			break;
		}

		if (p->right)
		{
			if (p != general_stack_get_top_ed(&right_stack))
			{
				ret = general_stack_push_ed(&right_stack, p);
				if (ret < 0)
				{
					goto out_free_right_stack;
				}
				p = p->right;
				continue;
			}

			general_stack_pop_ed(&right_stack);
		}

		traversal(p->data);
		general_stack_pop_ed(&stack);
		p = NULL;
	}

	ret = 0;

out_free_right_stack:
	general_stack_free(&right_stack);
out_free_stack:
	general_stack_free(&stack);

	return ret;
}

int binary_tree_traversal_level(struct binary_tree_descriptor *desc)
{
	int ret;
	struct list_queue queue;
	struct binary_tree_node *p;
	void (*traversal)(void *);

	traversal = desc->traversal;
	if (traversal == NULL)
	{
		return -ENODATA;
	}

	ret = list_queue_init(&queue, TREE_MAX_DEPTH);
	if (ret < 0)
	{
		return ret;
	}

	p = desc->root;

	while (p)
	{
		traversal(p->data);

		if (p->left)
		{
			ret = list_queue_insert(&queue, p->left);
			if (ret < 0)
			{
				goto out_free_queue;
			}
		}

		if (p->right)
		{
			ret = list_queue_insert(&queue, p->right);
			if (ret < 0)
			{
				goto out_free_queue;
			}
		}

		p = list_queue_remove(&queue);
	}

	ret = 0;

out_free_queue:
	list_queue_free(&queue);

	return ret;
}

static void binary_tree_traversal_first_recursion_body(struct binary_tree_node *root, void (*traversal)(void *))
{
	if (root)
	{
		traversal(root->data);
		binary_tree_traversal_first_recursion_body(root->left, traversal);
		binary_tree_traversal_first_recursion_body(root->right, traversal);
	}
}

int binary_tree_traversal_first_recursion(struct binary_tree_descriptor *desc)
{
	if (desc->traversal == NULL)
	{
		return -ENODATA;
	}

	binary_tree_traversal_first_recursion_body(desc->root, desc->traversal);

	return 0;
}

static void binary_tree_traversal_middle_recursion_body(struct binary_tree_node *root, void (*traversal)(void *))
{
	if (root)
	{
		binary_tree_traversal_middle_recursion_body(root->left, traversal);
		traversal(root->data);
		binary_tree_traversal_middle_recursion_body(root->right, traversal);
	}
}

int binary_tree_traversal_middle_recursion(struct binary_tree_descriptor *desc)
{
	if (desc->traversal == NULL)
	{
		return -ENODATA;
	}

	binary_tree_traversal_middle_recursion_body(desc->root, desc->traversal);

	return 0;
}

static void binary_tree_traversal_later_recursion_body(struct binary_tree_node *root, void (*traversal)(void *))
{
	if (root)
	{
		binary_tree_traversal_later_recursion_body(root->left, traversal);
		binary_tree_traversal_later_recursion_body(root->right, traversal);
		traversal(root->data);
	}
}

int binary_tree_traversal_later_recursion(struct binary_tree_descriptor *desc)
{
	if (desc->traversal == NULL)
	{
		return -ENODATA;
	}

	binary_tree_traversal_later_recursion_body(desc->root, desc->traversal);

	return 0;
}
