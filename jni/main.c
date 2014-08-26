#include <python2.7/Python.h>

#include <string.h>
#include <jni.h>

#include <android/log.h>


#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "PyWrap", __VA_ARGS__))

jstring Java_com_pywrapper_PyWrap_stringFromJNI(JNIEnv *env, jobject this,
jstring j_script)
{
  jboolean iscopy;
  FILE *fp;
  const char *script = (*env)->GetStringUTFChars(env, j_script, &iscopy);

  LOGI("Calling Py_Initialize()");
  LOGI("Running script %s", script);
  Py_Initialize();

  fp = fopen(script, "r");
  if (fp == NULL)
  {
    LOGI("Failed to open script");
  }

  LOGI("Return code: %d", PyRun_SimpleFile(fp, script));
  fclose(fp);

  if (PyErr_Occurred() != NULL)
  {
    LOGI("A python error occured!");
    PyErr_Print();
    if (Py_FlushLine())
      PyErr_Clear();
  }
  /*
  PyRun_SimpleString(
    "f = open('/data/local/tmp/output/iran', 'w')\n"\
    "f.write('hello world')\n"\
    "f.close()\n");
  */

  Py_Finalize();
  LOGI("Python script completed");

  return (*env)->NewStringUTF(env, script);

}
