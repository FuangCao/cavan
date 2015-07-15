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
#include <cavan++/Math.h>
#include <math.h>

#define CAVAN_MATH_DEBUG	1

void Operator::setSymbol(const char *symbol)
{
	mSymbol = symbol;
	mSymLen = strlen(symbol);
}

bool Operator::match(const char *name)
{
	return strncmp(name, mSymbol, mSymLen) == 0;
}

int Operator::compare(Operator *left, Operator *right)
{
	if (left->mSymLen == right->mSymLen)
	{
		return strcmp(left->mSymbol, right->mSymbol);
	}

	return right->mSymLen - left->mSymLen;
}

// ================================================================================

bool UnaryOperator::execute(Stack<double> &stack)
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

	return stack.push(value);
}

// ================================================================================

bool BinaryOperator::execute(Stack<double> &stack)
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

	return stack.push(result);
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

bool OperatorAnd::execute(double left, double right, double &result)
{
	result = ((u64) left) & ((u64) right);
	return true;
}

bool OperatorOr::execute(double left, double right, double &result)
{
	result = ((u64) left) | ((u64) right);
	return true;
}

bool OperatorXor::execute(double left, double right, double &result)
{
	result = ((u64) left) ^ ((u64) right);
	return true;
}

// ================================================================================

Calculator::Calculator() : mStackOperand(100), mStackOperator(100), mListOperator(100)
{
	mListOperator.append(new OperatorAdd());
	mListOperator.append(new OperatorAdd("add"));
	mListOperator.append(new OperatorSub());
	mListOperator.append(new OperatorSub("sub"));
	mListOperator.append(new OperatorMul());
	mListOperator.append(new OperatorMul("mul"));
	mListOperator.append(new OperatorMul("x"));
	mListOperator.append(new OperatorDiv());
	mListOperator.append(new OperatorDiv("div"));
	mListOperator.append(new OperatorMod());
	mListOperator.append(new OperatorMod("mod"));
	mListOperator.append(new OperatorAnd());
	mListOperator.append(new OperatorAnd("and"));
	mListOperator.append(new OperatorOr());
	mListOperator.append(new OperatorOr("or"));
	mListOperator.append(new OperatorXor());
	mListOperator.append(new OperatorXor("xor"));

	mListOperator.sort(Operator::compare);
}

Operator *Calculator::matchOperator(const char *formula)
{
	Operator **p;

	mListOperator.start();
	while ((p = mListOperator.next()))
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

			if (mStackOperand.isEmpty())
			{
				mStackOperand.push(0);
			}

			mStackOperator.push(op);

			formula += op->getSymLen();
		}
		else
		{
			switch (*formula)
			{
			case ' ':
			case '\r':
			case '\n':
			case '\t':
			case '\f':
				formula++;
				break;

			case '0' ... '9':
				value = text2double_unsigned(formula, formula_end, &formula, 10);
				mStackOperand.push(value);
				break;

			case '(':
			case '[':
			case '{':
				break;

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
	return execute(formula, formula + strlen(formula), result);
}
