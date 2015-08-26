#pragma once

#include <cavan.h>
#include <cavan/stack.h>

// Fuang.Cao <cavan.cfa@gmail.com> Mon Jul  4 10:32:22 CST 2011

__BEGIN_DECLS

#define OPERAND_MAX_SYMBOL_COUNT	4
#define ABS_VALUE(a)				((a) < 0 ? -(a) : (a))

#ifndef PI
#define PI 3.14159265358979323846
#endif

#define BUILD_MASK_LL(size, offset) \
	(((1LL << (size)) - 1) << (offset))

#define GET_PART_VALUE_LL(value, size, offset) \
	(((value) >> (offset)) & ((1LL << (size)) - 1))

#define DOUBLE_SIGN_OFFSET		63
#define DOUBLE_SIGN_SIZE		1
#define DOUBLE_SIGN_MASK \
	BUILD_MASK_LL(DOUBLE_SIGN_SIZE, DOUBLE_SIGN_OFFSET)
#define DOUBLE_SIGN_VALUE(value) \
	GET_PART_VALUE_LL(value, DOUBLE_SIGN_SIZE, DOUBLE_SIGN_OFFSET)

#define DOUBLE_EXPONENT_OFFSET	52
#define DOUBLE_EXPONENT_SIZE	11
#define DOUBLE_EXPONENT_MASK \
	BUILD_MASK_LL(DOUBLE_EXPONENT_SIZE, DOUBLE_EXPONENT_OFFSET)
#define DOUBLE_EXPONENT_VALUE(value) \
	GET_PART_VALUE_LL(value, DOUBLE_EXPONENT_SIZE, DOUBLE_EXPONENT_OFFSET)

#define DOUBLE_CONTENT_OFFSET	0
#define DOUBLE_CONTENT_SIZE		52
#define DOUBLE_CONTENT_MASK \
	BUILD_MASK_LL(DOUBLE_CONTENT_SIZE, DOUBLE_CONTENT_OFFSET)
#define DOUBLE_CONTENT_VALUE(value) \
	GET_PART_VALUE_LL(value, DOUBLE_CONTENT_SIZE, DOUBLE_CONTENT_OFFSET)

enum calculator_operator_identify
{
	OPERATOR_ADD_ID,
	OPERATOR_SUB_ID,
	OPERATOR_MUL_ID,
	OPERATOR_DIV_ID,
	OPERATOR_MOD_ID,
	OPERATOR_AND_ID,
	OPERATOR_OR_ID,
	OPERATOR_XOR_ID,
	OPERATOR_NEG_ID,
	OPERATOR_LEFT_ID,
	OPERATOR_RIGHT_ID,
	OPERATOR_SIN_ID,
	OPERATOR_COS_ID,
	OPERATOR_TAN_ID,
	OPERATOR_COT_ID,
	OPERATOR_ASIN_ID,
	OPERATOR_ACOS_ID,
	OPERATOR_ATAN_ID,
	OPERATOR_ACOT_ID,
	OPERATOR_POW_ID,
	OPERATOR_SQRT_ID,
	OPERATOR_ABS_ID,
	OPERATOR_RECI_ID,
	OPERATOR_FACT_ID,
	OPERATOR_BASE_ID,
	OPERATOR_PI_ID,
};

#define OPERATOR_PRIORITY_ADD	11
#define OPERATOR_PRIORITY_SUB	11
#define OPERATOR_PRIORITY_MUL	12
#define OPERATOR_PRIORITY_DIV	12
#define OPERATOR_PRIORITY_MOD	12
#define OPERATOR_PRIORITY_AND	7
#define OPERATOR_PRIORITY_OR	5
#define OPERATOR_PRIORITY_XOR	6
#define OPERATOR_PRIORITY_NEG	14
#define OPERATOR_PRIORITY_LEFT	10
#define OPERATOR_PRIORITY_RIGHT	10
#define OPERATOR_PRIORITY_SIN	15
#define OPERATOR_PRIORITY_COS	15
#define OPERATOR_PRIORITY_TAN	15
#define OPERATOR_PRIORITY_COT	15
#define OPERATOR_PRIORITY_ASIN	15
#define OPERATOR_PRIORITY_ACOS	15
#define OPERATOR_PRIORITY_ATAN	15
#define OPERATOR_PRIORITY_ACOT	15
#define OPERATOR_PRIORITY_ABS	15
#define OPERATOR_PRIORITY_RECI	15
#define OPERATOR_PRIORITY_FACT	15
#define OPERATOR_PRIORITY_POW	13
#define OPERATOR_PRIORITY_SQRT	13
#define OPERATOR_PRIORITY_BASE	16
#define OPERATOR_PRIORITY_PI	16

struct calculator_operator_descriptor
{
	const char *symbols[OPERAND_MAX_SYMBOL_COUNT];
	enum calculator_operator_identify id;
	int priority;
	bool need_push;
	int (*calculation)(const struct calculator_operator_descriptor *, struct double_stack *);
};

u64 double_integer_part_value_base(u64 value);
u64 double_fractional_part_value_base(u64 value);
char *double2text_base(u64 value, char *text, int size, char fill, int flags);

double angle_adjust(double angle, double min_value, double max_value, double period);

char *get_bracket_pair(const char *formula, const char *formula_end);
int check_bracket_match_pair(const char *formula, const char *formula_end);
int simple_calculation_base(const char *formula, const char *formula_end, double *result_last);
int simple_calculation(const char *formula, double *result_last);

const struct calculator_operator_descriptor *get_formula_operator(const char *formula, const char **formula_last);
int complete_calculation_base(const char *formula, const char *formula_end, double *result_last);
int complete_calculation(const char *formula, double *result_last);

static inline double angle2radian(double angle)
{
	return angle * PI / 180;
}

static inline double radian2angle(double radian)
{
	return radian * 180 / PI;
}

static inline int double_exponent_part_value_base(u64 value)
{
	return DOUBLE_EXPONENT_VALUE(value) - 1023;
}

static inline int double_exponent_part_value(double *value)
{
	return double_exponent_part_value_base(*(u64 *) value);
}

static inline u64 double_integer_part_value(double *value)
{
	return double_integer_part_value_base(*(u64 *) value);
}

static inline u64 double_fractional_part_value(double *value)
{
	return double_fractional_part_value_base(*(u64 *) value);
}

static inline char *double2text(double *value, char *text, int size, char fill, int flags)
{
	return double2text_base(*(u64 *) value, text, size, fill, flags);
}

__END_DECLS
