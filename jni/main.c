#include <python2.7/Python.h>

#include <string.h>
#include <jni.h>

#include <android/log.h>
#include <android/sensor.h>
#include <android_native_app_glue.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "PyWrap", __VA_ARGS__))

/* globally accessible java enviornment */
JNIEnv *global_env;

/* globals for sensors */
ASensorEventQueue *sensorEventQueue;
ASensorManager *sensorManager;
ASensor *sensor;

ASensorEvent event;

unsigned sensorEventLimit = 0;
unsigned sensorEventCount = 0;

/* Sensor Extension */
static PyObject *poll_event(PyObject *self, PyObject *args)
{
  
  LOGI("[last_event] calling ALooper_pollOnce");
  /* blocks until a new event arrived */
  ALooper_pollOnce(-1, NULL, NULL, NULL);

  return Py_BuildValue("[f,f,f]", event.acceleration.x, event.acceleration.y,
    event.acceleration.z);
}

static PyMethodDef SensorMethods[] = {
  {"poll_event", poll_event, METH_VARARGS,  
   "polls for a sensor event, blocks until one occurs."},
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

int get_sensor_events(int fd, int events, void *data)
{

  while(ASensorEventQueue_getEvents(sensorEventQueue, &event, 1)>0)
  {
    if (event.type == ASENSOR_TYPE_ACCELEROMETER) {
          LOGI("accelerometer: %d x=%f y=%f z=%f", 
              sensorEventCount, event.acceleration.x, event.acceleration.y,
              event.acceleration.z);
    }
  }
    
  return 1;
}

void destroy_sensors(void)
{
  LOGI("Destroying sensor manager from NDK");
  int ret = ASensorEventQueue_disableSensor(sensorEventQueue, sensor);
  LOGI("ASensorEventQueue_disableSensor returned %d", ret);

  ret = ASensorManager_destroyEventQueue(sensorManager, sensorEventQueue);
  LOGI("ASensorManager_destroyEventQueue returned %d", ret);
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

  sensorManager = ASensorManager_getInstance();
  LOGI("sensorManager @ %x", sensorManager);

  sensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
  LOGI("sensor @ %x", sensor);

  sensorEventQueue = ASensorManager_createEventQueue(sensorManager, looper, LOOPER_ID_USER, get_sensor_events, NULL);
  LOGI("sensorEventQueue @ %x", sensorEventQueue);

  int ret = ASensorEventQueue_enableSensor(sensorEventQueue, sensor);
  LOGI("ASensorEventQueue_enableSensor returned %d", ret);

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

  destroy_sensors();

}
