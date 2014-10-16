#include <python2.7/Python.h>

#include <string.h>
#include <jni.h>

#include <android/log.h>
#include <android/sensor.h>
#include <android_native_app_glue.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "PyWrap", __VA_ARGS__))

JNIEnv *global_env;

/* Sensor Extension */
static PyObject *last_event(PyObject *self, PyObject *args)
{

  jclass cls = (*global_env)->FindClass(global_env, "com/pywrapper/PyWrap");
  if (cls == NULL)
  {
    LOGI("Unable to resolve Sensor class");
    Py_RETURN_NONE;
  }
  LOGI("Sensor class resolved");

  jfieldID fid = (*global_env)->GetStaticFieldID(global_env, cls, "globalEvent", "F");

  if (fid == NULL)
  {
    LOGI("Unable to resolve the static field globalEvent");
    Py_RETURN_NONE;
  }
  LOGI("globalEvent field resolved");

  jfloat fl = (*global_env)->GetStaticFloatField(global_env, cls, fid);

  LOGI("globalEvent: %f", fl);

  /*
  jmethodID mid = (*global_env)->GetStaticMethodID(global_env, cls, "callme", "()V");
  if (mid == NULL)
  {
    LOGI("Unable to resolve Java method");
  }

  (*global_env)->CallStaticVoidMethod(global_env, cls, mid);
  */

  Py_RETURN_NONE;
}

static PyMethodDef SensorMethods[] = {
  {"last_event", last_event, METH_VARARGS,  
   "read off the last sensor event"},
  {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initsensors(void)
{
  (void) Py_InitModule("sensors", SensorMethods);
}

/* Android Embed Extension */
static PyObject *androidembed_log(PyObject *self, PyObject *args)
{
  char *logstr;
  if (!PyArg_ParseTuple(args, "s", &logstr))
    return NULL;

  LOGI("%s", logstr);
  Py_RETURN_NONE;
}

static PyMethodDef AndroidEmbedMethods[] = {
    {"log", androidembed_log, METH_VARARGS,
     "Log on android platform (stolen from python-for-android)"},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initandroidembed(void)
{
  (void) Py_InitModule("androidembed", AndroidEmbedMethods);
}

void Java_com_pywrapper_PyWrap_execPyScript(JNIEnv *env, jobject this,
jstring j_script)
{
  jboolean iscopy;
  FILE *fp;
  const char *script = (*env)->GetStringUTFChars(env, j_script, &iscopy);

  /* Make this accessible to our Python Module */
  global_env = env;

  LOGI("Calling Py_Initialize()");
  LOGI("Running script %s", script);
  Py_Initialize();

  initandroidembed();
  initsensors();

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

}
