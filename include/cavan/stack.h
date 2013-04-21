#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Wed Apr 13 10:47:15 CST 2011

#include <cavan.h>

struct general_stack
{
	void **buffer;
	int top;
	int max_count;
};

struct letter_stack
{
	char *buffer;
	int top;
	int max_count;
};

struct double_stack
{
	double *buffer;
	int top;
	int max_count;
};

struct memory_stack_node
{
	void *space;
	size_t size;
};

struct memory_stack
{
	struct memory_stack_node *buffer;
	int top;
	int max_count;
};

int general_stack_init_fr(struct general_stack *stack, int count);
int general_stack_init_er(struct general_stack *stack, int count);
int general_stack_init_fd(struct general_stack *stack, int count);
int general_stack_init_ed(struct general_stack *stack, int count);
void general_stack_free(struct general_stack *stack);

int general_stack_push_fr(struct general_stack *stack, void *data);
int general_stack_push_er(struct general_stack *stack, void *data);
int general_stack_push_fd(struct general_stack *stack, void *data);
int general_stack_push_ed(struct general_stack *stack, void *data);

void *general_stack_pop_fr(struct general_stack *stack);
void *general_stack_pop_er(struct general_stack *stack);
void *general_stack_pop_fd(struct general_stack *stack);
void *general_stack_pop_ed(struct general_stack *stack);

void *general_stack_get_top_fr(struct general_stack *stack);
void *general_stack_get_top_er(struct general_stack *stack);
void *general_stack_get_top_fd(struct general_stack *stack);
void *general_stack_get_top_ed(struct general_stack *stack);

// ============================================================

int letter_stack_init(struct letter_stack *stack, int count);
void letter_stack_free(struct letter_stack *stack);
int letter_stack_push(struct letter_stack *stack, char data);
int letter_stack_pop(struct letter_stack *stack, char *data);
int letter_stack_get_top(struct letter_stack *stack, char *data);

// ============================================================

int double_stack_init(struct double_stack *stack, int count);
void double_stack_free(struct double_stack *stack);
int double_stack_push(struct double_stack *stack, double data);
int double_stack_pop(struct double_stack *stack, double *data);
int double_stack_get_top(struct double_stack *stack, double *data);

// ============================================================

int memory_stack_init(struct memory_stack *stack, int count);
void memory_stack_free(struct memory_stack *stack);
int memory_stack_push(struct memory_stack *stack, void *mem, size_t size);
void *memory_stack_pop(struct memory_stack *stack, void *mem, size_t *size);
void *memory_stack_get_top(struct memory_stack *stack,void *mem, size_t *size);

// ============================================================

static inline int general_stack_empty_fr(struct general_stack *stack)
{
	return stack->top < 0;
}

static inline int general_stack_empty_er(struct general_stack *stack)
{
	return stack->top <= 0;
}

static inline int general_stack_empty_fd(struct general_stack *stack)
{
	return stack->top >= stack->max_count;
}

static inline int general_stack_empty_ed(struct general_stack *stack)
{
	return stack->top >= stack->max_count - 1;
}

static inline int general_stack_full_fr(struct general_stack *stack)
{
	return stack->top >= stack->max_count - 1;
}

static inline int general_stack_full_er(struct general_stack *stack)
{
	return stack->top >= stack->max_count;
}

static inline int general_stack_full_fd(struct general_stack *stack)
{
	return stack->top <= 0;
}

static inline int general_stack_full_ed(struct general_stack *stack)
{
	return stack->top < 0;
}

// ============================================================

static inline int letter_stack_is_empty(struct letter_stack *stack)
{
	return stack->top <= 0;
}

static inline int letter_stack_is_full(struct letter_stack *stack)
{
	return stack->top >= stack->max_count;
}

// ============================================================

static inline int double_stack_is_empty(struct double_stack *stack)
{
	return stack->top <= 0;
}

static inline int double_stack_is_full(struct double_stack *stack)
{
	return stack->top >= stack->max_count;
}

// ============================================================

static inline int memory_stack_is_empty(struct memory_stack *stack)
{
	return stack->top <= 0;
}

static inline int memory_stack_is_full(struct memory_stack *stack)
{
	return stack->top >= stack->max_count;
}

