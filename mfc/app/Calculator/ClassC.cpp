//=====================================classC.cpp============================
//创建者：曹福昂
//学号：200600404071
//单位：广西工学院
//院系：计算机工程系
//班级：计Y062班
//创建时间：2008年6月3日
int degree(long double &n,const string &s,int &i)//计算指数运算符^后面的算式的值
{	
	string s1;
	long double d;
	n=1;
	while(s[i])
	{
		if(s[i]!='^')
			break;
		i++;
		s1="\0";
		if(IsBracket(s[i])==1)
			s1=GetBracketData(s,i);
		else if(IsData(s[i])==1)
			while(IsData(s[i])==1)
				s1+=s[i++];
		else
			return 0;
		if(!compute1(d,s1))
			return 0;
		n*=d;
	}
	return 1;
}
int IsDXS(string s)//判断算式s是否是项式
{
	for(int i=0;s[i];i++)
		if(IsData(s[i])>1)
			return 1;
	return 0;
}
int IsOperator2(char a) //判断字符a是否是多项式的运算符
{
	char *p="+-*^/%";
	while(*p)
	{
		if(*p==a)
			return 1;
		p++;
	}
	return 0;
}
void C::Initialize()    //初始化多项式
{
	if(out==NULL)
		return;
	B *pb=out,*qb;
	A *pa,*qa;
	while(pb)
	{
		pa=pb->out;
		while(pa)
		{
			qa=pa->next;
			delete pa;
			pa=qa;
		}
		qb=pb->next;
		delete pb;
		pb=qb;
	}
	out=NULL;
}
void C::Insert(B *p)    //按从小到大的顺序，在多项式中插入一个不含运算符的多项式p
{
	if(!p)
		return;
	if(p->xs==0)
	{
		p->Initialize();
		delete p;
		return;
	}
	if(out==NULL)
	{
		out=p;
		return;
	}
	B *p1=out,*p2;
	while(p1&&compare(p1,p)<0)
	{
		p2=p1;
		p1=p1->next;
	}
	if(p1&&compare(p1,p)==0)
	{
		p1->xs+=p->xs;
		if(!p1->xs)
		{
			if(p1==out)
				out=out->next;
			else
				p2->next=p1->next;
			p1->Initialize();
			p->Initialize();
			delete p1;
			delete p;
		}
	}
	else
	{
		if(p1==out)
		{
			p->next=out;
			out=p;
		}
		else
		{
			p->next=p1;
			p2->next=p;
		}
	}
}
void C::Insert(long double a)//在多项式中插入符点数a
{
	if(!a)
		return;
	B *p=new B;
	p->xs=a;
	Insert(p);
}
void Add(C &a,C b) //计算多项式a、b之和并赋给a，通过参数带回
{
	B *p=b.out;
	while(p)
	{
		a.Insert(Copy(p));
		p=p->next;
	}
}
void Subtration(C &a,C b)    //计算多项式a、b之差并赋给a，通过参数带回
{
	B *p=b.out,*q;
	while(p)
	{
		q=Copy(p);
		q->xs*=-1;
		a.Insert(q);
		p=p->next;
	}
}
void Multip(C &a,C b)   //计算多项式a、b之积并赋给a，通过参数带回
{
	C c;
	B *pa=a.out,*pb;
	while(pa)
	{
		pb=b.out;
		while(pb)
		{
			c.Insert(Copy(Multip(pa,pb)));
			pb=pb->next;
		}
		pa=pa->next;
	}
	a.Initialize();
	a=c;
}
int Index(C &a,int n)   //计算多项式a的n次方并赋给a
{
	if(n<0)
		return 0;
	C b;
	B *p=new B;
	p->xs=1;
	b.Insert(p);
	for(int i=0;i<n;i++)
		Multip(b,a);
	a.Initialize();
	a=b;
	return 1;
}
int C::Multinomial_CString(CString &cs)//将多项式转化成CString类型，若溢出则返回0，否则返回1
{
	cs="\0";
	if(!out)
	{
		cs+="0";
		return 1;
	}
	CString s="\0";
	B *p=out;
	while(p)
	{
		if(!p->Multinomial_CString(s))
			return 0;
		cs+=s;
		p=p->next;
		if(p&&p->xs>0)
			cs+="+";
	}
	return 1;
}
void C::Output()   //在DOS环境下输出多项式
{
	if(!out)
	{
		cout<<0<<endl;
		return;
	}
	B *p=out;
	while(p)
	{
		p->Output();
		p=p->next;
		if(p&&p->xs>0)
			cout<<"+";
	}
	cout<<endl;
}
int Account2(C &a,C &b,char c)    //进行多项式运算
{
	switch(c)
	{
	case'+':Add(a,b);break;
	case'-':Subtration(a,b);break;
	case'*':Multip(a,b);break;
	default:return 0;
	}
	b.Initialize();
	return 1;
}
int compute2(C &result,string s)  //进行多项式综合运算
{
	if(!check_b(s))
		return 0;
	Stack<C> ss(20);
	Stack<char> sf(20);
	int i=0;
	string s1;
	C a,b;
	char c;
	long double d1,d2;
	B *pb;
	while(IsUselessChar(s[i]))
		i++;
	if(s[i]=='-')
	{
		a.Insert(-1);
		ss.PushStack(a);
		sf.PushStack('*');
		i++;
	}
	else if(s[i]=='+')
		i++;
	while(s[i])
	{
		if(IsUselessChar(s[i]))
			i++;
		else if(IsBracket(s[i])==1)
		{
			if(i>0&&!IsOperator2(s[i-1]))
				if(!sf.PushStack('*'))
					return 0;
			s1=GetBracketData(s,i);
			if(IsDXS(s1))
			{
				if(!compute2(a,s1))
					return 0;
			}
			else
			{
				if(!compute1(d1,s1))
					return 0;
				a.out=NULL;
				a.Insert(d1);
			}
			if(s[i]=='^')
				if(!degree(d1,s,i)||!Index(a,d1))
					return 0;
			if(!ss.PushStack(a))
				return 0;
		}
		else if(s[i]=='+'||s[i]=='-'||s[i]=='*')
		{
			while(sf.GetTopData(c))
			{
				if(PRI(s[i])>PRI(c))
					break;
				if(!ss.PopStack(b)||!ss.PopStack(a)||!sf.PopStack(c)||!Account2(a,b,c)||!ss.PushStack(a))
					return 0;
			}
			if(!sf.PushStack(s[i++]))
				return 0;
		}
		else if(s[i]=='/')
		{
			i++;
			s1="\0";
			while(s[i])
			{
				if(IsUselessChar(s[i]))
					i++;
				else if(IsOperator2(s[i])&&s[i]!='^')
					break;
				else if(IsBracket(s[i]))
				{
					s1+='(';
					s1+=GetBracketData(s,i);
					s1+=')';
				}
				else
					s1+=s[i++];
			}
			if(IsDXS(s1))
				return 0;
			if(!compute1(d1,s1)||d1==0||!ss.PopStack(a))
				return 0;
			b.out=NULL;
			b.Insert(1.0/d1);
			if(!Account2(a,b,'*')||!ss.PushStack(a))
				return 0;
		}
		else if(IsData(s[i]))
		{
			pb=new B;
			while(s[i])
			{
				if(IsUselessChar(s[i]))
						i++;
				else if(s[i]=='+'||s[i]=='-'||IsBracket(s[i])==1||s[i]=='/')
					break;
				else if(s[i]=='*')
					i++;
				else if(IsData(s[i])==1)
				{
					s1="\0";
					while(IsData(s[i])==1)
						s1+=s[i++];
					d1=string_double(s1);
					if(s[i]=='^')
					{
						if(!degree(d2,s,i))
							return 0;
						d1=pow(d1,d2);
					}
					pb->xs*=d1;
					if(pb->xs==0)
						break;
				}
				else if(IsData(s[i])>1)
				{
					c=s[i++];
					d1=1;
					if(s[i]=='^')
						if(!degree(d1,s,i))
							return 0;
					if(d1==0)
						continue;
					pb->Insert(c,d1);
				}
				else
					return 0;
			}
			a.out=NULL;
			if(pb->xs==0)
			{
				pb->Initialize();
				delete pb;
			}
			else
				a.Insert(pb);
			if(!ss.PushStack(a))
				return 0;
			if(IsBracket(s[i])==1&&s[i-1]=='*')
				if(!sf.PushStack('*'))
					return 0;
		}
		else
			return 0;
	}
	while(sf.PopStack(c))
		if(!ss.PopStack(b)||!ss.PopStack(a)||!Account2(a,b,c)||!ss.PushStack(a))
			return 0;
	if(!ss.PopStack(result)||ss.State()>=0)
		return 0;
	return 1;
}
