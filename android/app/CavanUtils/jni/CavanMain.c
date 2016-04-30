#include "CavanMain.h"

int CavanMainExecute(JNIEnv *env, jobjectArray args, const char *name, int (*main_func)(int argc, char *argv[]))
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
