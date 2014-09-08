//=====================================classB.cpp============================
//创建者：曹福昂
//学号：200600404071
//单位：广西工学院
//院系：计算机工程系
//班级：计Y062班
//创建时间：2008年6月3日
void B::Initialize()    //初始化一个不含运转符的多项式
{
	xs=0;
	next=NULL;
	if(out==NULL)
		return;
	A *pa=out,*qa;
	while(pa)
	{
		qa=pa->next;
		delete pa;
		pa=qa;
	}
	out=NULL;
}
int compare(B *a,B *b)  //比较两个不含运转符的多项式的大小
{
	A *p=a->out,*q=b->out;
	while(p&&q)
	{
		if(compare(p,q))
			break;
		p=p->next;
		q=q->next;
	}
	return compare(p,q);
}
void B::Insert(char ds,double zs) //按从小到大的顺序插入一个单字符的字符串
{
	if(zs==0)
		return;
	if(!out)
	{
		out=new A(ds,zs);
		return;
	}
	A *p=out,*q;
	while(p&&p->ds<ds)
	{
		q=p;
		p=p->next;
	}
	if(p&&p->ds==ds)
	{
		p->zs+=zs;
		if(!p->zs)
		{
			if(p==out)
				out=out->next;
			else
				q->next=p->next;
			delete p;
		}
	}
	else
	{
		A *np=new A(ds,zs);
		if(p==out)
		{
			np->next=out;
			out=np;
		}
		else
		{
			q->next=np;
			np->next=p;
		}
	}
}
B *Multip(B *a,B *b)    //计算多项式a乘以b，并返回结果
{
	if(!a||!b)
		return NULL;
	B *c=new B;
	c->xs=a->xs*b->xs;
	A *pa=a->out,*pb=b->out;
	while(pa)
	{
		c->Insert(pa->ds,pa->zs);
		pa=pa->next;
	}
	while(pb)
	{
		c->Insert(pb->ds,pb->zs);
		pb=pb->next;
	}
	return c;
}
int B::Multinomial_CString(CString &cs)//将多项式转化成CString类型
{
	
	if(!out)
	{
		if(!double_CString(cs,xs))
			return 0;
		return 1;
	}
	CString s="\0";
	cs="\0";
	if(xs==-1)
		cs+="-";
	else if(xs!=1)
	{
		if(!double_CString(s,xs))
			return 0;
		cs+=s;
		cs+="*";
	}
	A *p=out;
	while(p)
	{
		if(p->zs!=1)
		{
			cs+=p->ds;
			cs+="^";
			if(!double_CString(s,p->zs))
				return 0;
			if(p->zs<0)
				cs+="("+s+")";
			else
				cs+=s;
			if(p->next)
				cs+="*";
		}
		else
		{
			cs+=p->ds;
			if(p->next&&p->next->zs!=1)
				cs+="*";
		}
		p=p->next;
	}
	return 1;
}
void B::Output()   //在DOS环境下输出多项式
{
	if(!out)
		if(xs==1||xs==-1)
		{
			cout<<xs;
			return;
		}
	if(xs==-1)
		cout<<"-";
	else if(xs!=1)
	{
		cout<<xs;
		if(out)
			cout<<"*";
	}
	if(out)
	{
		A *p=out;
		while(p)
		{
			if(p->zs!=1)
			{
				cout<<p->ds<<"^";
				if(p->zs<0)
					cout<<"("<<p->zs<<")";
				else
					cout<<p->zs;
				if(p->next)
					cout<<"*";
			}
			else
			{
				cout<<p->ds;
				if(p->next&&p->next->zs!=1)
					cout<<"*";
			}
			p=p->next;
		}
	}
}
B *Copy(B *p)      //复制多项式p，并返回结果
{
	if(p==NULL)
		return NULL;
	B *np=new B;
	np->xs=p->xs;
	A *q=p->out;
	while(q)
	{
		np->Insert(q->ds,q->zs);
		q=q->next;
	}
	return np;
}
