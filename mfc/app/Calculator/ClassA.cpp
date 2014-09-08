//=====================================classA.cpp============================
//创建者：曹福昂
//学号：200600404071
//单位：广西工学院
//院系：计算机工程系
//班级：计Y062班
//创建时间：2008年6月3日
int compare(A *a,A *b)  //比较两个单字符的字符串的大小
{
	if(a&&b)
	{
		if(a->ds!=b->ds)
			return a->ds-b->ds;
		else
			return b->zs-a->zs;
	}
	else if(a)
		return 1;
	else if(b)
		return -1;
	else
		return 0;	
}