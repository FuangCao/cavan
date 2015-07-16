/*
 * File:		Math.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-07-15 10:32:36
 *
 * Copyright (c) 2015 Fuang.Cao <cavan.cfa@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <cavan.h>
#include <cavan/calculator.h>
#include <cavan++/Math.h>
#include <math.h>

#define CAVAN_MATH_DEBUG	1

void Operator::setSymbol(const char *symbol)
{
	mSymbol = symbol;
	mLength = strlen(symbol);
}

bool Operator::match(const char *name)
{
	return strncmp(name, mSymbol, mLength) == 0;
}

int Operator::compare(Operator *left, Operator *right)
{
	if (left->mLength == right->mLength)
	{
		return strcmp(left->mSymbol, right->mSymbol);
	}

	return right->mLength - left->mLength;
}

// ================================================================================

bool OperatorF1::execute(Stack<double> &stack)
{
	double value;

	if (!stack.pop(value))
	{
		setErrMsg("Missing operand");
		return false;
	}

	if (!execute(value))
	{
		return false;
	}

	if (!stack.push(value))
	{
		setErrMsg("Operand stack full");
		return false;
	}

	return true;
}

bool OperatorN1::execute(double &value)
{
	ulong nValue = value;
	if (nValue != value)
	{
		setErrMsg("Need a integer");
		return false;
	}

	bool res = execute(nValue);
	if (res)
	{
		value = nValue;
	}

	return res;
}

bool OperatorFactorial::execute(ulong &value)
{
	ulong step;

	for (step = value, value = 1; step > 1; step--)
	{
		value *= step;
	}

	return true;
}

// ================================================================================

bool OperatorF2::execute(Stack<double> &stack)
{
	double left, right, result;

	if (!(stack.pop(right) && stack.pop(left)))
	{
		setErrMsg("Missing operand");
		return false;
	}

	if (!execute(left, right, result))
	{
		return false;
	}

#if CAVAN_MATH_DEBUG
	println("%lf %s %lf = %lf", left, getSymbol(), right, result);
#endif

	if (!stack.push(result))
	{
		setErrMsg("Operand stack full");
		return false;
	}

	return true;
}

bool OperatorN2::execute(double left, double right, double &result)
{
	ulong nResult;
	ulong nLeft = left;
	ulong nRight = right;

	if (nLeft != left || nRight != right)
	{
		setErrMsg("Need a integer");
		return false;
	}

	bool res = execute(nLeft, nRight, nResult);
	if (res)
	{
		result = nResult;
	}

	return res;
}

bool OperatorAdd::execute(double left, double right, double &result)
{
	result = left + right;
	return true;
}

bool OperatorSub::execute(double left, double right, double &result)
{
	result = left - right;
	return true;
}

bool OperatorMul::execute(double left, double right, double &result)
{
	result = left * right;
	return true;
}

bool OperatorDiv::execute(double left, double right, double &result)
{
	if (right == 0)
	{
		setErrMsg("Divide by zero");
		return false;
	}

	result = left / right;
	return true;
}

bool OperatorMod::execute(double left, double right, double &result)
{
	if (right == 0)
	{
		setErrMsg("Divide by zero");
		return false;
	}

	result = fmod(left, right);
	return true;
}

bool OperatorAnd::execute(ulong left, ulong right, ulong &result)
{
	result = left & right;
	return true;
}

bool OperatorOr::execute(ulong left, ulong right, ulong &result)
{
	result = left | right;
	return true;
}

bool OperatorXor::execute(ulong left, ulong right, ulong &result)
{
	result = left ^ right;
	return true;
}

// ================================================================================

List<Operator *> Calculator::sListOperator(100);

Calculator::Calculator() : mStackOperand(100), mStackOperator(100)
{
	if (sListOperator.isEmpty())
	{
		sListOperator.append(new OperatorAdd());
		sListOperator.append(new OperatorAdd("add"));
		sListOperator.append(new OperatorSub());
		sListOperator.append(new OperatorSub("sub"));
		sListOperator.append(new OperatorMul());
		sListOperator.append(new OperatorMul("mul"));
		sListOperator.append(new OperatorMul("x"));
		sListOperator.append(new OperatorDiv());
		sListOperator.append(new OperatorDiv("div"));
		sListOperator.append(new OperatorMod());
		sListOperator.append(new OperatorMod("mod"));
		sListOperator.append(new OperatorAnd());
		sListOperator.append(new OperatorAnd("and"));
		sListOperator.append(new OperatorOr());
		sListOperator.append(new OperatorOr("or"));
		sListOperator.append(new OperatorXor());
		sListOperator.append(new OperatorXor("xor"));
		sListOperator.append(new OperatorFactorial());
		sListOperator.append(new OperatorNegation());
		sListOperator.append(new OperatorNegation("neg"));

		sListOperator.sort(Operator::compare);
	}
}

Operator *Calculator::matchOperator(const char *formula)
{
	Operator **p;

	sListOperator.start();
	while ((p = sListOperator.next()))
	{
		if ((*p)->match(formula))
		{
			return *p;
		}
	}

	return NULL;
}

bool Calculator::execute(const char *formula, const char *formula_end, double &result)
{
	double value;

	mStackOperand.clear();
	mStackOperator.clear();

	while (formula < formula_end)
	{
		Operator *op = matchOperator(formula);
		if (op)
		{
			Operator *top;
			if (mStackOperator.top(top))
			{
				if (top->getPriority() <= op->getPriority())
				{
					if (!top->execute(mStackOperand))
					{
						setErrMsg(top->getErrMsg());
						return false;
					}

					mStackOperator.pop(top);
				}
			}

			switch (op->getType())
			{
			case OPERATOR_TYPE2:
				if (mStackOperand.isEmpty())
				{
					if (!mStackOperand.push(0))
					{
						setErrMsg("Operand stack full");
						return false;
					}
				}

				if (!mStackOperator.push(op))
				{
					setErrMsg("Operator stack full");
					return false;
				}
				break;

			case OPERATOR_TYPE1_RIGHT:
				if (!mStackOperator.push(op))
				{
					setErrMsg("Operator stack full");
					return false;
				}
				break;

			case OPERATOR_TYPE1_LEFT:
				if (!op->execute(mStackOperand))
				{
					setErrMsg(op->getErrMsg());
					return false;
				}
				break;

			default:
				setErrMsg("Invalid operator");
				return false;
			}

			formula += op->getLength();
		}
		else
		{
			switch (*formula)
			{
			case ' ':
			case ',':
			case '\r':
			case '\n':
			case '\t':
			case '\f':
				formula++;
				break;

			case '0' ... '9':
				value = text2double_unsigned(formula, formula_end, &formula, 10);
				if (!mStackOperand.push(value))
				{
					setErrMsg("Operand stack full");
					return false;
				}
				break;

			case '(':
			case '[':
			case '{':
			{
				const char *p = get_bracket_pair(formula, formula_end);
				if (p == NULL)
				{
					setErrMsg("No matching brackets");
					return false;
				}

				Calculator calculator;
				if (!calculator.execute(formula + 1, p, value))
				{
					setErrMsg(calculator.getErrMsg());
					return false;
				}

				if (!mStackOperand.push(value))
				{
					setErrMsg("Operand stack full");
					return false;
				}

				formula = p + 1;
				break;
			}

			default:
				setErrMsg("Invalid symbol");
				return false;
			}
		}
	}

	while (1)
	{
		Operator *op;
		if (!mStackOperator.pop(op))
		{
			break;
		}

		if (!op->execute(mStackOperand))
		{
			setErrMsg(op->getErrMsg());
			return false;
		}
	}

	if (!mStackOperand.pop(result))
	{
		setErrMsg("Missing operand");
		return false;
	}

	if (mStackOperand.hasData())
	{
		setErrMsg("Too much operand");
		return false;
	}

	return true;
}

bool Calculator::execute(const char *formula, double &result)
{
	const char *formula_end = formula + strlen(formula);

	if (check_bracket_match_pair(formula, formula_end) < 0)
	{
		setErrMsg("No matching brackets");
		return false;
	}

	return execute(formula, formula_end, result);
}
