#include <jni.h>
#include <string>

#include "modelAssimp.h"
#include "myJNIHelper.h"

// global pointer is used in JNI calls to reference to same object of type Cube
ModelAssimp *gAssimpObject =NULL;

// global pointer to instance of MyJNIHelper that is used to read from assets
MyJNIHelper * gHelperObject=NULL;

extern "C" JNIEXPORT void JNICALL
Java_com_test_assimpviewandroid_MainActivity_CreateObjectNative(JNIEnv *env, jobject instance, jobject assetManager, jstring pathToInternalDir) {
    gHelperObject = new MyJNIHelper(env, instance, assetManager, pathToInternalDir);
    gAssimpObject = new ModelAssimp();
}