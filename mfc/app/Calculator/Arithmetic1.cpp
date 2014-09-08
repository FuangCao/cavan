//=====================================arithmetic1.cpp============================
//�����ߣ��ܸ���
//ѧ�ţ�200600404071
//��λ��������ѧԺ
//Ժϵ�����������ϵ
//�༶����Y062��
//����ʱ�䣺2008��6��3��
int double_CString(CString &cs,long double d)//�Ѹ�����ת�����ַ���
{
	if(d>WQD)      //������򷵻�0
		return 0;
	cs="\0";
	if(d==0)
	{
		cs+="0";
		return 1;
	}
	int n=0;       //ͳ��ָ��
	if(fabs(d)>1E+5)
	{
		while(fabs(d)>1)
		{
			d/=10;
			n++;
		}
	}
	else if(fabs(d)<1E-4)
	{
		while(fabs(d)<0.1)
		{
			d*=10;
			n--;
		}
	}
	CString s;
	int m=0;
	s.Format("%0.12lf",d);   //ͳ��û���õ�0����Ŀ
	for(int i=s.GetLength()-1;i>=0;i--)
		if(s[i]=='0')
			m++;
		else if(s[i]=='.')
		{
			m++;
			break;
		}
		else
			break;
	m=s.GetLength()-m;
	for(i=0;i<m;i++)
		cs+=s[i];
	if(n!=0)       //������10^5��С��10(-4)���ÿ�ѧ����������
	{
		cs+="*10^";
		s.Format("%d",n);
		if(n<0)
			cs+="("+s+")";
		else
			cs+=s;
	}
	return 1;
}
int PRI(char a)    //��������ȼ��ж�
{
	if(a=='+'||a=='-')
		return 1;
	else if(a=='*'||a=='/'||a=='%')
		return 2;
	else if(a=='^'||a=='A'||a=='C')
		return 3;
	else
		return 0;
}
int IsData(char a) //�ж�һ���ַ������ֻ�����ĸ���Ǵ�д��ĸ����Сд��ĸ
{
	if(a>='0'&&a<='9'||a=='.')
		return 1;
	else if(a>='a'&&a<='z')
		return 2;
	else if(a>='A'&&a<='Z')
		return 3;
	else
		return 0;
}
int IsBracket(char a)   //�ж�һ���ַ��ǲ������ţ��������Ż���������
{
	if(a=='('||a=='['||a=='{')
		return 1;
	else if(a==')'||a==']'||a=='}')
		return 2;
	return 0;
}
int IsUselessChar(char a)    //�ж�һ���ַ��ǲ��ǿ����ַ�
{
	if(a=='\n'||a=='\t'||a=='\b'||a=='\r'||a=='\f'||a==' ')
		return 1;
	return 0;
}
inline string GetBracketData(const string &s,int &i)  //��ȡһ�������е�����
{
	string s1="\0";
	int j,k=1;
	i++;
	while(s[i])
	{
		j=IsBracket(s[i]);
		if(j==1)
			k++;
		else if(j==2)
			k--;
		if(k==0)
		{
			i++;
			break;
		}
		s1+=s[i++];
	}
	return s1;
}
int check_b(const string &s) //���ʽ������Լ��
{
	Stack<char> a(100);
	char c;
	for(int i=0;s[i];i++)
	{
		if(!IsBracket(s[i]))
			continue;
		else if(IsBracket(s[i])==1)
		{
			if(!a.PushStack(s[i]))
				return 0;
		}
		else if(s[i]==')')
		{
			if(s[i-1]=='(')
				return 0;
			if(a.PopStack(c)&&c=='(')
				continue;
			return 0;
		}
		else if(s[i]==']')
		{
			if(s[i-1]=='(')
				return 0;
			if(a.PopStack(c)&&c=='[')
				continue;
			return 0;
		}
		else if(s[i]=='}')
		{
			if(s[i-1]=='(')
				return 0;
			if(a.PopStack(c)&&c=='{')
				continue;
			return 0;
		}
	}
	if(a.State()>=0)
		return 0;
	return 1;
}
long double string_double(string s)    //��һ��ֻ�������ַ���С����ı��ʽת���ɷ�����
{
	long double a=0,b=1;
	string c="\0";
	int i=0,j;
	while(s[i])    //��ȡ��������
	{
		if(s[i]=='.')
		{
			i++;
			break;
		}
		c+=s[i++];
	}
	for(j=c.length()-1;j>=0;j--)  //ת����������
	{
		a+=(c[j]-'0')*b;
		b*=10;
	}
	if(s[i])
	{
		b=0.1;
		c="\0";
		while(s[i])//��ȡС������
			c+=s[i++];
		for(i=c.length(),j=0;j<i;j++)  //ת��С������
		{
			a+=(c[j]-'0')*b;
			b*=0.1;
		}
	}	
	return a;
}
int IsOperator1(char a) //�ж�һ���ַ��ǲ������������
{
	char *p="+-*/%^!AC";
	while(*p)
	{
		if(*p==a)
			return 1;
		p++;
	}
	return 0;
}
int Sqrt(long double &a,long double b) //��������
{
	if(b==0)
		return 0;
	if(a<0)
	{
		CString s;
		s.Format("%lf",b);
		for(int i=s.GetLength()-1;s[i]=='0';i--);
		if(s[i]=='.')
			i--;
		if((s[i]-'0')%2==0)
			return 0;
		a*=-1;
		a=-1*pow(a,1.0/b);
	}
	else
		a=pow(a,1.0/b);
	return 1;
}
int factorial(long double &a,int b)    //����b�Ľ׳ˣ�������a
{
	if(b<0)
		return 0;
	a=1;
	while(b>0)
		a*=b--;
	return 1;
}
int nAr(long double &a,int x,int y)    //����x����ѡy�����������������������a
{
	if(y<0||x<0||y>x)
		return 0;
	if(y==0)
		a=1;
	else if(x==y)
		factorial(a,x);
	else
	{
		long double b;
		factorial(a,x);
		factorial(b,x-y);
		a/=b;
	}
	return 1;
}
int nCr(long double &a,int x,int y)    //����x����ѡy����������������������a
{
	if(!nAr(a,x,y))
		return 0;
	long double b;
	factorial(b,y);
	a/=b;
	return 1;
}
int Account1(long double &a,long double b,char c)//˫Ŀ��������
{
	switch(c)
	{
	case'+':a+=b;break;
	case'-':a-=b;break;
	case'*':a*=b;break;
	case'/':
		{
			if(b==0)
				return 0;
			a/=b;
			break;
		}
	case'%':
		{
			if(b==0)
				return 0;
			a=fmod(a,b);
			break;
		}
	case'^':a=pow(a,b);break;
	case'A':return nAr(a,a,b);
	case'C':return nCr(a,a,b);
	default:return 0;
	}
	return 1;
}
int compute1(long double &a,string s,int angle=0)//�����ۺ�����
{
	if(!check_b(s))
		return 0;
	Stack<long double> ss(20);
	Stack<char> sf(20);
	int i=0,j;
	string s1,s2;
	char c;
	long double b;
	while(IsUselessChar(s[i]))
		i++;
	if(s[i]=='-')
	{
		ss.PushStack(-1);
		sf.PushStack('*');
		i++;
	}
	else if(s[i]=='+')
		i++;
	while(s[i])
	{
		s2="\0";
		if(IsUselessChar(s[i]))
			i++;
		else if(IsBracket(s[i])==1)
		{
			if(i>0&&!IsOperator1(s[i-1]))
				if(!sf.PushStack('*'))
					return 0;
			s2=GetBracketData(s,i);
			if(!compute1(a,s2,angle)||!ss.PushStack(a))
				return 0;
		}
		else if(IsOperator1(s[i]))
		{
			if(s[i]=='!')
			{
				if(!ss.PopStack(b)||!factorial(a,(int)b)||!ss.PushStack(a))
					return 0;
				i++;
				continue;
			}
			while(sf.GetTopData(c))
			{
				if(PRI(s[i])>PRI(c))
					break;
				if(!ss.PopStack(b)||!ss.PopStack(a)||!sf.PopStack(c)||!Account1(a,b,c)||!ss.PushStack(a))
					return 0;
			}
			if(!sf.PushStack(s[i++]))
				return 0;
		}
		else if(IsData(s[i])==1)
		{
			while(IsData(s[i])==1)
				s2+=s[i++];
			if(!ss.PushStack(string_double(s2)))
				return 0;
		}
		else if(s[i]=='S')
		{
			if(i>0&&!IsOperator1(s[i-1]))
				if(!sf.PushStack('*'))
					return 0;
			s2="\0";
			for(j=0;j<4&&s[i];j++,i++)
				s2+=s[i];
			if(s2!="Sqrt"||IsBracket(s[i])!=1)
				return 0;
			s2=GetBracketData(s,i);
			s1="\0";
			for(j=0;s2[j];j++)
			{
				if(IsBracket(s2[j])==1)
					s1+="("+GetBracketData(s2,j)+")";
				if(s2[j]==',')
				{
					j++;
					break;
				}
				else
					s1+=s2[j];
			}
			if(!compute1(a,s1,angle))
				return 0;
			s1="\0";
			for(;s2[j];j++)
				s1+=s2[j];
			if(!compute1(b,s1,angle)||!Sqrt(a,b)||!ss.PushStack(a))
				return 0;
		}
		else if(s[i]>='a'&&s[i]<='z'||s[i]=='L')
		{
			s1="\0";
			if(i>0&&!IsOperator1(s[i-1]))
				if(!sf.PushStack('*'))
					return 0;
			while(s[i]>='a'&&s[i]<='z'||s[i]=='L'||s[i]=='^')
			{
				s1+=s[i++];
				if(s[i-1]=='e'&&s[i]!='^')
					s1+='^';
			}
			if(s[i-1]=='e')
				s2="1";
			else if(IsBracket(s[i])==1)
				s2=GetBracketData(s,i);
			else if(IsData(s[i])==1)
			{
				while(s[i])
				{
					if(IsData(s[i])==1)
						s2+=s[i++];
					else
						break;
				}
			}
			else if(s[i]=='P'||s[i+1]=='i')
				s2="Pi";
			else if(s[i]=='S')
			{
				for(j=0;j<4&&s[i];j++,i++)
					s2+=s[i];
				if(s2!="Sqrt"||IsBracket(s[i])!=1)
					return 0;
				s2+="("+GetBracketData(s,i)+")";
			}
			else
				return 0;
			if(!compute1(a,s2,angle)||!compute3(b,s1,a,angle)||!ss.PushStack(b))
				return 0;
		}
		else if(s[i]=='P')
		{	
			if(i>0&&!IsOperator1(s[i-1])&&!sf.PushStack('*')||!ss.PushStack(P))
				return 0;
			i+=2;
		}
		else
			return 0;
	}
	while(sf.PopStack(c))
		if(!ss.PopStack(b)||!ss.PopStack(a)||!Account1(a,b,c)||!ss.PushStack(a))
			return 0;
	if(!ss.PopStack(a)||ss.State()>=0)
		return 0;
	return 1;
}
