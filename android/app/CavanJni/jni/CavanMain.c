#include <cavan/command.h>
#include "CavanMain.h"

static struct CavanProcessInfo sProcessInfoList[100];
static pthread_mutex_t sProcessInfoLock = PTHREAD_MUTEX_INITIALIZER;

static inline void ProcessInfoLock(void)
{
	pthread_mutex_lock(&sProcessInfoLock);
}

static inline void ProcessInfoUnlock(void)
{
	pthread_mutex_unlock(&sProcessInfoLock);
}

int CavanProcessInfoAdd(const char *name, pid_t pid)
{
	int i;

	pd_info("Add: process = %s, pid = %d", name, pid);

	ProcessInfoLock();

	for (i = NELEM(sProcessInfoList) - 1; i >= 0; i--) {
		if (sProcessInfoList[i].name) {
			if (strcmp(name, sProcessInfoList[i].name) == 0) {
				break;
			}

			continue;
		}

		sProcessInfoList[i].name = name;
		sProcessInfoList[i].pid = pid;
		break;
	}

	ProcessInfoUnlock();

	return i;
}

void CavanProcessInfoRemoveLocked(int index)
{
	pd_info("Remove: process = %s, pid = %d", sProcessInfoList[index].name, sProcessInfoList[index].pid);

	sProcessInfoList[index].name = NULL;
	sProcessInfoList[index].pid = 0;
}

void CavanProcessInfoRemove(int index)
{
	ProcessInfoLock();
	CavanProcessInfoRemoveLocked(index);
	ProcessInfoUnlock();
}

int CavanProcessInfoFindLocked(const char *name)
{
	int i;

	for (i = NELEM(sProcessInfoList) - 1; i >= 0; i--) {
		if (sProcessInfoList[i].name == NULL) {
			continue;
		}

		if (strcmp(name, sProcessInfoList[i].name) == 0) {
			break;
		}
	}

	return i;
}

int CavanProcessInfoFind(const char *name)
{
	int index;

	ProcessInfoLock();
	index = CavanProcessInfoFindLocked(name);
	ProcessInfoUnlock();

	return index;
}

bool CavanProcessKillLocked(const char *name)
{
	int index;

	index = CavanProcessInfoFindLocked(name);
	if (index < 0) {
		return false;
	}

	pd_info("Kill: process = %s, pid = %d", sProcessInfoList[index].name, sProcessInfoList[index].pid);
	kill(sProcessInfoList[index].pid, SIGTERM);

	return true;
}

bool CavanProcessKill(const char *name)
{
	bool success;

	ProcessInfoLock();
	success = CavanProcessKillLocked(name);
	ProcessInfoUnlock();

	return success;
}

int CavanMainRun(JNIEnv *env, const char *name, jobjectArray args, int (*main_func)(int argc, char *argv[]))
{
	int ret;

	optind = 0;

	if (args) {
		int i;
		int length = (*env)->GetArrayLength(env, args);
		char *argv[length + 2];

		argv[0] = (char *) name;
		argv[length + 1] = NULL;

		for (i = 0; i < length; i++) {
			jstring text = (jstring) (*env)->GetObjectArrayElement(env, args, i);
			argv[i + 1] = (char *) (*env)->GetStringUTFChars(env, text, NULL);
		}

		ret = main_func(length + 1, argv);

		while (--i >= 0) {
			jstring text = (jstring) (*env)->GetObjectArrayElement(env, args, i);
			(*env)->ReleaseStringUTFChars(env, text, argv[i + 1]);
		}
	} else {
		char *argv[] = { (char *) name, NULL };

		ret = main_func(1, argv);
	}

	return ret;
}

int CavanMainExecute(JNIEnv *env, const char *name, jboolean async, jobjectArray args, int (*main_func)(int argc, char *argv[]))
{
	pid_t pid;

	pid = cavan_exec_fork();
	if (pid != 0) {
		int code;

		if (pid < 0 || async) {
			return pid;
		}

		code = cavan_exec_waitpid(pid);
		if (code != 0) {
			return -code;
		}

		return 0;
	}

	_exit(CavanMainRun(env, name, args, main_func) < 0);

	return 0;
}
