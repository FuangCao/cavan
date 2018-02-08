/* This file was automatically generated by Fuang.Cao, Do not modify */

#include <cavan/command.h>
#include "CavanMain.h"

JNIEXPORT jint Java_com_cavan_cavanjni_CavanJni_kill(JNIEnv *env, jclass clazz, jint pid)
{
	pd_info("kill: %d", pid);
	return kill(pid, SIGTERM);
}

JNIEXPORT jint Java_com_cavan_cavanjni_CavanJni_waitpid(JNIEnv *env, jclass clazz, jint pid)
{
	pd_info("waitpid: %d", pid);
	return cavan_exec_waitpid(pid);
}

JNIEXPORT jboolean Java_com_cavan_cavanjni_CavanJni_setEnv(JNIEnv *env, jclass clazz, jstring strKey, jstring strValue)
{
	jboolean success;
	const char *key, *value;

	if (strKey == NULL || strValue == NULL) {
		return false;
	}

	key = (char *) (*env)->GetStringUTFChars(env, strKey, NULL);
	value = (char *) (*env)->GetStringUTFChars(env, strValue, NULL);
	if (key && value) {
		success = (setenv(key, value, 1) == 0);
	} else {
		success = false;
	}

	if (key) {
		(*env)->ReleaseStringUTFChars(env, strKey, key);
	}

	if (value) {
		(*env)->ReleaseStringUTFChars(env, strValue, value);
	}

	return success;
}

JNIEXPORT jboolean Java_com_cavan_cavanjni_CavanJni_symlink(JNIEnv *env, jclass clazz, jstring strTarget, jstring strLinkPath)
{
	jboolean success;
	const char *target, *linkpath;

	if (strTarget == NULL || strLinkPath == NULL) {
		return false;
	}

	target = (char *) (*env)->GetStringUTFChars(env, strTarget, NULL);
	linkpath = (char *) (*env)->GetStringUTFChars(env, strLinkPath, NULL);
	success = (target && linkpath && cavan_symlink(target, linkpath) == 0);

	if (target) {
		(*env)->ReleaseStringUTFChars(env, strTarget, target);
	}

	if (linkpath) {
		(*env)->ReleaseStringUTFChars(env, strLinkPath, linkpath);
	}

	return success;
}
