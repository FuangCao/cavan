//=====================================Stack.cpp============================
//创建者：曹福昂
//学号：200600404071
//单位：广西工学院
//院系：计算机工程系
//班级：计Y062班
//创建时间：2008年6月3日
#include<iostream>
#include<string>
using namespace std;
template <class StackDataType>
class Stack
{
private:
	int StackMax;
	StackDataType *StackData;
	int top;
public:	
	Stack(int m=10);
	~Stack();
	int PushStack(StackDataType a);
	int PopStack(StackDataType &a);
	int GetTopData(StackDataType &a);
	int State();
};
template <class StackDataType>
Stack<StackDataType>::Stack(int m)//构造函数
{
	top=-1;
	StackMax=m;
	StackData=new StackDataType[StackMax];
}
template <class StackDataType>
Stack<StackDataType>::~Stack()    //析构函数
{
	if(StackData)
		delete[] StackData;
}
template <class StackDataType>
int Stack<StackDataType>::GetTopData(StackDataType &a)//获取栈顶元素
{
	if(State()<0)
		return 0;
	a=StackData[top];
	return 1;
}
template <class StackDataType>
int Stack<StackDataType>::State() //判断栈的状态
{
	if(top<0)
		return -1; //栈空
	else if(top<StackMax-1)
		return 0; //正常
	else
		return 1; //栈满
}
template <class StackDataType>
int Stack<StackDataType>::PushStack(StackDataType a)  //入栈
{
	if(State()>0)
		return 0;
	StackData[++top]=a;
	return 1;
}
template <class StackDataType>
int Stack<StackDataType>::PopStack(StackDataType &a)  //出栈
{
	if(State()<0)
		return 0;
	a=StackData[top--];
	return 1;
}
