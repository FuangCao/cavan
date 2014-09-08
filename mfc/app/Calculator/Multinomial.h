//=====================================multinomial.h============================
//创建者：曹福昂
//学号：200600404071
//单位：广西工学院
//院系：计算机工程系
//班级：计Y062班
//创建时间：2008年6月3日
#include<iostream>
#include<string>
#include"math.h"
#include"stdio.h"
const long double P=3.141592653589793238462643383279502884197169399375105820974944;
const long double WQD=0.179769313486209E+309;
#include"Stack.cpp"
#include"Arithmetic2.cpp"
#include"Arithmetic1.cpp"
using namespace std;
struct A
{
	int zs;
	char ds;
	A *next;
	A(char d='\0',double z=0):ds(d),zs(z),next(NULL){}
	friend int compare(A *a,A *b);
};
struct B
{
	double xs;
	A *out;
	B *next;
	B():next(NULL),xs(1),out(NULL){}
	void Initialize();
	int Multinomial_CString(CString &cs);
	void Output();
	void Insert(char ds,double xs);
	friend int compare(B *a,B *b);
	friend B *Copy(B *p);
	friend B *Multip(B *a,B *b);	
};
class C
{
private:
	B *out;
public:
	C():out(NULL){}
	int Multinomial_CString(CString &cs);
	void Output();
	void Insert(B *p);
	void Insert(long double a);
	void Initialize();
	friend int Index(C &a,int n);
	friend long double switchover2(string s);
	friend void Multip(C &a,C b);
	friend void Add(C &a,C b);
	friend void Subtration(C &a,C b);
	friend int Account2(C &a,C &b,char c);
	friend int compute2(C &result,string s);
};
#include"ClassA.cpp"
#include"ClassB.cpp"
#include"ClassC.cpp"