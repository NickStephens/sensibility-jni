package com.pywrapper;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.content.Context;
import android.content.Intent;
import android.app.Activity;
import android.widget.TextView;
import android.os.Bundle;
import android.util.Log;

public class PyWrap extends Activity
{
    public static String TAG = "PyWrap";
    public static float globalEvent;
    private static SensorManager sensorService = null;
    private Sensor sensor;

    private SensorEventListener mySensorEventListener = new SensorEventListener () {

      @Override
      public void onAccuracyChanged(Sensor sensor, int accuracy)
      {

      }

      @Override
      public void onSensorChanged(SensorEvent event)
      {
        globalEvent = event.values[0];
        Log.v(TAG, "Java reporting:" + globalEvent);
        /* uncomment these for a torrent of sensor data in logcat
        Log.v(TAG, "x:" + Float.toString(event.values[0])); 
        Log.v(TAG, "y:" + Float.toString(event.values[1])); 
        Log.v(TAG, "z:" + Float.toString(event.values[2])); 
        */
      }
    };

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        //Hardware.context = (Context) this;

        Log.v(TAG, "Starting up...");
        Log.v(TAG, "Initializing Sensor Manager...");
        sensorService = (SensorManager) getSystemService(SENSOR_SERVICE);
        sensor = sensorService.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        if (sensor != null)
        {
          sensorService.registerListener(mySensorEventListener, sensor,
            SensorManager.SENSOR_DELAY_NORMAL);
          Log.v(TAG, "Registered accelerometer listener");
        }


        TextView tv = new TextView(this);
        execPyScript("/data/local/tmp/test.py");
        tv.setText("Executed");
        setContentView(tv);

        /* 
        Hardware.accelerometerEnable(true);
        while (true)
        {
          if (Hardware.lastEvent != null)  
            Log.v(TAG, Hardware.lastEvent.toString());
          else
            Log.v(TAG, "No data to report");  
          try {
            Thread.sleep(1000);
          } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
          }
        }
        */
    }

    public native void execPyScript(String script);

    static {
      System.loadLibrary("python2.7");
      Log.v(TAG, "libpython2.7.so loaded...");
      System.loadLibrary("application");
      Log.v(TAG, "libapplication.so loaded...");
    }
}
