/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sun Jan 27 00:47:46 CST 2013
 */

#include <cavan.h>
#include <cavan/command.h>

int main(int argc, char *argv[])
{
	assert(argc == 2);

	return cavan_tty_redirect(argv[1]);
}
