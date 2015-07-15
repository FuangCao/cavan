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

#include <cavan.h>
#include <cavan++/List.h>
#include <cavan++/Stack.h>

class Operator
{
private:
	int mPriority;
	int mSymLen;
	const char *mSymbol;
	const char *mErrMsg;

public:
	Operator(const char *symbol, int priority)
	{
		setSymbol(symbol);
		mPriority = priority;
	}

	int getSymLen(void)
	{
		return mSymLen;
	}

	int getPriority(void)
	{
		return mPriority;
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
		mErrMsg = msg;
	}

	void setSymbol(const char *symbol);
	virtual bool match(const char *name);
	virtual bool execute(Stack<double> &stack) = 0;

	static int compare(Operator *left, Operator *right);
};

// ================================================================================

class UnaryOperator : public Operator
{
public:
	UnaryOperator(const char *symbol, int priority) : Operator(symbol, priority) {}
	virtual bool execute(Stack<double> &stack);
	virtual bool execute(double &value) = 0;
};

// ================================================================================

class BinaryOperator : public Operator
{
public:
	BinaryOperator(const char *symbol, int priority) : Operator(symbol, priority) {}
	virtual bool execute(Stack<double> &stack);
	virtual bool execute(double left, double right, double &result) = 0;
};

class OperatorAdd : public BinaryOperator
{
public:
	OperatorAdd(const char *symbol = "+") : BinaryOperator(symbol, 4) {}
	virtual bool execute(double left, double right, double &result);
};

class OperatorSub : public BinaryOperator
{
public:
	OperatorSub(const char *symbol = "-") : BinaryOperator(symbol, 4) {}
	virtual bool execute(double left, double right, double &result);
};

class OperatorMul : public BinaryOperator
{
public:
	OperatorMul(const char *symbol = "*") : BinaryOperator(symbol, 3) {}
	virtual bool execute(double left, double right, double &result);
};

class OperatorDiv : public BinaryOperator
{
public:
	OperatorDiv(const char *symbol = "/") : BinaryOperator(symbol, 3) {}
	virtual bool execute(double left, double right, double &result);
};

class OperatorMod : public BinaryOperator
{
public:
	OperatorMod(const char *symbol = "%") : BinaryOperator(symbol, 3) {}
	virtual bool execute(double left, double right, double &result);
};

class OperatorAnd : public BinaryOperator
{
public:
	OperatorAnd(const char *symbol = "&") : BinaryOperator(symbol, 8) {}
	virtual bool execute(double left, double right, double &result);
};

class OperatorOr : public BinaryOperator
{
public:
	OperatorOr(const char *symbol = "|") : BinaryOperator(symbol, 10) {}
	virtual bool execute(double left, double right, double &result);
};

class OperatorXor : public BinaryOperator
{
public:
	OperatorXor(const char *symbol = "^") : BinaryOperator(symbol, 9) {}
	virtual bool execute(double left, double right, double &result);
};

class Calculator
{
private:
	static List<Operator *> sListOperator;

	Stack<double> mStackOperand;
	Stack<Operator *> mStackOperator;
	const char *mErrMsg;

public:
	Calculator(void);

	const char *getErrMsg(void)
	{
		return mErrMsg;
	}

	void setErrMsg(const char *msg)
	{
		mErrMsg = msg;
	}

	Operator *matchOperator(const char *formula);
	bool execute(const char *formula, double &result);
	bool execute(const char *start, const char *end, double &result);
};
