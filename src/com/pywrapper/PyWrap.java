package com.pywrapper;

import android.app.Activity;
import android.widget.TextView;
import android.os.Bundle;
import android.util.Log;

public class PyWrap extends Activity
{
    private static String TAG = "PyWrap";

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        Log.v(TAG, "Starting up...");
        TextView tv = new TextView(this);
        tv.setText(stringFromJNI("/data/local/tmp/hello.py"));
        setContentView(tv);
    }

    public native String stringFromJNI(String script);

    static {
      System.loadLibrary("python2.7");
      Log.v(TAG, "libpython2.7.so loaded...");
      System.loadLibrary("application");
      Log.v(TAG, "libapplication.so loaded...");
    }
}
