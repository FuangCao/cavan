// Fuang.Cao <cavan.cfa@gmail.com> Mon Jul  4 10:32:22 CST 2011

#include <cavan.h>
#include <cavan/calculator.h>
#include <cavan/stack.h>

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

static char *get_float_letters(const char *formula)
{
	if (formula[0] == '0' && text_has_char("BbXxOoDd", formula[1]))
	{
		formula += 2;
	}

	while (IS_FLOAT(*formula) || IS_LETTER_AF(*formula))
	{
		formula++;
	}

	return (char *)formula;
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

int text2double(const char *text, const char *text_end, double *result_last)
{
	int base, value;
	double result, weight;

	if (text >= text_end)
	{
		*result_last = 0;
		return 0;
	}

	if (text[0] == '0' && text[1] != '.')
	{
		text++;

		if (text >= text_end)
		{
			*result_last = 0;
			return 0;
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

	for (result = 0; text < text_end && *text != '.'; text++)
	{
		value = char2value(*text);
		if (value < 0 || value >= base)
		{
			error_msg("char2value");
			return -EINVAL;
		}

		result = result * base + value;
	}

	*result_last = result;
	text++;

	for (result = 0, weight = 1.0 / base; text < text_end; text++)
	{
		value = char2value(*text);
		if (value < 0 || value >= base)
		{
			error_msg("invalid charector '%c`", *text);
			return -EINVAL;
		}

		result += value * weight;
		weight /= base;
	}

	*result_last += result;

	return 0;
}

int simple_calculation_base(const char *formula, const char *formula_end, double *result_last)
{
	int ret;
	const char *formula_tmp;
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
			formula_tmp = get_bracket_pair(formula, formula_end);
			ret = simple_calculation_base(formula + 1, formula_tmp, &result_tmp);
			if (ret < 0)
			{
				error_msg("__simple_calculator");
				goto out_free_data_stack;
			}

			ret = double_stack_push(&data_stack, result_tmp);
			if (ret < 0)
			{
				error_msg("double_stack_push");
				goto out_free_data_stack;
			}

			formula = formula_tmp + 1;
			continue;

		case '0' ... '9':
			formula_tmp = get_float_letters(formula);
			ret = text2double(formula, formula_tmp, &result_tmp);
			if (ret < 0)
			{
				error_msg("text2double");
				goto out_free_data_stack;
			}

			ret = double_stack_push(&data_stack, result_tmp);
			if (ret < 0)
			{
				error_msg("double_stack_push");
				goto out_free_data_stack;
			}

			formula = formula_tmp;
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

	println("formula = %s", formula);

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
		print_error("double_stack_pop");
		return ret;
	}

	ret = double_stack_pop(stack_operand, &left_operand);
	if (ret < 0)
	{
		switch (operator->type)
		{
		case OPERATOR_TYPE_ADD:
			result = right_operand;
			goto out_double_stack_push;
		case OPERATOR_TYPE_SUB:
			result = -right_operand;
			goto out_double_stack_push;
			break;
		default:
			print_error("double_stack_pop");
			return ret;
		}
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
		result = ((u64)left_operand) & ((u64)right_operand);
		break;
	case OPERATOR_TYPE_OR:
		result = ((u64)left_operand) | ((u64)right_operand);
		break;
	case OPERATOR_TYPE_XOR:
		result = ((u64)left_operand) ^ ((u64)right_operand);
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
	default:
		print_error("invalid operator `%s'", operator->symbols[0]);
		return -EINVAL;
	}

out_double_stack_push:
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
		print_error("double_stack_pop");
		return ret;
	}

	switch (operator->type)
	{
	case OPERATOR_TYPE_NEG:
		result = ~(u64)operand;
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
		.priority = 4,
		.calculation = complete_operation2
	},
	{
		.symbols = {"-", "sub", NULL},
		.type = OPERATOR_TYPE_SUB,
		.priority = 4,
		.calculation = complete_operation2
	},
	{
		.symbols = {"*", "x", "mul", NULL},
		.type = OPERATOR_TYPE_MUL,
		.priority = 3,
		.calculation = complete_operation2
	},
	{
		.symbols = {"/", "div", NULL},
		.type = OPERATOR_TYPE_DIV,
		.priority = 3,
		.calculation = complete_operation2
	},
	{
		.symbols = {"%", "mode", NULL},
		.type = OPERATOR_TYPE_MODE,
		.priority = 3,
		.calculation = complete_operation2
	},
	{
		.symbols = {"&", "and", NULL},
		.type = OPERATOR_TYPE_AND,
		.priority = 8,
		.calculation = complete_operation2
	},
	{
		.symbols = {"|", "or", NULL},
		.type = OPERATOR_TYPE_OR,
		.priority = 10,
		.calculation = complete_operation2
	},
	{
		.symbols = {"~", "neg", NULL},
		.type = OPERATOR_TYPE_NEG,
		.priority = 2,
		.calculation = complete_operation1
	},
	{
		.symbols = {"^", "xor", NULL},
		.type = OPERATOR_TYPE_XOR,
		.priority = 9,
		.calculation = complete_operation2
	},
	{
		.symbols = {"<<", "left", NULL},
		.type = OPERATOR_TYPE_LEFT,
		.priority = 5,
		.calculation = complete_operation2
	},
	{
		.symbols = {">>", "right", NULL},
		.type = OPERATOR_TYPE_RIGHT,
		.priority = 5,
		.calculation = complete_operation2
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
				print_error("get_bracket_pair");
				ret = -EINVAL;
				goto out_operand_stack_free;
			}

			ret = complete_calculation_base(formula + 1, formula_last, result_last);
			if (ret < 0)
			{
				print_error("complete_calculation_base");
				goto out_operand_stack_free;
			}

			ret = double_stack_push(&stack_operand, *result_last);
			if (ret < 0)
			{
				print_error("double_stack_push");
				goto out_operand_stack_free;
			}

			formula = formula_last + 1;
			continue;

		case '0' ... '9':
			formula_last = get_float_letters(formula);
			ret = text2double(formula, formula_last, result_last);
			if (ret < 0)
			{
				print_error("text2double");
				goto out_operand_stack_free;
			}

			ret = double_stack_push(&stack_operand, *result_last);
			if (ret < 0)
			{
				print_error("double_stack_push");
				goto out_operand_stack_free;
			}

			formula = formula_last;
			continue;

		default:
			operator = get_formula_operator(formula, &formula_last);
			if (operator == NULL)
			{
				print_error("get_formula_operator, formula = %s", formula);
				ret = -EINVAL;
				goto out_operand_stack_free;
			}

			while (1)
			{
				operator_top = general_stack_get_top_fd(&stack_operator);
				if (operator_top == NULL || operator_top->priority > operator->priority)
				{
					break;
				}

				general_stack_pop_fd(&stack_operator);

				ret = operator_top->calculation(operator_top, &stack_operand);
				if (ret < 0)
				{
					print_error("operator_top->calculation");
					goto out_operand_stack_free;
				}
			}

			ret = general_stack_push_fd(&stack_operator, (void *)operator);
			if (ret < 0)
			{
				print_error("general_stack_push_fd");
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
			print_error("operator_top->calculation");
			goto out_operand_stack_free;
		}
	}

	ret = double_stack_pop(&stack_operand, result_last);
	if (ret < 0)
	{
		print_error("double_stack_pop");
		goto out_operand_stack_free;
	}

	if (double_stack_is_empty(&stack_operand))
	{
		ret = 0;
	}
	else
	{
		print_error("operand stack is not empty");
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

	println("formula = %s", formula);

	ret = check_bracket_match_pair(formula, formula_end);
	if (ret < 0)
	{
		error_msg("check_bracket_match_pair");
		return ret;
	}

	return complete_calculation_base(formula, formula_end, result_last);
}
