// Fuang.Cao <cavan.cfa@gmail.com> Mon Jul  4 10:32:22 CST 2011

#include <cavan.h>
#include <cavan/calculator.h>
#include <cavan/stack.h>
#include <math.h>

double angle_adjust(double angle, double min_value, double max_value, double period)
{
	while (angle >= max_value)
	{
		angle -= period;
	}

	while (angle < min_value)
	{
		angle += period;
	}

	return angle;
}

char *get_bracket_pair(const char *formula, const char *formula_end)
{
	int counter;
	char left_bracket;
	char right_bracket;

	switch ((left_bracket = *formula++))
	{
	case '(':
		right_bracket = ')';
		break;
	case '[':
		right_bracket = ']';
		break;
	case '{':
		right_bracket = '}';
		break;
	default:
		pr_red_info("unknown bracket `%c'", left_bracket);
		return NULL;
	}

	for (counter = 1; formula < formula_end; formula++)
	{
		if (*formula == left_bracket)
		{
			counter++;
		}
		else if (*formula == right_bracket)
		{
			if (counter <= 1)
			{
				return (char *)formula;
			}

			counter--;
		}
	}

	return NULL;
}

int check_bracket_match_pair(const char *formula, const char *formula_end)
{
	int ret;
	struct letter_stack stack;
	char tmp;

	ret = letter_stack_init(&stack, 10);
	if (ret < 0)
	{
		error_msg("letter_stack_init");
		return ret;
	}

	while (formula < formula_end)
	{
		switch (*formula)
		{
		case '(':
		case '[':
		case '{':
			ret = letter_stack_push(&stack, *formula);
			if (ret < 0)
			{
				error_msg("");
				goto out_free_stack;
			}
			break;

		case ')':
			if (letter_stack_pop(&stack, &tmp) < 0 || tmp != '(')
			{
				error_msg("'%c' is not match", *formula);
				ret = -EFAULT;
				goto out_free_stack;
			}
			break;

		case ']':
			if (letter_stack_pop(&stack, &tmp) < 0 || tmp != '[')
			{
				error_msg("'%c' is not match", *formula);
				ret = -EFAULT;
				goto out_free_stack;
			}
			break;

		case '}':
			if (letter_stack_pop(&stack, &tmp) < 0 || tmp != '{')
			{
				error_msg("'%c' is not match", *formula);
				ret = -EFAULT;
				goto out_free_stack;
			}
			break;
		}

		formula++;
	}

	if (letter_stack_is_empty(&stack))
	{
		ret = 0;
	}
	else
	{
		error_msg("stack is not empty");
		ret = -EFAULT;
	}

out_free_stack:
	letter_stack_free(&stack);

	return ret;
}

static int get_operator_priority(char operator)
{
	switch (operator)
	{
	case '+':
	case '-':
		return 0;

	case '*':
	case '/':
	case '%':
		return 1;

	default:
		warning_msg("unknown operator '%c'", operator);
		return 0;
	}
}

static int simple_operation2(struct double_stack *stack, char operator)
{
	int ret;
	double operand1, operand2, result;

	ret = double_stack_pop(stack, &operand2);
	if (ret < 0)
	{
		error_msg("double_stack_pop");
		return ret;
	}

	ret = double_stack_pop(stack, &operand1);
	if (ret < 0)
	{
		error_msg("double_stack_pop");
		return ret;
	}

	switch (operator)
	{
	case '+':
		result = operand1 + operand2;
		break;

	case '-':
		result = operand1 - operand2;
		break;

	case '*':
		result = operand1 * operand2;
		break;

	case '/':
		result = operand1 / operand2;
		break;

	default:
		error_msg("unknown operator '%c'", operator);
		return -EINVAL;
	}

	return double_stack_push(stack, result);
}

char *double2text(double value, char *text, int size, char fill, int flags)
{
	int i;
	int base;

	text = value2text_base((s64)value, text, size, fill, flags);
	if (value == ((s64)value))
	{
		return text;
	}

	base = flags & 0xFF;
	value -= (s64)value;

	*text++ = '.';

	for (i = 0; i < 16; i++)
	{
		value *= base;
		*text++ = value2char(((s64)value) % base);
		if (value == (s64)value)
		{
			break;
		}
	}

	*text = 0;

	return text;
}

const char *text2double(const char *text, const char *text_end, double *result_last)
{
	int base, value;
	double result, weight;

	if (text >= text_end)
	{
		*result_last = 0;
		return text;
	}

	if (text[0] == '0' && text[1] != '.')
	{
		text++;

		if (text >= text_end)
		{
			*result_last = 0;
			return text;
		}

		switch (*text)
		{
		case 'o':
		case 'O':
			text++;
			base = 8;
			break;

		case 'x':
		case 'X':
			text++;
			base = 16;
			break;

		case 'b':
		case 'B':
			text++;
			base = 2;
			break;

		case 'd':
		case 'D':
			text++;
			base = 10;
			break;

		default:
			base = 8;
		}
	}
	else
	{
		base = 10;
	}

	for (result = 0; text < text_end; text++)
	{
		if (*text == '.')
		{
			text++;
			break;
		}

		value = char2value(*text);
		if (value < 0 || value >= base)
		{
			*result_last = result;
			return text;
		}

		result = result * base + value;
	}

	*result_last = result;

	for (result = 0, weight = 1.0 / base; text < text_end; text++)
	{
		value = char2value(*text);
		if (value < 0 || value >= base)
		{
			break;
		}

		result += value * weight;
		weight /= base;
	}

	*result_last += result;

	return text;
}

int simple_calculation_base(const char *formula, const char *formula_end, double *result_last)
{
	int ret;
	const char *formula_last;
	double result_tmp;
	struct double_stack data_stack;
	struct letter_stack operator_stack;
	char operator_tmp;
	int priority_tmp;

	ret = letter_stack_init(&operator_stack, 100);
	if (ret < 0)
	{
		error_msg("letter_stack_init");
		return ret;
	}

	ret = double_stack_init(&data_stack, 100);
	if (ret < 0)
	{
		error_msg("double_stack_init");
		goto out_free_operator_stack;
	}

	if (*formula == '+' || *formula == '-')
	{
		ret = double_stack_push(&data_stack, 0);
		if (ret < 0)
		{
			error_msg("double_stack_push");
			goto out_free_data_stack;
		}
	}

	while (formula < formula_end)
	{
		switch (*formula)
		{
		case '(':
		case '[':
		case '{':
			formula_last = get_bracket_pair(formula, formula_end);
			if (formula_last == NULL)
			{
				print_error("Bracket do't pair");
				ret = -EINVAL;
				goto out_free_data_stack;
			}

			ret = simple_calculation_base(formula + 1, formula_last, &result_tmp);
			if (ret < 0)
			{
				error_msg("simple_calculation_base");
				goto out_free_data_stack;
			}

			ret = double_stack_push(&data_stack, result_tmp);
			if (ret < 0)
			{
				error_msg("double_stack_push");
				goto out_free_data_stack;
			}

			formula = formula_last + 1;
			continue;

		case '0' ... '9':
			formula = text2double(formula, formula_end, &result_tmp);
			ret = double_stack_push(&data_stack, result_tmp);
			if (ret < 0)
			{
				error_msg("double_stack_push");
				goto out_free_data_stack;
			}
			continue;

		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
			priority_tmp = get_operator_priority(*formula);
			while (letter_stack_get_top(&operator_stack, &operator_tmp) >= 0 && get_operator_priority(operator_tmp) >= priority_tmp)
			{
				ret = simple_operation2(&data_stack, operator_tmp);
				if (ret < 0)
				{
					error_msg("simple_operation2");
					goto out_free_data_stack;
				}

				letter_stack_pop(&operator_stack, &operator_tmp);
			}

			ret = letter_stack_push(&operator_stack, *formula);
			if (ret < 0)
			{
				error_msg("letter_stack_push");
				goto out_free_data_stack;
			}
			break;

		case ' ':
		case '\t':
		case '\r':
		case '\n':
			break;

		case 0:
			goto label_complete_calculte;

		default:
			ret = -EINVAL;
			error_msg("unknown char '%c'", *formula);
			goto out_free_data_stack;
		}
		formula++;
	}

label_complete_calculte:
	while (letter_stack_pop(&operator_stack, &operator_tmp) >= 0)
	{
		ret = simple_operation2(&data_stack, operator_tmp);
		if (ret < 0)
		{
			error_msg("simple_operation2");
			return ret;
		}
	}

	ret = double_stack_pop(&data_stack, result_last);
	if (ret < 0)
	{
		error_msg("double_stack_pop");
		goto out_free_data_stack;
	}

	if (double_stack_is_empty(&data_stack))
	{
		ret = 0;
	}
	else
	{
		error_msg("data stack is not empty");
		ret = -EFAULT;
	}

out_free_data_stack:
	double_stack_free(&data_stack);
out_free_operator_stack:
	letter_stack_free(&operator_stack);

	return ret;
}

int simple_calculation(const char *formula, double *result_last)
{
	int ret;
	const char *formula_end = formula + text_len(formula);

	// println("formula = %s", formula);

	ret = check_bracket_match_pair(formula, formula_end);
	if (ret < 0)
	{
		error_msg("check_bracket_match_pair");
		return ret;
	}

	ret = simple_calculation_base(formula, formula_end, result_last);
	if(ret < 0)
	{
		error_msg("simple_calculator_base");
		return ret;
	}

	return 0;
}

// ============================================================

static int complete_operation2(const struct calculator_operator_descriptor *operator, struct double_stack *stack_operand)
{
	int ret;
	double left_operand;
	double right_operand;
	double result;

	ret = double_stack_pop(stack_operand, &right_operand);
	if (ret < 0)
	{
		pr_red_info("Too a few operand");
		return ret;
	}

	ret = double_stack_pop(stack_operand, &left_operand);
	if (ret < 0)
	{
		pr_red_info("Too a few operand");
		return ret;
	}

	switch (operator->type)
	{
	case OPERATOR_TYPE_ADD:
		result = left_operand + right_operand;
		break;
	case OPERATOR_TYPE_SUB:
		result = left_operand - right_operand;
		break;
	case OPERATOR_TYPE_MUL:
		result = left_operand * right_operand;
		break;
	case OPERATOR_TYPE_DIV:
		if (right_operand == 0)
		{
			pr_red_info("divide zero");
			return -EINVAL;
		}
		result = left_operand / right_operand;
		break;
	case OPERATOR_TYPE_MODE:
		if (right_operand == 0)
		{
			pr_red_info("divide zero");
			return -EINVAL;
		}
		result = left_operand / right_operand;
		result -= (s64)result;
		break;
	case OPERATOR_TYPE_AND:
		result = ((u32)left_operand) & ((u32)right_operand);
		break;
	case OPERATOR_TYPE_OR:
		result = ((u32)left_operand) | ((u32)right_operand);
		break;
	case OPERATOR_TYPE_XOR:
		result = ((u32)left_operand) ^ ((u32)right_operand);
		break;
	case OPERATOR_TYPE_LEFT:
		if (right_operand > 0)
		{
			while (right_operand > 0)
			{
				left_operand *= 2;
				right_operand--;
			}
		}
		else
		{
			while (right_operand < 0)
			{
				left_operand /= 2;
				right_operand++;
			}
		}
		result = left_operand;
		break;
	case OPERATOR_TYPE_RIGHT:
		if (right_operand > 0)
		{
			while (right_operand > 0)
			{
				left_operand /= 2;
				right_operand--;
			}
		}
		else
		{
			while (right_operand < 0)
			{
				left_operand *= 2;
				right_operand++;
			}
		}
		result = left_operand + right_operand;
		break;
	case OPERATOR_TYPE_POW:
		result = pow(left_operand, right_operand);
		break;
	case OPERATOR_TYPE_SQRT:
		if (left_operand == 0)
		{
			pr_red_info("Can't sqrt by 0");
			return -EINVAL;
		}
		result = pow(right_operand, 1 / left_operand);
		break;
	default:
		print_error("invalid operator `%s'", operator->symbols[0]);
		return -EINVAL;
	}

	return double_stack_push(stack_operand, result);
}

static int complete_operation1(const struct calculator_operator_descriptor *operator, struct double_stack *stack_operand)
{
	int ret;
	double operand;
	double result;

	ret = double_stack_pop(stack_operand, &operand);
	if (ret < 0)
	{
		pr_red_info("Too a few operand");
		return ret;
	}

	switch (operator->type)
	{
	case OPERATOR_TYPE_NEG:
		result = ~((u32)operand);
		break;
	case OPERATOR_TYPE_SIN:
		operand = angle_adjust(operand, 0, 180, 360);
		result = sin(angle2radian(operand));
		break;
	case OPERATOR_TYPE_COS:
		operand = angle_adjust(operand, 0, 180, 360);
		result = cos(angle2radian(operand));
		break;
	case OPERATOR_TYPE_TAN:
		operand = angle_adjust(operand, 0, 180, 180);
		if (operand == 90)
		{
			pr_red_info("angle = %lf", operand);
			return -EINVAL;
		}
		result = tan(angle2radian(operand));
		break;
	case OPERATOR_TYPE_COT:
		operand = angle_adjust(90 - operand, 0, 180, 180);
		if (operand == 90)
		{
			pr_red_info("angle = %lf", operand);
			return -EINVAL;
		}
		result = tan(angle2radian(operand));
		break;
	case OPERATOR_TYPE_ASIN:
		if (operand > 1 || operand < -1)
		{
			pr_red_info("asin operand = %lf", operand);
			return -EINVAL;
		}
		result = radian2angle(asin(operand));
		break;
	case OPERATOR_TYPE_ACOS:
		if (operand > 1 || operand < -1)
		{
			pr_red_info("acos operand = %lf", operand);
			return -EINVAL;
		}
		result = radian2angle(acos(operand));
		break;
	case OPERATOR_TYPE_ATAN:
		result = radian2angle(atan(operand));
		break;
	case OPERATOR_TYPE_ACOT:
		result = 90 - radian2angle(atan(operand));
		break;
	case OPERATOR_TYPE_ABS:
		result = operand < 0 ? (-operand) : operand;
		break;
	case OPERATOR_TYPE_RECI:
		if (operand == 0)
		{
			pr_red_info("Can't calculation reciprocal of 0");
			return -EINVAL;
		}
		result = 1 / operand;
		break;
	case OPERATOR_TYPE_FACT:
		for (result = 1; operand > 1; operand--)
		{
			result *= operand;
		}
		break;
	default:
		print_error("invalid operator `%s'", operator->symbols[0]);
		return -EINVAL;
	}

	return double_stack_push(stack_operand, result);
}

static const struct calculator_operator_descriptor operator_descs[] =
{
	{
		.symbols = {"+", "add", NULL},
		.type = OPERATOR_TYPE_ADD,
		.priority = OPERATOR_PRIORITY_ADD,
		.calculation = complete_operation2
	},
	{
		.symbols = {"-", "sub", NULL},
		.type = OPERATOR_TYPE_SUB,
		.priority = OPERATOR_PRIORITY_SUB,
		.calculation = complete_operation2
	},
	{
		.symbols = {"*", "x", "mul", NULL},
		.type = OPERATOR_TYPE_MUL,
		.priority = OPERATOR_PRIORITY_MUL,
		.calculation = complete_operation2
	},
	{
		.symbols = {"/", "div", NULL},
		.type = OPERATOR_TYPE_DIV,
		.priority = OPERATOR_PRIORITY_DIV,
		.calculation = complete_operation2
	},
	{
		.symbols = {"%", "mode", NULL},
		.type = OPERATOR_TYPE_MODE,
		.priority = OPERATOR_PRIORITY_MODE,
		.calculation = complete_operation2
	},
	{
		.symbols = {"&", "and", NULL},
		.type = OPERATOR_TYPE_AND,
		.priority = OPERATOR_PRIORITY_AND,
		.calculation = complete_operation2
	},
	{
		.symbols = {"|", "or", NULL},
		.type = OPERATOR_TYPE_OR,
		.priority = OPERATOR_PRIORITY_OR,
		.calculation = complete_operation2
	},
	{
		.symbols = {"~", "neg", NULL},
		.type = OPERATOR_TYPE_NEG,
		.priority = OPERATOR_PRIORITY_NEG,
		.calculation = complete_operation1
	},
	{
		.symbols = {"xor", NULL},
		.type = OPERATOR_TYPE_XOR,
		.priority = OPERATOR_PRIORITY_XOR,
		.calculation = complete_operation2
	},
	{
		.symbols = {"<<", "left", NULL},
		.type = OPERATOR_TYPE_LEFT,
		.priority = OPERATOR_PRIORITY_LEFT,
		.calculation = complete_operation2
	},
	{
		.symbols = {">>", "right", NULL},
		.type = OPERATOR_TYPE_RIGHT,
		.priority = OPERATOR_PRIORITY_RIGHT,
		.calculation = complete_operation2
	},
	{
		.symbols = {"sin", NULL},
		.type = OPERATOR_TYPE_SIN,
		.priority = OPERATOR_PRIORITY_SIN,
		.calculation = complete_operation1
	},
	{
		.symbols = {"cos", NULL},
		.type = OPERATOR_TYPE_COS,
		.priority = OPERATOR_PRIORITY_COS,
		.calculation = complete_operation1
	},
	{
		.symbols = {"tan", NULL},
		.type = OPERATOR_TYPE_TAN,
		.priority = OPERATOR_PRIORITY_TAN,
		.calculation = complete_operation1
	},
	{
		.symbols = {"cot", NULL},
		.type = OPERATOR_TYPE_COT,
		.priority = OPERATOR_PRIORITY_COT,
		.calculation = complete_operation1
	},
	{
		.symbols = {"asin", NULL},
		.type = OPERATOR_TYPE_ASIN,
		.priority = OPERATOR_PRIORITY_ASIN,
		.calculation = complete_operation1
	},
	{
		.symbols = {"acos", NULL},
		.type = OPERATOR_TYPE_ACOS,
		.priority = OPERATOR_PRIORITY_ACOS,
		.calculation = complete_operation1
	},
	{
		.symbols = {"atan", NULL},
		.type = OPERATOR_TYPE_ATAN,
		.priority = OPERATOR_PRIORITY_ATAN,
		.calculation = complete_operation1
	},
	{
		.symbols = {"acot", NULL},
		.type = OPERATOR_TYPE_ACOT,
		.priority = OPERATOR_PRIORITY_ACOT,
		.calculation = complete_operation1
	},
	{
		.symbols = {"^", "pow", NULL},
		.type = OPERATOR_TYPE_POW,
		.priority = OPERATOR_PRIORITY_POW,
		.calculation = complete_operation2
	},
	{
		.symbols = {"sqrt", NULL},
		.type = OPERATOR_TYPE_SQRT,
		.priority = OPERATOR_PRIORITY_SQRT,
		.calculation = complete_operation2
	},
	{
		.symbols = {"abs", NULL},
		.type = OPERATOR_TYPE_ABS,
		.priority = OPERATOR_PRIORITY_ABS,
		.calculation = complete_operation1
	},
	{
		.symbols = {"reci", NULL},
		.type = OPERATOR_TYPE_RECI,
		.priority = OPERATOR_PRIORITY_RECI,
		.calculation = complete_operation1
	},
	{
		.symbols = {"!", "fact", NULL},
		.type = OPERATOR_TYPE_FACT,
		.priority = OPERATOR_PRIORITY_FACT,
		.calculation = complete_operation1
	},
};

const struct calculator_operator_descriptor *get_formula_operator(const char *formula, const char **formula_last)
{
	const struct calculator_operator_descriptor *p;

	for (p = operator_descs + NELEM(operator_descs) - 1; p >= operator_descs; p--)
	{
		int i;

		for (i = 0; p->symbols[i]; i++)
		{
			if (text_lhcmp(p->symbols[i], formula) == 0)
			{
				*formula_last = formula + text_len(p->symbols[i]);
				return p;
			}
		}
	}

	return NULL;
}

int complete_calculation_base(const char *formula, const char *formula_end, double *result_last)
{
	int ret;
	const char *formula_last;
	const struct calculator_operator_descriptor *operator, *operator_top;
	struct general_stack stack_operator;
	struct double_stack stack_operand;

	ret = general_stack_init_fd(&stack_operator, 10);
	if (ret < 0)
	{
		print_error("general_stack_init_fd");
		return ret;
	}

	ret = double_stack_init(&stack_operand, 100);
	if (ret < 0)
	{
		print_error("double_stack_init");
		goto out_operator_stack_free;
	}

	if (*formula == '+' || *formula == '-')
	{
		double_stack_push(&stack_operand, 0);
	}

	while (formula < formula_end)
	{
		switch (*formula)
		{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			break;

		case '(':
		case '[':
		case '{':
			formula_last = get_bracket_pair(formula, formula_end);
			if (formula_last == NULL)
			{
				print_error("Bracket do't pair");
				ret = -EINVAL;
				goto out_operand_stack_free;
			}

			ret = complete_calculation_base(formula + 1, formula_last, result_last);
			if (ret < 0)
			{
				goto out_operand_stack_free;
			}

			ret = double_stack_push(&stack_operand, *result_last);
			if (ret < 0)
			{
				pr_red_info("Operand stack overflow");
				goto out_operand_stack_free;
			}

			formula = formula_last + 1;
			continue;

		case '0' ... '9':
			formula = text2double(formula, formula_end, result_last);
			ret = double_stack_push(&stack_operand, *result_last);
			if (ret < 0)
			{
				pr_red_info("Operand stack overflow");
				goto out_operand_stack_free;
			}
			continue;

		default:
			operator = get_formula_operator(formula, &formula_last);
			if (operator == NULL)
			{
				pr_red_info("invalid formula `%s'", formula);
				ret = -EINVAL;
				goto out_operand_stack_free;
			}

			while (1)
			{
				operator_top = general_stack_get_top_fd(&stack_operator);
				if (operator_top == NULL || operator_top->priority < operator->priority)
				{
					break;
				}

				general_stack_pop_fd(&stack_operator);

				ret = operator_top->calculation(operator_top, &stack_operand);
				if (ret < 0)
				{
					goto out_operand_stack_free;
				}
			}

			ret = general_stack_push_fd(&stack_operator, (void *)operator);
			if (ret < 0)
			{
				pr_red_info("Operator stack overflow");
				goto out_operand_stack_free;
			}
			formula = formula_last;
			continue;
		}
		formula++;
	}

	while (1)
	{
		operator_top = general_stack_pop_fd(&stack_operator);
		if (operator_top == NULL)
		{
			break;
		}

		ret = operator_top->calculation(operator_top, &stack_operand);
		if (ret < 0)
		{
			goto out_operand_stack_free;
		}
	}

	ret = double_stack_pop(&stack_operand, result_last);
	if (ret < 0)
	{
		print_error("Too a few operand");
		goto out_operand_stack_free;
	}

	if (double_stack_is_empty(&stack_operand))
	{
		ret = 0;
	}
	else
	{
		pr_red_info("Too much operand");
		ret = -EINVAL;
	}

out_operand_stack_free:
	double_stack_free(&stack_operand);
out_operator_stack_free:
	general_stack_free(&stack_operator);

	return ret;
}

int complete_calculation(const char *formula, double *result_last)
{
	int ret;
	const char *formula_end = formula + text_len(formula);

	// println("formula = %s", formula);

	ret = check_bracket_match_pair(formula, formula_end);
	if (ret < 0)
	{
		error_msg("check_bracket_match_pair");
		return ret;
	}

	return complete_calculation_base(formula, formula_end, result_last);
}
