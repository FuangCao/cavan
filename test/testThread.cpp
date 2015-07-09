#include <stdio.h>
#include <cavan++/Thread.h>

class MyThread : public CavanThread
{
public:
	MyThread(const char *name) : CavanThread(name) {}
	int run(void)
	{
		pr_pos_info();
		msleep(200);

		return 0;
	}
};

static int handler(CavanThread *CavanThread)
{
	pr_pos_info();
	msleep(200);

	return 0;
}

int main(int argc, char *argv[])
{
	CavanThread CavanThread("Test", handler);

	CavanThread.start();
	// CavanThread.join();
	msleep(2000);
	CavanThread.suspend();
	msleep(2000);
	CavanThread.resume();
	msleep(2000);
	CavanThread.stop();

	MyThread thread2("Test2");
	thread2.start();
	// thread2.join();
	msleep(2000);
	thread2.suspend();
	msleep(2000);
	thread2.resume();
	msleep(2000);
	thread2.stop();

	return 0;
}
