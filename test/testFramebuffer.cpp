#include <stdio.h>
#include <cavan++/framebuffer.h>

int main(int argc, char *argv[])
{
	Framebuffer fb;

	fb.open();
	fb.close();

	return 0;
}
