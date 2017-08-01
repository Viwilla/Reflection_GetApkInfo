#include <stdio.h>
#include <jni.h>
#include <android/log.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "invoke.h"
#include "apkutils.h"
#include "jni_common.h"
#include <assert.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <pthread.h>
#include "start.h"

/**调用回调函数**/
static int callfunc()
{
    int32_t result = 0;
    if(res1&&res2&&res3&&res4){
        result = VERIFY_SUCCESS;
    }else{
        result = VERIFY_FAILED;
    }
    if (CBfunc != NULL){
        CBfunc(envglobal, result, opaque);
        return 0;
    }else{
        LOGD("NO CALLBACK");
        return 1;
    }
}

/**创建线程**/
static int createThread(void *(*fn)(void *), void *arg) {
	int err;
	pthread_t tid;
	pthread_attr_t attr ;
	err = pthread_attr_init(&attr);
	if (err != 0)
		return err;
	err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (err == 0) {
		err = pthread_create(&tid, &attr, fn, arg);
	}
	pthread_join(tid,NULL);
	pthread_attr_destroy(&attr);
	return err;
}


/**sub_233获取jApplication**/
static jobject Get_jApplication(JNIEnv *env) {
    jobject jCurrentActivityThread = NULL;
	int okey = 0;
	okey = invoke_func()->callStaticObjectMethod(env, &jCurrentActivityThread,
			"android/app/ActivityThread", "()Landroid/app/ActivityThread;",
			"currentActivityThread");
	if (!okey || !jCurrentActivityThread) {
		LOGE("getCurrentActivityThread null！");
		return 0;
	}

    jobject jApplication = NULL;
	invoke_func()->callObjectMethod(env, &jApplication,
			"android/app/ActivityThread", jCurrentActivityThread,
			"()Landroid/app/Application;", "getApplication");

	if (!jApplication) {
		LOGE("jApplication NULL");
		return 0;
	}
	return jApplication;
}

/**获取包信息**/
static jobject Get_jPackinfo(JNIEnv *env){
    jobject jcontext = Get_jApplication(env);
    jobject jPackageManager = NULL;
	invoke_func()->callObjectMethod(env, &jPackageManager,
			"android/content/Context", jcontext,
			"()Landroid/content/pm/PackageManager;", "getPackageManager");
	if (!jPackageManager) {
		LOGE("jGetPackageManager NULL");
		return 0;
	}

    jobject jPackageName = NULL;
	invoke_func()->callObjectMethod(env, &jPackageName,
			"android/content/Context", jcontext,
			"()Ljava/lang/String;", "getPackageName");
	if (!jPackageName) {
		LOGE("jgetPackageName NULL");
		return 0;
	}

    /**获取包的正版hashcode，将jPackageName赋值为需要获取签名的包名即可**/
	//jstring pg1 = invoke_func()->tojstring(env,"com.meelive.ingkee");
	//jstring pg2 = invoke_func()->tojstring(env,"com.netease.mail.oneduobaohydrid");
    jobject jPackageInfo = NULL;
	invoke_func()->callObjectMethod(env, &jPackageInfo,
			"android/content/pm/PackageManager", jPackageManager,
			"(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;", "getPackageInfo",
			jPackageName,64);

    if (jcontext != NULL){
        env->DeleteLocalRef(jcontext);
	}

    if (jPackageManager != NULL){
        env->DeleteLocalRef(jPackageManager);
	}
	if (!jPackageInfo) {
		LOGE("jgetPackageInfo NULL");
		return 0;
	}
	return jPackageInfo;

}

/**获取file目录**/
static const char* getFilepath(JNIEnv* env){
    jobject jApplication = Get_jApplication(env) ;
    //get filepath
    jobject jFile = NULL;
	invoke_func()->callObjectMethod(env, &jFile, "android/content/Context",
			jApplication, "()Ljava/io/File;", "getFilesDir");
	if (!jFile) {
		LOGE("jFile NULL");
		return 0;
    }

	jobject jPath = NULL;
	invoke_func()->callObjectMethod(env, &jPath, "java/io/File", jFile,
			"()Ljava/lang/String;", "getPath");
	if (!jPath) {
		LOGE("jPath NULL");
		return 0;
	}
	const char* mPath = env->GetStringUTFChars((jstring) jPath, NULL);
    if (jApplication != NULL){
        env->DeleteLocalRef(jApplication);
	}
	//cFilepath = (char*)malloc((strlen(mPath)+1)*sizeof(char));
	memcpy(cFilepath, mPath, LINE_MAX);
	env->ReleaseStringUTFChars((jstring) jPath, mPath);
	return (const char*)cFilepath;
}

/**读取assert目录下的文件信息**/
static int getAssertFileInfo(JNIEnv* env,char* assetfile){
    jstring filename  = env->NewStringUTF(assetfile);
    jobject jApplication = Get_jApplication(env) ;
    //get filepath
    jobject jAsserts = NULL;
	invoke_func()->callObjectMethod(env, &jAsserts, "android/content/Context",
			jApplication, "()Landroid/content/res/AssetManager;", "getAssets");
	if (!jAsserts) {
		LOGE("jAsserts NULL");
		return 0;
	}

	jobject jAssetmanager = NULL;
	invoke_func()->callObjectMethod(env, &jAssetmanager,
			"android/content/ContextWrapper", jApplication,
			"()Landroid/content/res/AssetManager;", "getAssets");
	if (!jAssetmanager) {
		LOGE("jAssetmanager NULL");
		return 0;
	}

	//LOGI("ReadAssets");
    AAssetManager* mgr = AAssetManager_fromJava(env, jAssetmanager);
    if(mgr==NULL)
    {
      LOGE(" %s","AAssetManager==NULL");
      return 0;
    }

    /*获取文件名并打开*/
    jboolean iscopy;
    const char *mfile = env->GetStringUTFChars(filename, &iscopy);
    AAsset* asset = AAssetManager_open(mgr, mfile,AASSET_MODE_UNKNOWN);
    env->ReleaseStringUTFChars(filename, mfile);
    if(asset==NULL)
    {
      //LOGI(" %s","asset==NULL");
      return 0;
    }
    /*获取文件大小*/
    off_t bufferSize = AAsset_getLength(asset);
    char* buffer=(char *)malloc(bufferSize+1);
    buffer[bufferSize]=0;
    int numBytesRead = AAsset_read(asset, buffer, bufferSize);
    /**dosomething**/
    /*关闭文件*/
    free(buffer);
    Asset_close(asset);
    return 1;
}


/**sub_c 签名校验**/
static int CheckSign(JNIEnv *env) {
    int flaguser = 0, flagadmin = 0;
    jobject jPackageInfo = Get_jPackinfo(env);
    jobject jsignatures = NULL;
	invoke_func()->getObject(env, &jsignatures,
			"android/content/pm/PackageInfo", jPackageInfo,
            "signatures","[Landroid/content/pm/Signature;");
	if (!jsignatures) {
		LOGE("jsignatures NULL");
		return 0;
	}
	jobjectArray signatures = reinterpret_cast<jobjectArray>(jsignatures);
	//get sign
	jobject signature = env->GetObjectArrayElement(signatures, 0);
	jclass signatureClazz = env->GetObjectClass(signature);
	jmethodID toCharString = env->GetMethodID(signatureClazz, "toCharsString",
			"()Ljava/lang/String;");

	//signature toCharsString
	jstring signCharString = static_cast<jstring>(env->CallObjectMethod(
			signature, toCharString));

	// get signature hash code
	jclass stringClass = env->GetObjectClass(signCharString);
	jmethodID jhashCode = env->GetMethodID(stringClass, "hashCode", "()I");
	int hash_code = env->CallIntMethod(signCharString, jhashCode);

	//LOGI("the signtures is :%d", hash_code);

	if (signatureClazz != NULL){
	    env->DeleteLocalRef(signatureClazz);
	}

	if (jPackageInfo != NULL){
        env->DeleteLocalRef(jPackageInfo);
	}
	if (jsignatures != NULL){
        env->DeleteLocalRef(jsignatures);
	}

	if (hash_code - CHECKSIGNUSER + OFFSET != 0) {
		//LOGI("the CHECKSIGNUSER is :%d", CHECKSIGNUSER + OFFSET);
		flaguser = 1;
	}
    if (hash_code - CHECKSIGNADMIN + OFFSET != 0) {
		//LOGI("the CHECKSIGNADMIN is :%d", CHECKSIGNADMIN + OFFSET);
		flagadmin = 1;
	}

	if(flaguser&&flagadmin){
        LOGE("SIG ERROR!");
        return 0;
        //exit(-1);
	}
	return  (flaguser&&flagadmin)? 0 : 1;
}

/**jContextWrapper_getApkPath**/
static jobject GetApkPath(JNIEnv *env) {
    jobject source_dir = NULL;
	jobject applicationinfo = NULL;
	bool okey = false;
    jobject jApplication = Get_jApplication(env) ;

    //get applicationinfo
	invoke_func()->callObjectMethod(env, &applicationinfo, "android/content/Context",
			jApplication, "()Landroid/content/pm/ApplicationInfo;", "getApplicationInfo");

	if (!applicationinfo) {
		LOGE("jContextWrapper_getApkPath application_info_object is null");
		return NULL;
	}

	okey = invoke_func()->getObject(env, &source_dir, "android/content/pm/ApplicationInfo",
			applicationinfo, "sourceDir", "Ljava/lang/String;");
	if (!source_dir) {
		LOGE("jContextWrapper_getApkPath is null");
		return NULL;
	}
	return source_dir;
}

/**DEX文件校验**/
static void* Check(void* arg){
    /**..do something.**/

//调用回调函数处理
_end:
    if(callfunc()==0){
        LOGD("callback success");
    }else{
        if (res1&&res2&&res3&&res4){
            return NULL;
        }
        else{
            LOGD("NO CALLBACK EXIT");
            exit(-1);
        }
    }
    return NULL;
}

void do_check(JNIEnv *env){
    Create_thread(&Check, NULL);
    return ;
}

void start(JNIEnv* env, callback CB, void *oq){

    if(CONFIG_OPEN){
        do_check(env);
    }
}

//test linker bug
/**
__attribute__((constructor)) void testlinker(){
    LOGD("dexcount init:%d",sub_cxd5);
    return;
}**/


