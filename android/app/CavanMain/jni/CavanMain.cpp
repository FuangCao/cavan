#include "CavanMain.h"

int CavanMainExecute(JNIEnv *env, jobjectArray args, const char *name, int (*main_func)(int argc, char *argv[]))
{
	int ret;
	int length = env->GetArrayLength(args);
	char *argv[length + 2];
	jstring strArgs[length];

	argv[0] = (char *) name;
	argv[length + 1] = NULL;

	for (int i = 0; i < length; i++) {
		strArgs[i] = (jstring) env->GetObjectArrayElement(args, i);
		argv[i + 1] = (char *) env->GetStringUTFChars(strArgs[i], NULL);
	}

	ret = main_func(length + 1, argv);

	for (int i = 0; i< length; i++) {
		env->ReleaseStringUTFChars(strArgs[i], argv[i + 1]);
	}

	return ret;
}
