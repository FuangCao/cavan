#pragma once

/*
 * File:		Math.h
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

#include <math.h>
#include <cavan.h>
#include <cavan/calculator.h>
#include <cavan++/List.h>
#include <cavan++/Stack.h>

typedef enum
{
	OPERATOR_TYPE1_LEFT,
	OPERATOR_TYPE1_RIGHT,
	OPERATOR_TYPE2,
	OPERATOR_TYPE_LIST,
	OPERATOR_TYPE_CONSTANT,
} operator_type_t;

typedef enum
{
	FIELD_TYPE_NONE,
	FIELD_TYPE_OPERATOR,
	FIELD_TYPE_VALUE,
	FIELD_TYPE_BRACKET,
} field_type_t;

class Operator
{
private:
	int mType;
	int mPriority;
	int mLength;
	bool mOmitMul;
	const char *mSymbol;
	const char *mErrMsg;

public:
	virtual ~Operator(void) {}
	Operator(const char *symbol, int priority, int type, bool omit = false)
	{
		setSymbol(symbol);
		mPriority = priority;
		mType = type;
		mOmitMul = omit;
		mErrMsg = "unknown";
	}

	int getLength(void)
	{
		return mLength;
	}

	int getPriority(void)
	{
		return mPriority;
	}

	int getType(void)
	{
		return mType;
	}

	void setOmitMul(int omit)
	{
		mOmitMul = omit;
	}

	bool getOmitMul(void)
	{
		return mOmitMul;
	}

	const char *getSymbol(void)
	{
		return mSymbol;
	}

	const char *getErrMsg(void)
	{
		return mErrMsg;
	}

	void setErrMsg(const char *msg)
	{
		if (msg)
		{
			mErrMsg = msg;
		}
	}

	void setSymbol(const char *symbol);
	bool execute(Stack<double> &stackData, Stack<double> &stackResult);
	bool execute(Stack<double> &stack)
	{
		return execute(stack, stack);
	}

	virtual bool match(const char *name);
	virtual bool execute(Stack<double> &stack, double &result) = 0;

	static int compare(Operator *left, Operator *right);
};

// ================================================================================

class OperatorF1 : public Operator
{
public:
	OperatorF1(const char *symbol, int type, bool omit = false) : Operator(symbol, 0, type, omit) {}
	virtual bool execute(Stack<double> &stack, double &result);
	virtual bool execute(double &value) = 0;
};

class OperatorN1 : public OperatorF1
{
public:
	OperatorN1(const char *symbol, int type = OPERATOR_TYPE1_RIGHT) : OperatorF1(symbol, type) {}
	virtual bool execute(double &value);
	virtual bool execute(ulong &value) = 0;
};

class OperatorFactorial : public OperatorN1
{
public:
	OperatorFactorial(const char *symbol = "!") : OperatorN1(symbol, OPERATOR_TYPE1_LEFT) {}
	virtual bool execute(ulong &value);
};

class OperatorNegation : public OperatorN1
{
public:
	OperatorNegation(const char *symbol = "~") : OperatorN1(symbol) {}
	virtual bool execute(ulong &value)
	{
		value = ~value;
		return true;
	}
};

class OperatorAbs : public OperatorF1
{
public:
	OperatorAbs(const char *symbol = "abs") : OperatorF1(symbol, OPERATOR_TYPE1_RIGHT, true) {}
	virtual bool execute(double &value)
	{
		value = fabs(value);
		return true;
	}
};

class OperatorFloor : public OperatorF1
{
public:
	OperatorFloor(const char *symbol = "floor") : OperatorF1(symbol, OPERATOR_TYPE1_RIGHT, true) {}
	virtual bool execute(double &value)
	{
		value = floor(value);
		return true;
	}
};

class OperatorCeil : public OperatorF1
{
public:
	OperatorCeil(const char *symbol = "ceil") : OperatorF1(symbol, OPERATOR_TYPE1_RIGHT, true) {}
	virtual bool execute(double &value)
	{
		value = ceil(value);
		return true;
	}
};

class OperatorRound : public OperatorF1
{
public:
	OperatorRound(const char *symbol = "round") : OperatorF1(symbol, OPERATOR_TYPE1_RIGHT, true) {}
	virtual bool execute(double &value)
	{
		value = round(value);
		return true;
	}
};

class OperatorReci : public OperatorF1
{
public:
	OperatorReci(const char *symbol = "reci") : OperatorF1(symbol, OPERATOR_TYPE1_RIGHT, true) {}
	virtual bool execute(double &value)
	{
		value = 1 / value;
		return true;
	}
};

// ================================================================================

class OperatorF2 : public Operator
{
public:
	OperatorF2(const char *symbol, int priority, int type = OPERATOR_TYPE2, bool omit = false) : Operator(symbol, priority, type, omit) {}
	virtual bool execute(Stack<double> &stack, double &result);
	virtual bool execute(double left, double right, double &result) = 0;
};

class OperatorN2 : public OperatorF2
{
public:
	OperatorN2(const char *symbol, int priority) : OperatorF2(symbol, priority) {}
	virtual bool execute(double left, double right, double &result);
	virtual bool execute(ulong left, ulong right, ulong &result) = 0;
};

class OperatorAdd : public OperatorF2
{
public:
	OperatorAdd(const char *symbol = "+") : OperatorF2(symbol, 4) {}
	virtual bool execute(double left, double right, double &result)
	{
		result = left + right;
		return true;
	}
};

class OperatorSub : public OperatorF2
{
public:
	OperatorSub(const char *symbol = "-") : OperatorF2(symbol, 4) {}
	virtual bool execute(double left, double right, double &result)
	{
		result = left - right;
		return true;
	}
};

class OperatorMul : public OperatorF2
{
public:
	OperatorMul(const char *symbol = "*") : OperatorF2(symbol, 3) {}
	bool execute(Stack<double> &stack)
	{
		return Operator::execute(stack);
	}

	virtual bool execute(double left, double right, double &result)
	{
		result = left * right;
		return true;
	}
};

class OperatorDiv : public OperatorF2
{
public:
	OperatorDiv(const char *symbol = "/") : OperatorF2(symbol, 3) {}
	virtual bool execute(double left, double right, double &result);
};

class OperatorMod : public OperatorF2
{
public:
	OperatorMod(const char *symbol = "%") : OperatorF2(symbol, 3) {}
	virtual bool execute(double left, double right, double &result);
};

class OperatorGreaterThan : public OperatorF2
{
public:
	OperatorGreaterThan(const char *symbol = ">") : OperatorF2(symbol, 6) {}
	virtual bool execute(double left, double right, double &result)
	{
		result = left > right;
		return true;
	}
};

class OperatorLessThan : public OperatorF2
{
public:
	OperatorLessThan(const char *symbol = "<") : OperatorF2(symbol, 6) {}
	virtual bool execute(double left, double right, double &result)
	{
		result = left < right;
		return true;
	}
};

class OperatorEqual : public OperatorF2
{
public:
	OperatorEqual(const char *symbol = "==") : OperatorF2(symbol, 7) {}
	virtual bool execute(double left, double right, double &result)
	{
		result = left == right;
		return true;
	}
};

class OperatorNotEqual : public OperatorF2
{
public:
	OperatorNotEqual(const char *symbol = "!=") : OperatorF2(symbol, 7) {}
	virtual bool execute(double left, double right, double &result)
	{
		result = left != right;
		return true;
	}
};

class OperatorGreaterThanOrEqual : public OperatorF2
{
public:
	OperatorGreaterThanOrEqual(const char *symbol = ">=") : OperatorF2(symbol, 6) {}
	virtual bool execute(double left, double right, double &result)
	{
		result = left >= right;
		return true;
	}
};

class OperatorLessThanOrEqual : public OperatorF2
{
public:
	OperatorLessThanOrEqual(const char *symbol = "<=") : OperatorF2(symbol, 6) {}
	virtual bool execute(double left, double right, double &result)
	{
		result = left <= right;
		return true;
	}
};

class OperatorAnd : public OperatorN2
{
public:
	OperatorAnd(const char *symbol = "&") : OperatorN2(symbol, 8) {}
	virtual bool execute(ulong left, ulong right, ulong &result)
	{
		result = left & right;
		return true;
	}
};

class OperatorOr : public OperatorN2
{
public:
	OperatorOr(const char *symbol = "|") : OperatorN2(symbol, 10) {}
	virtual bool execute(ulong left, ulong right, ulong &result)
	{
		result = left | right;
		return true;
	}
};

class OperatorXor : public OperatorN2
{
public:
	OperatorXor(const char *symbol = "^") : OperatorN2(symbol, 9) {}
	virtual bool execute(ulong left, ulong right, ulong &result)
	{
		result = left ^ right;
		return true;
	}
};

class OperatorShiftL : public OperatorN2
{
public:
	OperatorShiftL(const char *symbol = "<<") : OperatorN2(symbol, 5) {}
	virtual bool execute(ulong left, ulong right, ulong &result)
	{
		result = left << right;
		return true;
	}
};

class OperatorShiftR : public OperatorN2
{
public:
	OperatorShiftR(const char *symbol = ">>") : OperatorN2(symbol, 5) {}
	virtual bool execute(ulong left, ulong right, ulong &result)
	{
		result = left >> right;
		return true;
	}
};

class OperatorPow : public OperatorF2
{
public:
	OperatorPow(const char *symbol = "pow") : OperatorF2(symbol, 0, OPERATOR_TYPE_LIST, true) {}
	virtual bool execute(double left, double right, double &result)
	{
		result = pow(left, right);
		return true;
	}
};

class OperatorSqrt : public OperatorF2
{
public:
	OperatorSqrt(const char *symbol = "sqrt") : OperatorF2(symbol, 0, OPERATOR_TYPE_LIST, true) {}
	virtual bool execute(double left, double right, double &result)
	{
		if (left == 0)
		{
			setErrMsg("Sqrt by zero");
			return false;
		}

		result = pow(right, 1 / left);
		return true;
	}
};

// ================================================================================

class OperatorAvg : public Operator
{
public:
	OperatorAvg(const char *symbol = "avg") : Operator(symbol, 0, OPERATOR_TYPE_LIST, true) {}
	virtual bool execute(Stack<double> &stack, double &result);
};

class OperatorSum : public Operator
{
public:
	OperatorSum(const char *symbol = "sum") : Operator(symbol, 0, OPERATOR_TYPE_LIST, true) {}
	virtual bool execute(Stack<double> &stack, double &result);
};

class OperatorMin : public Operator
{
public:
	OperatorMin(const char *symbol = "min") : Operator(symbol, 0, OPERATOR_TYPE_LIST, true) {}
	virtual bool execute(Stack<double> &stack, double &result);
};

class OperatorMax : public Operator
{
public:
	OperatorMax(const char *symbol = "max") : Operator(symbol, 0, OPERATOR_TYPE_LIST, true) {}
	virtual bool execute(Stack<double> &stack, double &result);
};

// ================================================================================

class OperatorE : public Operator
{
public:
	OperatorE(const char *symbol = "E") : Operator(symbol, 0, OPERATOR_TYPE_CONSTANT, true) {}
	virtual bool execute(Stack<double> &stack, double &result)
	{
		result = 2.71828182845904523536;
		return true;
	}
};

class OperatorPI : public Operator
{
public:
	OperatorPI(const char *symbol = "PI") : Operator(symbol, 0, OPERATOR_TYPE_CONSTANT, true) {}
	virtual bool execute(Stack<double> &stack, double &result)
	{
		result = 3.14159265358979323846;
		return true;
	}
};

// ================================================================================

class OperatorTrigonometricBase : public OperatorF1
{
private:
	double mPeriod;

public:
	OperatorTrigonometricBase(const char *symbol, int period) : OperatorF1(symbol, OPERATOR_TYPE1_RIGHT, true), mPeriod(period) {}
	double getPeriod(void)
	{
		return mPeriod;
	}

	double angleAdjust(double angle, double max = 180)
	{
		return angle_adjust(angle, 0, max, mPeriod);
	}

	virtual bool checkAngle(double &value)
	{
		return true;
	}

	virtual bool checkValue(double &value)
	{
		return true;
	}

	virtual bool execute(double &value)
	{
		return executeAngle(value);
	}

	virtual bool executeAngle(double &value) = 0;
	virtual bool executeRadian(double &value) = 0;
};

class OperatorTrigonometric : public OperatorTrigonometricBase
{
public:
	OperatorTrigonometric(const char *symbol, int period) : OperatorTrigonometricBase(symbol, period) {}
	virtual bool executeAngle(double &value);
	virtual bool executeRadian(double &value) = 0;
};

class OperatorTrigonometricArc : public OperatorTrigonometricBase
{
public:
	OperatorTrigonometricArc(const char *symbol, int period) : OperatorTrigonometricBase(symbol, period) {}
	virtual bool executeAngle(double &value);
	virtual bool executeRadian(double &value) = 0;
};

class OperatorSin : public OperatorTrigonometric
{
public:
	OperatorSin(const char *symbol = "sin") : OperatorTrigonometric(symbol, 360) {}
	virtual bool executeRadian(double &value)
	{
		value = sin(value);
		return true;
	}
};

class OperatorCos : public OperatorSin
{
public:
	OperatorCos(const char *symbol = "cos") : OperatorSin(symbol) {}
	virtual bool executeRadian(double &value)
	{
		value = cos(value);
		return true;
	}
};

class OperatorTan : public OperatorTrigonometric
{
public:
	OperatorTan(const char *symbol = "tan") : OperatorTrigonometric(symbol, 180) {}

	virtual bool checkAngle(double &value)
	{
		if (value == 90)
		{
			return false;
		}

		return true;
	}

	virtual bool executeRadian(double &value)
	{
		value = tan(value);
		return true;
	}
};

class OperatorCot : public OperatorTan
{
public:
	OperatorCot(const char *symbol = "cot") : OperatorTan(symbol) {}

	virtual bool checkAngle(double &value)
	{
		value = 90 - value;
		return OperatorTan::checkAngle(value);
	}
};

class OperatorAsin : public OperatorTrigonometricArc
{
public:
	OperatorAsin(const char *symbol = "asin") : OperatorTrigonometricArc(symbol, 360) {}

	virtual bool checkValue(double &value)
	{
		return value >= -1 && value <= 1;
	}

	virtual bool executeRadian(double &value)
	{
		value = asin(value);
		return true;
	}
};

class OperatorAcos : public OperatorAsin
{
public:
	OperatorAcos(const char *symbol = "acos") : OperatorAsin(symbol) {}

	virtual bool executeRadian(double &value)
	{
		value = acos(value);
		return true;
	}
};

class OperatorAtan : public OperatorTrigonometricArc
{
public:
	OperatorAtan(const char *symbol = "atan") : OperatorTrigonometricArc(symbol, 360) {}

	virtual bool executeRadian(double &value)
	{
		value = atan(value);
		return true;
	}
};

class OperatorAcot : public OperatorAtan
{
public:
	OperatorAcot(const char *symbol = "acot") : OperatorAtan(symbol) {}

	virtual bool checkAngle(double &value)
	{
		value = 90 - value;
		return true;
	}
};

// ================================================================================

class Calculator
{
private:
	static List<Operator *> sListOperator;

	Stack<double> mStackOperand;
	Stack<Operator *> mStackOperator;
	const char *mErrMsg;
	int mLastFieldType;

public:
	Calculator(void);

	const char *getErrMsg(void)
	{
		return mErrMsg;
	}

	void setErrMsg(const char *msg)
	{
		if (msg)
		{
			mErrMsg = msg;
		}
	}

	Operator *matchOperator(const char *formula);
	bool execute(const char *formula, double &result);
	bool execute(const char *formula, const char *formula_end, double &result);
	bool parseDataList(const char *formula, const char *formula_end, const char **last, Stack<double> &stack);
};
