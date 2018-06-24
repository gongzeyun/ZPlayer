package com.example.bangl.zplayer;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.Surface;

import java.lang.ref.WeakReference;

public class ZPlayer {
    private String mUrl;
    private Surface mSurface;
    private static final String TAG = "ZPlayer";
    private final int START_PLAY = 0;


    private class NotLeakHandler extends Handler{
        private WeakReference<ZPlayer> mWeakReference;
        private NotLeakHandler(ZPlayer reference) {
            Log.d(TAG, "NotLeakHandler construct ");
            mWeakReference = new WeakReference<ZPlayer>(reference);
        }

        @Override
        public void handleMessage(Message msg) {
            
            ZPlayer reference = (ZPlayer) mWeakReference.get();
            if (reference == null) { // the referenced object has been cleared
                return;
            }
            int msg_type = (int) msg.obj;
            switch (msg_type) {
                case START_PLAY:
                    Log.d(TAG, "handleMessage: start play");
                    reference.startPlay(mUrl, mSurface);
                    break;
            }
        }
    }


    NotLeakHandler mHandler;

    /* native method collection */
    public native int startPlay(String url, Surface surface);
    public native int pausePlay();
    public native int resumePlay();
    public native int releasePlay();
    public native int seekTo(int timeUs);


    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avcodec-58");
        System.loadLibrary("avfilter-7");
        System.loadLibrary("avformat-58");
        System.loadLibrary("avutil-56");
        System.loadLibrary( "swresample-3");
        System.loadLibrary("swscale-5");
    }


    public ZPlayer(String url, Surface surface)
    {
        mUrl = url;
        mSurface = surface;
        new Thread(new Runnable() {
        @Override
            public void run() {
                play_thread_loop();
            }
        }).start();
    }

    public void play_thread_loop()
    {
        Looper.prepare();
        mHandler = new NotLeakHandler(this);
        Looper.loop();
    }

    public String GetUrl()
    {
        return mUrl;
    }

    public int Start()
    {
        Log.d(TAG, "start: ");
        Message msg = new Message();
        msg.obj = START_PLAY;
        mHandler.sendMessage(msg);
        return 0;
    }

    public int Pause()
    {
        Log.d(TAG, "pause: ");
        return 0;
    }
}
