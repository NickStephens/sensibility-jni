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

ASensorEventQueue *sensorEventQueue;
unsigned sensorEventLimit;
unsigned sensorsEventCount;

int get_sensor_events(int fd, int events, void *data)
{
  ASensorEvent event;

  while(ASensorEventQueue_getEvents(sensorEventQueue, &event, 1)>0)
  {
    if (event.type == ASENSOR_TYPE_ACCELEROMETER) {
          LOGI("accelerometer: x=%f y=%f z=%f", 
              event.acceleration.x, event.acceleration.y,
              event.acceleration.z);
    }
  }

  return 1;
}

void setup_sensors(void)
{
  LOGI("Initializing sensor manager from NDK");
  ALooper *looper = ALooper_forThread();
  LOGI("ALooper_forThread() returned %x", looper);
  if (looper == NULL)
  {
    looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
  }
  LOGI("looper object @ %x", looper);

  ASensorManager *sensorManager = ASensorManager_getInstance();
  LOGI("sensorManager @ %x", sensorManager);

  ASensor *sensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
  LOGI("sensor @ %x", sensor);

  sensorEventQueue = ASensorManager_createEventQueue(sensorManager, looper, LOOPER_ID_USER, get_sensor_events, NULL);
  LOGI("sensorEventQueue @ %x", sensorEventQueue);

  int ret = ASensorEventQueue_enableSensor(sensorEventQueue, sensor);
  LOGI("ASensorEventQueue_enableSensor returned %d", ret);


  LOGI("Entering sensor checking loop");
  while (1) 
  {
    int ident;
    int events;
    struct android_poll_source* source;

    
    LOGI("ident: %d", ident);
    while((ident=ALooper_pollAll(-1, NULL, &events, (void**)&source)) >= 0) 
    {

      LOGI("ALooper_pollAll returned with %d", ident);
      if (ident == LOOPER_ID_USER) {
        ASensorEvent event;

        LOGI("Event processed");
        /*
        while (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0)
        {
          LOGI("accelerometer: x=%f y=%f z=%f", 
              event.acceleration.x, event.acceleration.y,
              event.acceleration.z);
        }
        */
      }
    }
  }

}

void Java_com_pywrapper_PyWrap_execPyScript(JNIEnv *env, jobject this,
jstring j_script)
{
  jboolean iscopy;
  FILE *fp;
  const char *script = (*env)->GetStringUTFChars(env, j_script, &iscopy);

  /* set up our sensor manager */
  setup_sensors();

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

  Py_Finalize();
  LOGI("Python script completed");

}
