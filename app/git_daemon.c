// Fuang.Cao <cavan.cfa@gmail.com> Tue Mar 29 10:25:45 CST 2011

#include <cavan.h>

#define GIT_DAEMON_PATH			"/usr/lib/git-core/git-daemon"
#define DEFAULT_GIT_ARGUMENT	GIT_DAEMON_PATH " --verbose --export-all --enable=receive-pack --enable=upload-pack --enable=upload-archive"

int main(int argc, char *argv[])
{
	char command[1024];

	sprintf(command, DEFAULT_GIT_ARGUMENT " --base-path=%s", argc > 2 ? argv[2] : "/git");

	if (argc > 1 && argv[1][0] != '-') {
		sprintf(command + strlen(command), " --port=%s", argv[1]);
	}

	if (fork() == 0) {
		return system(command);
	}

	return 0;
}
