
#include <jni.h>
#ifndef LIBVERIFY_H_
#define LIBVERIFY_H_

#define VERIFY_SUCCESS (0) //正确返回0
#define VERIFY_FAILED (1) //错误返回1
#define CHECK_MAPS 0
#define CHECK_ENV 0
#define CHECK_SIGN (1)
#define CHECK_FILE (1)

#define EXPORT //static

typedef void (*callback)(JNIEnv *env, int32_t result, void *opaque);
#ifdef __cplusplus
extern "C" {
#endif
EXPORT void start(JNIEnv* env, callback CB, void *opaque);
#ifdef __cplusplus
}
#endif
#endif

