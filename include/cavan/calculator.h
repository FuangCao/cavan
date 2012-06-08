#pragma once

#include <cavan/stack.h>

// Fuang.Cao <cavan.cfa@gmail.com> Mon Jul  4 10:32:22 CST 2011

#define OPERAND_MAX_SYMBOL_COUNT	8
#define ABS_VALUE(a)				((a) < 0 ? -(a) : (a))

enum calculator_operator_type
{
	OPERATOR_TYPE_ADD,
	OPERATOR_TYPE_SUB,
	OPERATOR_TYPE_MUL,
	OPERATOR_TYPE_DIV,
	OPERATOR_TYPE_MODE,
	OPERATOR_TYPE_AND,
	OPERATOR_TYPE_OR,
	OPERATOR_TYPE_XOR,
	OPERATOR_TYPE_NEG,
	OPERATOR_TYPE_LEFT,
	OPERATOR_TYPE_RIGHT
};

struct calculator_operator_descriptor
{
	const char *symbols[OPERAND_MAX_SYMBOL_COUNT];
	enum calculator_operator_type type;
	int priority;
	int (*calculation)(const struct calculator_operator_descriptor *operator, struct double_stack *stack_operand);
};

char *get_bracket_pair(const char *formula, const char *formula_end);
int check_bracket_match_pair(const char *formula, const char *formula_end);
int simple_calculation_base(const char *formula, const char *formula_end, double *result_last);
int simple_calculation(const char *formula, double *result_last);
int text2double(const char *text, const char *text_end, double *result_last);

const struct calculator_operator_descriptor *get_formula_operator(const char *formula, const char **formula_last);
int complete_calculation_base(const char *formula, const char *formula_end, double *result_last);
int complete_calculation(const char *formula, double *result_last);
