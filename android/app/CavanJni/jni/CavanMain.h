#pragma once

#include <cavan.h>
#include <jni.h>

__BEGIN_DECLS

struct CavanProcessInfo {
	const char *name;
	pid_t pid;
};

int CavanProcessInfoAdd(const char *name, pid_t pid);
void CavanProcessInfoRemoveLocked(int index);
void CavanProcessInfoRemove(int index);
int CavanProcessInfoFindLocked(const char *name);
int CavanProcessInfoFind(const char *name);
bool CavanProcessKillLocked(const char *name);
bool CavanProcessKill(const char *name);

int CavanMainRun(JNIEnv *env, jobjectArray args, const char *name, int (*main_func)(int argc, char *argv[]));
int CavanMainExecute(JNIEnv *env, jobjectArray args, const char *name, int (*main_func)(int argc, char *argv[]));

__END_DECLS;
