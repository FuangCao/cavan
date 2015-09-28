// Fuang.Cao <cavan.cfa@gmail.com> Wed Apr 13 10:47:15 CST 2011

#include <cavan.h>
#include <cavan/stack.h>

static int general_stack_init(struct general_stack *stack, int count)
{
	stack->buffer = malloc(sizeof(stack->buffer[0]) * count);
	if (stack->buffer == NULL) {
		return -ENOMEM;
	}

	stack->max_count = count;

	return 0;
}

int general_stack_init_fr(struct general_stack *stack, int count)
{
	int ret;

	ret = general_stack_init(stack, count);
	if (ret < 0) {
		return ret;
	}

	stack->top = -1;

	return 0;
}

int general_stack_init_er(struct general_stack *stack, int count)
{
	int ret;

	ret = general_stack_init(stack, count);
	if (ret < 0) {
		return ret;
	}

	stack->top = 0;

	return 0;
}

int general_stack_init_fd(struct general_stack *stack, int count)
{
	int ret;

	ret = general_stack_init(stack, count);
	if (ret < 0) {
		return ret;
	}

	stack->top = count;

	return 0;
}

int general_stack_init_ed(struct general_stack *stack, int count)
{
	int ret;

	ret = general_stack_init(stack, count);
	if (ret < 0) {
		return ret;
	}

	stack->top = count - 1;

	return 0;
}

void general_stack_free(struct general_stack *stack)
{
	free(stack->buffer);
	stack->max_count = 0;
}

int general_stack_push_fr(struct general_stack *stack, void *data)
{
	if (general_stack_full_fr(stack)) {
		return -ENOMEM;
	}

	stack->buffer[++(stack->top)] = data;

	return 0;
}

int general_stack_push_er(struct general_stack *stack, void *data)
{
	if (general_stack_full_er(stack)) {
		return -ENOMEM;
	}

	stack->buffer[(stack->top)++] = data;

	return 0;
}

int general_stack_push_fd(struct general_stack *stack, void *data)
{
	if (general_stack_full_fd(stack)) {
		return -ENOMEM;
	}

	stack->buffer[--(stack->top)] = data;

	return 0;
}

int general_stack_push_ed(struct general_stack *stack, void *data)
{
	if (general_stack_full_ed(stack)) {
		return -ENOMEM;
	}

	stack->buffer[(stack->top)--] = data;

	return 0;
}

void *general_stack_pop_fr(struct general_stack *stack)
{
	if (general_stack_empty_fr(stack)) {
		return NULL;
	}

	return stack->buffer[(stack->top)--];
}

void *general_stack_pop_er(struct general_stack *stack)
{
	if (general_stack_empty_er(stack)) {
		return NULL;
	}

	return stack->buffer[--(stack->top)];
}

void *general_stack_pop_fd(struct general_stack *stack)
{
	if (general_stack_empty_fd(stack)) {
		return NULL;
	}

	return stack->buffer[(stack->top)++];
}

void *general_stack_pop_ed(struct general_stack *stack)
{
	if (general_stack_empty_ed(stack)) {
		return NULL;
	}

	return stack->buffer[++(stack->top)];
}

void *general_stack_get_top_fr(struct general_stack *stack)
{
	if (general_stack_empty_fr(stack)) {
		return NULL;
	}

	return stack->buffer[stack->top];
}

void *general_stack_get_top_er(struct general_stack *stack)
{
	if (general_stack_empty_er(stack)) {
		return NULL;
	}

	return stack->buffer[stack->top - 1];
}

void *general_stack_get_top_fd(struct general_stack *stack)
{
	if (general_stack_empty_fd(stack)) {
		return NULL;
	}

	return stack->buffer[stack->top];
}

void *general_stack_get_top_ed(struct general_stack *stack)
{
	if (general_stack_empty_ed(stack)) {
		return NULL;
	}

	return stack->buffer[stack->top + 1];
}

// ============================================================

int letter_stack_init(struct letter_stack *stack, int count)
{
	stack->buffer = malloc(sizeof(stack->buffer[0]) * count);
	if (stack->buffer == NULL) {
		return -ENOMEM;
	}

	stack->top = 0;
	stack->max_count = count;

	return 0;
}

void letter_stack_free(struct letter_stack *stack)
{
	if (stack && stack->buffer) {
		free(stack->buffer);
	}
}

int letter_stack_push(struct letter_stack *stack, char data)
{
	if (letter_stack_is_full(stack)) {
		return -ENOMEM;
	}

	stack->buffer[(stack->top)++] = data;

	return 0;
}

int letter_stack_pop(struct letter_stack *stack, char *data)
{
	if (letter_stack_is_empty(stack)) {
		return -ENOMEM;
	}

	*data = stack->buffer[--(stack->top)];

	return 0;
}

int letter_stack_get_top(struct letter_stack *stack, char *data)
{
	if (letter_stack_is_empty(stack)) {
		return -ENOMEM;
	}

	*data = stack->buffer[stack->top - 1];

	return 0;
}

// ============================================================

int double_stack_init(struct double_stack *stack, int count)
{
	stack->buffer = malloc(sizeof(stack->buffer[0]) * count);
	if (stack->buffer == NULL) {
		return -ENOMEM;
	}

	stack->top = 0;
	stack->max_count = count;

	return 0;
}

void double_stack_free(struct double_stack *stack)
{
	if (stack && stack->buffer) {
		free(stack->buffer);
	}
}

int double_stack_push(struct double_stack *stack, double data)
{
	if (double_stack_is_full(stack)) {
		return -ENOMEM;
	}

	stack->buffer[(stack->top)++] = data;

	return 0;
}

int double_stack_pop(struct double_stack *stack, double *data)
{
	if (double_stack_is_empty(stack)) {
		return -ENOMEM;
	}

	*data = stack->buffer[--(stack->top)];

	return 0;
}

int double_stack_get_top(struct double_stack *stack, double *data)
{
	if (double_stack_is_empty(stack)) {
		return -ENOMEM;
	}

	*data = stack->buffer[stack->top - 1];

	return 0;
}

// ============================================================

static int memory_stack_node_init(struct memory_stack_node *node, void *mem, size_t size)
{
	if (size) {
		node->space = malloc(size);
		if (node->space == NULL) {
			return -ENOMEM;
		}

		mem_copy(node->space, mem, size);
		node->size = size;
	} else {
		node->space = mem;
		node->size = 0;
	}

	return 0;
}

static void *memory_stack_node_get(struct memory_stack_node *node, void *mem, size_t *size)
{
	if (node->size) {
		mem_copy(mem, node->space, node->size);
	} else {
		mem = node->space;
	}

	if (size) {
		*size = node->size;
	}

	return mem;
}

static void *memory_stack_node_free(struct memory_stack_node *node, void *mem, size_t *size)
{
	if (node->size) {
		mem_copy(mem, node->space, node->size);
		free(node->space);
	} else {
		mem = node->space;
	}

	if (size) {
		*size = node->size;
	}

	return mem;
}

int memory_stack_init(struct memory_stack *stack, int count)
{
	stack->buffer = malloc(sizeof(stack->buffer[0]) * count);
	if (stack->buffer == NULL) {
		return -ENOMEM;
	}

	stack->top = 0;
	stack->max_count = count;

	return 0;
}

void memory_stack_free(struct memory_stack *stack)
{
	struct memory_stack_node *node, *node_end;

	if (stack == NULL) {
		return;
	}

	for (node = stack->buffer, node_end = node + stack->top; node < node_end; node++) {
		if (node->size) {
			free(node->space);
		}
	}

	free(stack->buffer);
}

int memory_stack_push(struct memory_stack *stack, void *mem, size_t size)
{
	int ret;

	if (memory_stack_is_full(stack)) {
		return -ENOMEM;
	}

	ret = memory_stack_node_init(stack->buffer + stack->top, mem, size);
	if (ret < 0) {
		return ret;
	}

	stack->top++;

	return 0;
}

void *memory_stack_pop(struct memory_stack *stack, void *mem, size_t *size)
{
	if (memory_stack_is_empty(stack)) {
		return NULL;
	}

	stack->top--;

	return memory_stack_node_free(stack->buffer + stack->top, mem, size);
}

void *memory_stack_get_top(struct memory_stack *stack, void *mem, size_t *size)
{
	if (memory_stack_is_empty(stack)) {
		return NULL;
	}

	return memory_stack_node_get(stack->buffer + stack->top - 1, mem, size);
}
