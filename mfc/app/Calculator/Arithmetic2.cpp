//=====================================arithmetic2.cpp============================
//�����ߣ��ܸ���
//ѧ�ţ�200600404071
//��λ��������ѧԺ
//Ժϵ�����������ϵ
//�༶����Y062��
//����ʱ�䣺2008��6��3��
long double Sin(long double a,int angle)    //���Ǻ���sin���㲢���ؽ��
{
	if(angle==0)
	{
		a=fmod(a,360);
		if(a==0||fabs(a)==180)
			return 0;
		else
			return sin(a*P/180);
	}
	else
	{
		a=fmod(a,2*P);
		if(a==0||fabs(a)==P)
			return 0;
		else
			return sin(a);
	}
}
long double Cos(long double a,int angle)    //���Ǻ���cos���㲢���ؽ��
{
	if(angle==0)
	{
		a=fmod(a,360);
		if(fabs(a)==90||fabs(a)==270)
			return 0;
		else
			return cos(a*P/180);
	}
	else
	{
		a=fmod(a,2*P);
		if(fabs(a)==P/2||fabs(a)==3*P/2)
			return 0;
		else
			return cos(a);
	}
}
int Tan(long double &a,long double b,int angle)  //���Ǻ���tan����
{
	if(angle==0)
	{
		b=fmod(b,180);
		if(fabs(a)==90)
			return 0;
		a=tan(b*P/180);
	}
	else
	{
		b=fmod(b,P);
		if(fabs(a)==P/2)
			return 0;
		a=tan(b);
	}
	return 1;
}
int Cot(long double &a,long double b,int angle)  //���Ǻ���cot����
{
	a=Sin(b,angle);
	if(a==0)
		return 0;
	a=Cos(b,angle)/a;
	return 1;
}
long double ACot(long double a,int angle)   //�����Ǻ���arccot����
{
	if(a==0)
	{
		if(angle==0)
			return 90;
		else
			return P/2;
	}
	a=atan(1/a);
	if(angle==0)
		a=a*180/P;
	return a;
}
int IsOperator3(string s)    //�ж�һ���ַ����Ƿ��ǵ�Ŀ�����
{
	string str[13]={"Ln","e^","log","sqr","sqt","sin","cos","tan","cot","arcsin","arccos","arctan","arccot"};
	for(int i=0;i<13;i++)
		if(str[i]==s)
			return 1;
	return 0;
}
int Access3(long double &a,long double b,string s,int angle=0)//�������������ĸ�ĵ�Ŀ���������������
{
	if(s=="Ln")
	{
		if(b<=0)
			return 0;
		a=log(b);
	}
	else if(s=="e^")
		a=exp(b);
	else if(s=="log")
	{
		if(b<=0)
			return 0;
		a=log10(b);
	}
	else if(s=="sqr")
	{
		if(b<0)
			return 0;
		a=sqrt(b);
	}
	else if(s=="sqt")
	{
		if(b<0)
		{
			b*=-1;
			a=pow(b,1.0/3.0);
			a*=-1;
		}
		else
			a=pow(b,1.0/3.0);
	}
	else if(s=="sin")
			a=Sin(b,angle);
	else if(s=="cos")
			a=Cos(b,angle);
	else if(s=="tan")
	{
		if(!Tan(a,b,angle))
			return 0;
	}
	else if(s=="cot")
	{
		if(!Cot(a,b,angle))
			return 0;
	}
	else if(s=="arcsin")
	{
		if(b*b>1)
			return 0;
		a=asin(b);
		if(angle==0)
			a=a*180/P;
	}
	else if(s=="arccos")
	{
		if(b*b>1)
			return 0;
		a=acos(b);
		if(angle==0)
			a=a*180/P;
	}
	else if(s=="arctan")
	{
		a=atan(b);
		if(angle==0)
			a=a*180/P;
	}
	else if(s=="arccot")
		a=ACot(b,angle);
	else
		return 0;
	return 1;
}
int compute3(long double &a,string s,long double b,int angle=0) //�������������ĸ�ĵ�Ŀ��������ۺ�����
{
	Stack<string> ss(10);
	string s1;
	int i=0,j;
	while(s[i])
	{
		s1="\0";
		for(j=0;s[i];j++)
		{
			s1+=s[i++];
			if(IsOperator3(s1))
					break;
		}
		if(j>6)
			return 0;
		if(!ss.PushStack(s1))
			return 0;
	}
	while(ss.PopStack(s1))
		if(!Access3(b,b,s1,angle))
			return 0;
	a=b;
	return 1;
}