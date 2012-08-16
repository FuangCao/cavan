package com.cavan.calculator2;

import java.util.EmptyStackException;
import java.util.Stack;

enum OperatorEnum
{
	ADD, SUB, MUL, DIV, MOD, LEFT_SHIFT, RIGHT_SHIFT, AND, OR, XOR, BASE, POW, SQRT,
	NON, RECIP, SIN, ASIN, COS, ACOS, TAN, ATAN, COT, ACOT,
	FACT, PI, E
}

public class Calculator
{
	private byte[] mFormula;
	private Stack<Double> mStackOperand;
	private Stack<Operator> mStackOperator;
	private String mErrorMessage;
	private int mStart, mLast;

	private Operator[] mOperators = new Operator[]
	{
		// need zero operands
		new OperatorOne(OperatorEnum.BASE, new String [] {"@", "base"}, 6),
		// need two operands
		new OperatorOne(OperatorEnum.SIN, new String [] {"sin"}, 4),
		new OperatorOne(OperatorEnum.ASIN, new String [] {"asin"}, 4),
		new OperatorOne(OperatorEnum.COS, new String [] {"cos"}, 4),
		new OperatorOne(OperatorEnum.ACOS, new String [] {"acos"}, 4),
		new OperatorOne(OperatorEnum.TAN, new String [] {"tan"}, 4),
		new OperatorOne(OperatorEnum.ATAN, new String [] {"atan"}, 4),
		new OperatorOne(OperatorEnum.COT, new String [] {"cot"}, 4),
		new OperatorOne(OperatorEnum.ACOT, new String [] {"acot"}, 4),
		new OperatorOne(OperatorEnum.RECIP, new String [] {"recip"}, 4),
		new OperatorOne(OperatorEnum.NON, new String [] {"~", "non"}, 4),
		// need one operand
		new OperatorTwo(OperatorEnum.POW, new String [] {"**", "pow"}, 5),
		new OperatorTwo(OperatorEnum.SQRT, new String [] {"//", "sqrt"}, 5),
		new OperatorTwo(OperatorEnum.ADD, new String [] {"+", "add"}, 1),
		new OperatorTwo(OperatorEnum.SUB, new String [] {"-", "sub"}, 1),
		new OperatorTwo(OperatorEnum.LEFT_SHIFT, new String [] {"<<", "left"}, 1),
		new OperatorTwo(OperatorEnum.RIGHT_SHIFT, new String [] {">>", "right"}, 1),
		new OperatorTwo(OperatorEnum.MUL, new String [] {"*", "mul"}, 2),
		new OperatorTwo(OperatorEnum.DIV, new String [] {"/", "div"}, 2),
		new OperatorTwo(OperatorEnum.MOD, new String [] {"%", "mod"}, 2),
		new OperatorTwo(OperatorEnum.AND, new String [] {"&", "and"}, 2),
		new OperatorTwo(OperatorEnum.OR, new String [] {"|", "or"}, 2),
		new OperatorTwo(OperatorEnum.XOR, new String [] {"^", "xor"}, 2),
	};

	private Operator[] mOperatorSpecials = new Operator[]
	{
		new OperatorOne(OperatorEnum.FACT, new String [] {"!", "fact"}, 0),
		new OperatorConst(OperatorEnum.PI, new String [] {"pi"}, 0),
		new OperatorConst(OperatorEnum.E, new String [] {"e"}, 0)
	};

	public Calculator(String formula)
	{
		mFormula = formula.toLowerCase().getBytes();
		mStart = 0;
		mLast = mFormula.length - 1;
	}

	public Calculator(byte[] formula)
	{
		mFormula = formula;
		mStart = 0;
		mLast = mFormula.length - 1;
	}

	public Calculator(byte[] formula, int start, int last)
	{
		mFormula = formula;
		mStart = start;
		mLast = last;
	}

	public String getErrorMessage()
	{
		return mErrorMessage;
	}

	public void setErrorMessage(String format, Object ...args)
	{
		mErrorMessage = String.format(format, args);
	}

	boolean CheckBracketPair()
	{
		Stack<Byte> stack = new Stack<Byte>();
		for (int i = mStart; i <= mLast; i++)
		{
			switch (mFormula[i])
			{
			case '{':
			case '[':
			case '(':
				stack.push(mFormula[i]);
				break;

			case '}':
				if (stack.isEmpty() || stack.pop() != '{')
				{
					setErrorMessage("`%c' is not pair", mFormula[i]);
					return false;
				}
				break;

			case ']':
				if (stack.isEmpty() || stack.pop() != '[')
				{
					setErrorMessage("`%c' is not pair", mFormula[i]);
					return false;
				}
				break;

			case ')':
				if (stack.isEmpty() || stack.pop() != '(')
				{
					setErrorMessage("`%c' is not pair", mFormula[i]);
					return false;
				}
				break;
			}
		}

		return stack.isEmpty();
	}

	private int getDouble(int i, int base)
	{
		if (mFormula[i] == '0' && i < mLast)
		{
			switch (mFormula[i + 1])
			{
			case 'b':
			case 'B':
				base = 2;
				i += 2;
				break;
			case 'd':
			case 'D':
				base = 10;
				i += 2;
				break;
			case 'x':
			case 'X':
				base = 16;
				i += 2;
				break;
			default:
				if (mFormula[i + 1] >= '0' && mFormula[i + 1] <= '7')
				{
					base = 8;
					i++;
				}
				else
				{
					i++;
				}
			}
		}

		if (base < 2)
		{
			base = 10;
		}

		double value;

		for (value = 0; i <= mLast; i++)
		{
			int tmp = ByteToValue(mFormula[i]);
			if (tmp < 0 || tmp >= base)
			{
				break;
			}

			value = value * base + tmp;
		}

		if (i <= mLast && mFormula[i] == '.')
		{
			i++;
			for (double weight = (double) 1 / base; i <= mLast; i++, weight /= base)
			{
				int tmp = ByteToValue(mFormula[i]);
				if (tmp < 0 || tmp >= base)
				{
					break;
				}

				value += weight * tmp;
			}
		}

		if (i <= mLast && mFormula[i] == 'e')
		{
			i = getDouble(i + 1, base);
			double indicial = mStackOperand.pop();
			value *= Math.pow(base, indicial);
		}

		mStackOperand.push(value);

		return i;
	}

	int getOperator(int i)
	{
		int length = 0;
		int j;

		for (j = 0; j < mOperatorSpecials.length; j++)
		{
			length = mOperatorSpecials[j].isMatch(mFormula, i);
			if (length > 0)
			{
				if (mOperatorSpecials[j].calculate())
				{
					return i + length;
				}
				else
				{
					return -1;
				}
			}
		}

		for (j = 0; j < mOperators.length; j++)
		{
			length = mOperators[j].isMatch(mFormula, i);
			if (length > 0)
			{
				break;
			}
		}

		if (length <= 0)
		{
			return -1;
		}

		while (true)
		{
			if (mStackOperator.isEmpty())
			{
				break;
			}

			Operator operator = mStackOperator.peek();
			if (mOperators[j].PriorityCompare(operator) > 0)
			{
				break;
			}

			if (operator.calculate() == false)
			{
				return -1;
			}

			mStackOperator.pop();
		}

		mStackOperator.push(mOperators[j]);

		return i + length;
	}

	int getBracket(int i) throws Exception
	{
		int count;
		int i_bak;

		for (i_bak = i, count = 0; i <= mLast; i++)
		{
			switch (mFormula[i])
			{
			case '(':
			case '[':
			case '{':
				count++;
				break;

			case ')':
			case ']':
			case '}':
				count--;
				break;
			}

			if (count == 0)
			{
				break;
			}
		}

		Calculator calculator = new Calculator(mFormula, i_bak + 1, i - 1);
		mStackOperand.push(calculator.getResult());

		return i + 1;
	}

	public Double getResult() throws Exception
	{
		if (CheckBracketPair() == false)
		{
			throw new Exception("CheckBracketPair");
		}

		mStackOperand = new Stack<Double>();
		mStackOperator = new Stack<Calculator.Operator>();
		int i;

		for (i = mStart; i <= mLast; )
		{
			switch (mFormula[i])
			{
			case ' ':
			case '\t':
			case '\n':
			case '\f':
			case ')':
			case ']':
			case '}':
				i++;
				break;

			case '(':
			case '[':
			case '{':
				i = getBracket(i);
				break;

			default:
				if (mFormula[i] >= '0' && mFormula[i] <= '9')
				{
					double base;

					if (mStackOperator.isEmpty() || mStackOperator.peek().getEnumOperator() != OperatorEnum.BASE)
					{
						base = 0;
					}
					else
					{
						if (mStackOperand.isEmpty())
						{
							setErrorMessage("Too a few operand");
							throw new Exception("Too a few operand");
						}

						base = mStackOperand.pop();
						mStackOperator.pop();
					}

					i = getDouble(i, (int) base);
				}
				else
				{
					int tmp = getOperator(i);
					if (tmp < 0)
					{
						setErrorMessage("unknown char `%c'", mFormula[i]);
						throw new Exception("getOperator");
					}

					i = tmp;
				}
			}
		}

		while (true)
		{
			if (mStackOperator.isEmpty())
			{
				break;
			}

			Operator operator = mStackOperator.pop();
			if (operator.calculate() == false)
			{
				throw new Exception("operator.calculate");
			}
		}

		if (mStackOperand.isEmpty())
		{
			setErrorMessage("Too a few operand");
			throw new Exception("Too a few operand");
		}

		double result = mStackOperand.pop();
		if (mStackOperand.isEmpty())
		{
			return result;
		}

		setErrorMessage("Too much operand");
		throw new Exception("Too much operand");
	}

	public static final boolean ByteArrayMatchLH(byte[] left, byte[] right, int start)
	{
		if (left.length > (right.length - start))
		{
			return false;
		}

		for (byte b : left)
		{
			if (b != right[start])
			{
				return false;
			}

			start++;
		}

		return true;
	}

	public static final double RadiansToAngle(double radians)
	{
		return radians * 180 / Math.PI;
	}

	public static final double AngleToRadians(double angle)
	{
		return angle * Math.PI / 180;
	}

	public static int ByteToValue(byte b)
	{
		if (ByteIsNumber(b))
		{
			return b - '0';
		}
		else if (ByteIsLowercase(b))
		{
			return b - 'a' + 10;
		}
		else if (ByteIsUppercase(b))
		{
			return b - 'A' + 10;
		}
		else
		{
			return -1;
		}
	}

	public static boolean ByteIsNumber(byte b)
	{
		return b >= '0' && b <= '9';
	}

	public static boolean ByteIsUppercase(byte b)
	{
		return b >= 'A' && b <= 'Z';
	}

	public static boolean ByteIsLowercase(byte b)
	{
		return b >= 'a' && b <= 'z';
	}

	public static boolean ByteIsSpace(byte b)
	{
		return b == ' ' || b == '\t' || b == '\n' || b == '\r' || b == '\f';
	}

	abstract class Operator
	{
		private String[] mSymbols;
		private int mPriority;
		protected OperatorEnum mOperatorEnum;

		public Operator(OperatorEnum operator, String[] symbols, int priority)
		{
			mOperatorEnum = operator;
			mSymbols = symbols;
			mPriority = priority;
		}

		public int getPriority()
		{
			return mPriority;
		}

		public OperatorEnum getEnumOperator()
		{
			return mOperatorEnum;
		}

		public int PriorityCompare(Operator operator)
		{
			return mPriority - operator.getPriority();
		}

		public int isMatch(byte[] formula, int start)
		{
			for (String symbol : mSymbols)
			{
				if (ByteArrayMatchLH(symbol.getBytes(), formula, start))
				{
					return symbol.length();
				}
			}

			return 0;
		}

		abstract public boolean calculate();
	}

	class OperatorZero extends Operator
	{
		public OperatorZero(OperatorEnum operator, String[] symbols, int priority)
		{
			super(operator, symbols, priority);
		}

		@Override
		public boolean calculate()
		{
			switch (mOperatorEnum)
			{
			case BASE:
				return true;

			default:
				return false;
			}
		}
	}

	class OperatorOne extends Operator
	{
		public OperatorOne(OperatorEnum operator, String[] symbols, int priority)
		{
			super(operator, symbols, priority);
		}

		@Override
		public boolean calculate()
		{
			double operand;

			try
			{
				operand = mStackOperand.pop();
			}
			catch (EmptyStackException e)
			{
				setErrorMessage("Too a few operand");
				return false;
			}

			switch (mOperatorEnum)
			{
			case NON:
				operand = ~(long) operand;
				break;
			case FACT:
				long i = (long) operand;
				for (operand = 1; i > 1; i--)
				{
					operand *= i;
				}
				break;
			case RECIP:
				operand = 1 / operand;
				break;
			case SIN:
				operand = Math.sin(AngleToRadians(operand));
				break;
			case ASIN:
				operand = RadiansToAngle(Math.asin(operand));
				break;
			case COS:
				operand = Math.cos(AngleToRadians(operand));
				break;
			case ACOS:
				operand = RadiansToAngle(Math.acos(operand));
				break;
			case TAN:
				operand = Math.tan(AngleToRadians(operand));
				break;
			case ATAN:
				operand = RadiansToAngle(Math.atan(operand));
				break;
			case COT:
				operand = Math.tan(AngleToRadians(90 - operand));
				break;
			case ACOT:
				operand = 90 - RadiansToAngle(Math.atan(operand));
				break;

			default:
				setErrorMessage("unknown operator");
				return false;
			}

			mStackOperand.push(operand);

			return true;
		}
	}

	class OperatorTwo extends Operator
	{
		public OperatorTwo(OperatorEnum operator, String[] symbols, int priority)
		{
			super(operator, symbols, priority);
		}

		@Override
		public boolean calculate()
		{
			double left, right;

			try
			{
				right = mStackOperand.pop();
				left = mStackOperand.pop();
			}
			catch (EmptyStackException e)
			{
				setErrorMessage("Too a few operand");
				return false;
			}

			switch (mOperatorEnum)
			{
			case ADD:
				left += right;
				break;
			case SUB:
				left -= right;
				break;
			case MUL:
				left *= right;
				break;
			case DIV:
				if (right == 0)
				{
					setErrorMessage("div by zero");
					return false;
				}
				left /= right;
				break;
			case MOD:
				if (right == 0)
				{
					setErrorMessage("mod by zero");
					return false;
				}
				left %= right;
				break;
			case LEFT_SHIFT:
				while (right >= 1)
				{
					left *= 2;
					right--;
				}
				break;
			case RIGHT_SHIFT:
				while (right >= 1)
				{
					left /= 2;
					right--;
				}
				break;
			case AND:
				left = ((int) left) & ((int) right);
				break;
			case OR:
				left = ((long) left) | ((long) right);
				break;
			case XOR:
				left = ((long) left) ^ ((long) right);
				break;
			case POW:
				left = Math.pow(left, right);
				break;
			case SQRT:
				if (left == 2.0)
				{
					left = Math.sqrt(right);
				}
				else
				{
					left = Math.pow(right, 1 / left);
				}
				break;

			default:
				return false;
			}

			mStackOperand.push(left);

			return true;
		}
	}

	class OperatorConst extends Operator
	{
		public OperatorConst(OperatorEnum operator, String[] symbols, int priority)
		{
			super(operator, symbols, priority);
		}

		@Override
		public boolean calculate()
		{
			double result;

			switch (mOperatorEnum)
			{
			case PI:
				result = Math.PI;
				break;

			case E:
				result = Math.E;
				break;

			default:
				setErrorMessage("unknown operator");
				return false;
			}

			mStackOperand.push(result);

			return true;
		}
	}
}