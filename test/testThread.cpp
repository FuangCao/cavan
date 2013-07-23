#include <stdio.h>
#include <cavan++/thread.h>

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

	return 0;
}
