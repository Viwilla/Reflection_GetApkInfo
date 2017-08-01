/*
 * ApkUtils.h
 *
 *  Created on: Aug 8, 2013
 *      Author: jevey
 */

#ifndef APKUTILS_H_
#define APKUTILS_H_
#include "jni_common.h"
#include <jni.h>
#ifdef GAME
#define DEX_ENC_FILE "assets/ig.dat"
#define METHOD_CODE_FILE "assets/ig.ajm"
#else
#define DEX_ENC_FILE "assets/ijiami.dat"
#define METHOD_CODE_FILE "assets/ijiami.ajm"
#endif

typedef struct ApkUtils{
	bool (*getEncryptDexData)(const char *pApkPath,unsigned char **ppdata, size_t *unsize) ;
	bool (*getDexMethodData)(const char *pApkPath,unsigned char **ppdata, size_t *unsize) ;
	bool (*getZipEntry)(const char *pApkPath,const char * zipentry,unsigned char **ppdata, size_t *unsize);
	bool (*getAjmEntry)(const char *zipBuf,size_t size, const char * zipentry,unsigned char **ppdata, size_t *unsize);
	bool (*closeZip)();
	bool (*getZipEntryOnce)(const char *pApkPath, const char * zipentry,
			unsigned char **ppdata, size_t *unsize);
	bool (*getAssetsEntryOnce)(JNIEnv * env,jobject am,const char *pApkPath, const char * zipentry,
			unsigned char **ppdata, size_t *unsize);
	bool (*saveAssetsEntry)(JNIEnv * env,jobject am,const char *savaPath, const char * zipentry);
//	int (*addDataZip)(const char *pZipFilePath, const char *pAddPathNameInZip, const char *pInFile, int iDataLen,unsigned char bNewZip);
	bool (*writeToFile)(char *path, char *data, size_t size);
}ApkUtils;

#ifdef __cplusplus
extern "C" {
#endif
__attribute__ ((visibility ("default")))
ApkUtils* ApkUtils_func();
#ifdef __cplusplus
}
#endif

#endif /* APKUTILS_H_ */
