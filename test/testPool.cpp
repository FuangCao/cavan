#include <stdio.h>
#include <cavan++/pool.h>

struct AA
{
	int a;
	int b;
	int c;
	int d;
	int e;
};

int main(int argc, char *argv[])
{
	LinkPool<AA> pool(5);
	AA *data[20];

	for (int i = 0; i < 2; i++)
	{
		for (int i = 0; i < NELEM(data); i++)
		{
			cout << "getNode: i = " << i << endl;
			data[i] = pool.getNode();
			memset(data[i], 0, sizeof(AA));
		}

		for (int i = 0; i < NELEM(data); i++)
		{
			cout << "putNode: i = " << i << endl;
			pool.putNode(data[i]);
		}
	}

	return 0;
}
