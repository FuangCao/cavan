//=====================================classB.cpp============================
//�����ߣ��ܸ���
//ѧ�ţ�200600404071
//��λ��������ѧԺ
//Ժϵ�����������ϵ
//�༶����Y062��
//����ʱ�䣺2008��6��3��
void B::Initialize()    //��ʼ��һ��������ת���Ķ���ʽ
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
int compare(B *a,B *b)  //�Ƚ�����������ת���Ķ���ʽ�Ĵ�С
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
void B::Insert(char ds,double zs) //����С�����˳�����һ�����ַ����ַ���
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
B *Multip(B *a,B *b)    //�������ʽa����b�������ؽ��
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
int B::Multinomial_CString(CString &cs)//������ʽת����CString����
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
void B::Output()   //��DOS�������������ʽ
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
B *Copy(B *p)      //���ƶ���ʽp�������ؽ��
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
