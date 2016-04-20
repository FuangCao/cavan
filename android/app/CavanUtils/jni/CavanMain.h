#pragma once

#include <cavan.h>
#include <jni.h>

__BEGIN_DECLS

int CavanMainExecute(JNIEnv *env, jobjectArray args, const char *name, int (*main_func)(int argc, char *argv[]));

__END_DECLS;