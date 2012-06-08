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
	while (IS_FLOAT(*formula) || IS_LETTER(*formula))
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

static int binary_operation(struct double_stack *stack, char operator)
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

int simple_calculator_base(const char *formula, const char *formula_end, double *result_last)
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
			ret = simple_calculator_base(formula + 1, formula_tmp, &result_tmp);
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
				ret = binary_operation(&data_stack, operator_tmp);
				if (ret < 0)
				{
					error_msg("binary_operation");
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
		ret = binary_operation(&data_stack, operator_tmp);
		if (ret < 0)
		{
			error_msg("binary_operation");
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

int simple_calculator(const char *formula, double *result_last)
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

	ret = simple_calculator_base(formula, formula_end, result_last);
	if(ret < 0)
	{
		error_msg("__simple_calculator");
		return ret;
	}

	return 0;
}
