#include <python2.7/Python.h>

#include <string.h>
#include <jni.h>

#include <android/log.h>


#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "PyWrap", __VA_ARGS__))

jstring Java_com_pywrapper_PyWrap_stringFromJNI(JNIEnv *env, jobject this)
{
  LOGI("Calling Py_Initialize()");

  Py_Initialize();

  PyRun_SimpleString(
    "f = open('/data/local/tmp/output/iran', 'w')\n"\
    "f.write('hello world')\n"\
    "f.close()\n");

  Py_Finalize();

  return (*env)->NewStringUTF(env, "Shitbrains");

}
