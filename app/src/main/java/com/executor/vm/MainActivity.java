package com.executor.vm;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

public class MainActivity extends Activity {

    private static final String TAG = "ExecutorVM";

    // C++ VFS motorumuzu (vfscore.so) uygulamaya yüklüyoruz.
    static {
        System.loadLibrary("vfscore");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        TextView textView = new TextView(this);
        textView.setText("EVM Host Motoru Çalışıyor.\nVFS (Sanal Dosya Sistemi) Aktif.");
        textView.setTextSize(20f);
        setContentView(textView);

        Log.i(TAG, "MainActivity initialized. Setting up spoofed environment...");
        
        // VFS'in yönlendireceği sahte dosyaları oluşturma aşaması
        setupFakeEnvironment();
    }

    private void setupFakeEnvironment() {
        try {
            File fakeMapsFile = new File(getFilesDir(), "fake_maps");
            if (!fakeMapsFile.exists()) {
                FileOutputStream fos = new FileOutputStream(fakeMapsFile);
                // Burada tertemiz, hilesiz bir Android 10 hafıza haritası stringi yazılır.
                String cleanMapsData = "00400000-00409000 r-xp 00000000 103:02 1234 /system/bin/app_process64\n" +
                                       "00609000-0060a000 r--p 00009000 103:02 1234 /system/bin/app_process64\n" +
                                       "0060a000-0060b000 rw-p 0000a000 103:02 1234 /system/bin/app_process64\n";
                fos.write(cleanMapsData.getBytes());
                fos.close();
                Log.i(TAG, "Fake /proc/self/maps generated successfully.");
            }
        } catch (IOException e) {
            Log.e(TAG, "Failed to setup fake environment: " + e.getMessage());
        }
    }
}

