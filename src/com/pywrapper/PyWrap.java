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

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        Log.v(TAG, "Starting up...");

        TextView tv = new TextView(this);
        execPyScript("/data/local/tmp/test.py");
        tv.setText("Executed");
        setContentView(tv);

    }

    public native void execPyScript(String script);

    static {
      System.loadLibrary("python2.7");
      Log.v(TAG, "libpython2.7.so loaded...");
      System.loadLibrary("application");
      Log.v(TAG, "libapplication.so loaded...");
    }
}
