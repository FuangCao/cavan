//=====================================Stack.cpp============================
//�����ߣ��ܸ���
//ѧ�ţ�200600404071
//��λ��������ѧԺ
//Ժϵ�����������ϵ
//�༶����Y062��
//����ʱ�䣺2008��6��3��
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
Stack<StackDataType>::Stack(int m)//���캯��
{
	top=-1;
	StackMax=m;
	StackData=new StackDataType[StackMax];
}
template <class StackDataType>
Stack<StackDataType>::~Stack()    //��������
{
	if(StackData)
		delete[] StackData;
}
template <class StackDataType>
int Stack<StackDataType>::GetTopData(StackDataType &a)//��ȡջ��Ԫ��
{
	if(State()<0)
		return 0;
	a=StackData[top];
	return 1;
}
template <class StackDataType>
int Stack<StackDataType>::State() //�ж�ջ��״̬
{
	if(top<0)
		return -1; //ջ��
	else if(top<StackMax-1)
		return 0; //����
	else
		return 1; //ջ��
}
template <class StackDataType>
int Stack<StackDataType>::PushStack(StackDataType a)  //��ջ
{
	if(State()>0)
		return 0;
	StackData[++top]=a;
	return 1;
}
template <class StackDataType>
int Stack<StackDataType>::PopStack(StackDataType &a)  //��ջ
{
	if(State()<0)
		return 0;
	a=StackData[top--];
	return 1;
}
