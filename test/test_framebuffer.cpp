#include <stdio.h>
#include <cavan++/Framebuffer.h>

int main(int argc, char *argv[])
{
	Framebuffer fb;

	fb.open();
	fb.close();

	return 0;
}
