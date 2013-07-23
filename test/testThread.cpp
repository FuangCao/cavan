#include <stdio.h>
#include <cavan++/thread.h>

class MyThread : public Thread
{
public:
	MyThread(const char *name) : Thread(name) {}
	int run(void)
	{
		pr_pos_info();
		msleep(200);

		return 0;
	}
};

static int handler(Thread *thread)
{
	pr_pos_info();
	msleep(200);

	return 0;
}

int main(int argc, char *argv[])
{
	Thread thread("Test", handler);

	thread.start();
	// thread.join();
	msleep(2000);
	thread.suspend();
	msleep(2000);
	thread.resume();
	msleep(2000);
	thread.stop();

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
