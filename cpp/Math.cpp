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
#include <cavan/ctype.h>
#include <cavan++/Math.h>

#define CAVAN_MATH_DEBUG	0

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

bool Operator::execute(Stack<double> &stackData, Stack<double> &stackResult)
{
	double result;

	if (!execute(stackData, result))
	{
		return false;
	}

	if (!stackResult.push(result))
	{
		setErrMsg("Data stack overfrow");
		return false;
	}

	return true;
}

// ================================================================================

bool OperatorF1::execute(Stack<double> &stack, double &result)
{
	if (!stack.pop(result))
	{
		setErrMsg("Missing operand");
		return false;
	}

	return execute(result);
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

bool OperatorF2::execute(Stack<double> &stack, double &result)
{
	double left, right;

	if (!(stack.pop(right) && stack.pop(left)))
	{
		setErrMsg("Missing operand");
		return false;
	}

	return execute(left, right, result);
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

// ================================================================================

bool OperatorAvg::execute(Stack<double> &stack, double &result)
{
	OperatorSum sum;
	int count = stack.count();

	if (!sum.execute(stack, result))
	{
		setErrMsg(sum.getErrMsg());
		return false;
	}

	result /= count;

	return true;
}

bool OperatorSum::execute(Stack<double> &stack, double &result)
{
	double value;

	for (result = 0; stack.pop(value); result += value);

	return true;
}

bool OperatorMin::execute(Stack<double> &stack, double &result)
{
	if (!stack.pop(result))
	{
		return false;
	}

	double value;

	while (stack.pop(value))
	{
		if (value < result)
		{
			result = value;
		}
	}

	return true;
}

bool OperatorMax::execute(Stack<double> &stack, double &result)
{
	if (!stack.pop(result))
	{
		return false;
	}

	double value;

	while (stack.pop(value))
	{
		if (value > result)
		{
			result = value;
		}
	}

	return true;
}

// ================================================================================

bool OperatorTrigonometric::executeAngle(double &value)
{
	value = angleAdjust(value);
	if (!checkAngle(value))
	{
		setErrMsg("Invalid trigonometric angle");
		return false;
	}

	value = angle2radian(value);

	return executeRadian(value);
}

bool OperatorTrigonometricArc::executeAngle(double &value)
{
	if (!checkValue(value))
	{
		setErrMsg("Invalid arc trigonometric value");
		return false;
	}

	if (!executeRadian(value))
	{
		return false;
	}

	value = radian2angle(value);
	if (!checkAngle(value))
	{
		setErrMsg("Invalid trigonometric angle");
		return false;
	}

	return true;
}

// ================================================================================

List<Operator *> Calculator::sListOperator(100);

Calculator::Calculator() : mStackOperand(100), mStackOperator(100), mErrMsg("unknown")
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
		sListOperator.append(new OperatorGreaterThan());
		sListOperator.append(new OperatorLessThan());
		sListOperator.append(new OperatorEqual());
		sListOperator.append(new OperatorNotEqual());
		sListOperator.append(new OperatorGreaterThanOrEqual());
		sListOperator.append(new OperatorLessThanOrEqual());
		sListOperator.append(new OperatorAnd());
		sListOperator.append(new OperatorAnd("and"));
		sListOperator.append(new OperatorOr());
		sListOperator.append(new OperatorOr("or"));
		sListOperator.append(new OperatorXor());
		sListOperator.append(new OperatorXor("xor"));
		sListOperator.append(new OperatorShiftL());
		sListOperator.append(new OperatorShiftR());
		sListOperator.append(new OperatorPow());
		sListOperator.append(new OperatorSqrt());
		sListOperator.append(new OperatorFactorial());
		sListOperator.append(new OperatorNegation());
		sListOperator.append(new OperatorNegation("neg"));
		sListOperator.append(new OperatorAbs());
		sListOperator.append(new OperatorFloor());
		sListOperator.append(new OperatorCeil());
		sListOperator.append(new OperatorRound());
		sListOperator.append(new OperatorReci());
		sListOperator.append(new OperatorAvg());
		sListOperator.append(new OperatorSum());
		sListOperator.append(new OperatorMax());
		sListOperator.append(new OperatorMin());
		sListOperator.append(new OperatorPI());
		sListOperator.append(new OperatorE());
		sListOperator.append(new OperatorSin());
		sListOperator.append(new OperatorCos());
		sListOperator.append(new OperatorTan());
		sListOperator.append(new OperatorCot());
		sListOperator.append(new OperatorAsin());
		sListOperator.append(new OperatorAcos());
		sListOperator.append(new OperatorAtan());
		sListOperator.append(new OperatorAcot());

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

#if CAVAN_MATH_DEBUG
	println("formula = %s", text_header(formula, formula_end - formula));
#endif

	mStackOperand.clear();
	mStackOperator.clear();
	mLastFieldType = FIELD_TYPE_NONE;

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

			if (op->getOmitMul() && mLastFieldType == FIELD_TYPE_VALUE && mStackOperand.hasData())
			{
				Operator *mul = matchOperator("*");
				if (mul)
				{
					mStackOperator.push(mul);
				}
			}

			switch (op->getType())
			{
			case OPERATOR_TYPE2:
			case OPERATOR_TYPE1_RIGHT:
				if (!mStackOperator.push(op))
				{
					setErrMsg("Operator stack overfrow");
					return false;
				}

				formula += op->getLength();
				break;

			case OPERATOR_TYPE1_LEFT:
			case OPERATOR_TYPE_CONSTANT:
				if (!op->execute(mStackOperand))
				{
					setErrMsg(op->getErrMsg());
					return false;
				}

				formula += op->getLength();
				break;

			case OPERATOR_TYPE_LIST:
			{
				Stack<double> stack(200);
				if (!parseDataList(formula + op->getLength(), formula_end, &formula, stack))
				{
					return false;
				}

				if (!op->execute(stack, mStackOperand))
				{
					setErrMsg(op->getErrMsg());
					return false;
				}
				break;
			}

			default:
				setErrMsg("Invalid operator");
				return false;
			}

			mLastFieldType = FIELD_TYPE_OPERATOR;
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
				if (mLastFieldType == FIELD_TYPE_OPERATOR && mStackOperator.count() == 1 && mStackOperand.isEmpty())
				{
					Operator *op;
					if (mStackOperator.top(op) && strcmp(op->getSymbol(), "-") == 0)
					{
						value = -value;
						mStackOperator.pop(op);
					}
				}

				if (!mStackOperand.push(value))
				{
					setErrMsg("Operand stack overfrow");
					return false;
				}
				mLastFieldType = FIELD_TYPE_VALUE;
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
					setErrMsg("Operand stack overfrow");
					return false;
				}

				formula = p + 1;
				mLastFieldType = FIELD_TYPE_BRACKET;
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

bool Calculator::parseDataList(const char *formula, const char *formula_end, const char **last, Stack<double> &stack)
{
	while (formula < formula_end && cavan_isspace(*formula))
	{
		formula++;
	}

	if (!cavan_isbracket_left(*formula))
	{
		setErrMsg("Need a bracket");
		return false;
	}

	formula_end = get_bracket_pair(formula, formula_end);
	if (formula_end == NULL)
	{
		setErrMsg("Bracket not pair");
		return false;
	}

	int bracket = 0;
	const char *formula_tail = ++formula;

	while (1)
	{
		if (formula_tail >= formula_end || (*formula_tail == ',' && bracket == 0))
		{
			double value;
			Calculator calculator;

			if (formula == formula_tail)
			{
				value = 0;
			}
			else if (!calculator.execute(formula, formula_tail, value))
			{
				setErrMsg(calculator.getErrMsg());
				return false;
			}

			if (!stack.push(value))
			{
				setErrMsg("Data stack overfrow");
				return false;
			}

			if (formula_tail >= formula_end)
			{
				break;
			}

			formula = ++formula_tail;
		}
		else
		{
			if (cavan_isbracket(*formula_tail))
			{
				if (cavan_isbracket_left(*formula_tail))
				{
					bracket++;
				}
				else if (bracket > 0)
				{
					bracket--;
				}
				else
				{
					setErrMsg("Bracket is not pair");
					return false;
				}
			}

			formula_tail++;
		}
	}

	*last = formula_end + 1;

	return true;
}
