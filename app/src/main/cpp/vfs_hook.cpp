#include <jni.h>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <android/log.h>
#include <dlfcn.h>
#include <cstring>
#include "dobby.h" // FetchContent ile indirdiğimiz Dobby başlığı

#define LOG_TAG "VFS_CORE"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Orijinal open fonksiyonunun adresini tutacak pointer (Dobby bunu dolduracak)
typedef int (*open_func_t)(const char *pathname, int flags, mode_t mode);
open_func_t original_open = nullptr;

// Orijinal open çağrıldığında araya girecek ve işlemi devralacak fonksiyonumuz
int fake_open(const char *pathname, int flags, mode_t mode) {
    if (pathname != nullptr) {
        std::string path(pathname);

        // Hafıza haritası kontrolü (Hyperion vs.)
        if (path == "/proc/self/maps") {
            LOGI("VFS Intercepted: /proc/self/maps -> Redirecting to clean fake maps.");
            return original_open("/data/data/com.executor.vm/files/fake_maps", flags, mode);
        }
        
        // Emülatör/VM tespiti
        if (path.find("qemu") != std::string::npos || path.find("virtual") != std::string::npos) {
            LOGI("VFS Intercepted: Emulator trace search -> Blocked.");
            errno = ENOENT;
            return -1;
        }

        // Root ve Magisk tespiti
        if (path == "/system/xbin/su" || path == "/system/bin/su" || path == "/sbin/.magisk") {
            LOGI("VFS Intercepted: Root check -> Masking as unrooted.");
            errno = ENOENT;
            return -1;
        }
    }

    // Listede yoksa orijinal fonksiyona devam et, kimse bir şey anlamasın
    return original_open(pathname, flags, mode);
}

// JNI_OnLoad, sistem vfscore.so'yu hafızaya aldığında otomatik çalışır
extern "C" JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("VFS Core Engine Initializing with Dobby Hooking...");

    // libc içindeki 'open' fonksiyonunun bellek adresini buluyoruz
    void* open_ptr = dlsym(RTLD_DEFAULT, "open");
    
    if (open_ptr != nullptr) {
        LOGI("VFS: Found 'open' at memory address: %p", open_ptr);
        
        // DobbyHook ile Inline Hooking işlemi
        // 1. Parametre: Hedef fonksiyon adresi
        // 2. Parametre: Kendi yazdığımız fonksiyon
        // 3. Parametre: Orijinal fonksiyona dönebilmek için Dobby'nin hazırladığı trampoline adresi
        int hook_status = DobbyHook(open_ptr, (void *)fake_open, (void **)&original_open);
        
        if (hook_status == 0) { // Dobby başarısı genelde 0'dır (veya RS_SUCCESS)
            LOGI("VFS: Successfully hooked 'open' via Dobby. We are in the matrix.");
        } else {
            LOGE("VFS: DobbyHook failed. Hook status code: %d", hook_status);
        }
    } else {
        LOGE("VFS: Failed to locate 'open' in memory (dlsym returned null).");
    }

    return JNI_VERSION_1_6;
}
