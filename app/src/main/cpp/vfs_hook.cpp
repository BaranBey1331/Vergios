#include <jni.h>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <android/log.h>
#include <dlfcn.h>
#include <cstring>

#define LOG_TAG "VFS_CORE"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Orijinal open fonksiyonunun referansını tutacağımız pointer
typedef int (*open_func_t)(const char *pathname, int flags, mode_t mode);
open_func_t original_open = nullptr;

// Kendi yazdığımız, araya giren fake_open fonksiyonu
int fake_open(const char *pathname, int flags, mode_t mode) {
    if (pathname != nullptr) {
        std::string path(pathname);

        // Anti-tamper'ın en çok sevdiği hedeflerden biri: hafıza haritası
        if (path == "/proc/self/maps") {
            LOGI("VFS Intercepted: /proc/self/maps -> Redirecting to clean fake maps.");
            // Spooflanmış, içinde hile/root izi olmayan temiz bir haritaya yönlendir.
            return original_open("/data/data/com.executor.vm/files/fake_maps", flags, mode);
        }
        
        // Emülatör tespiti için QEMU izi kontrolü veya batarya sıcaklığı
        if (path.find("qemu") != std::string::npos || path.find("virtual") != std::string::npos) {
            LOGI("VFS Intercepted: Emulator trace search -> Blocked.");
            // Dosya yokmuş gibi davran (No such file or directory)
            errno = ENOENT;
            return -1;
        }

        // Magisk veya Root yetkisi sorgusu
        if (path == "/system/xbin/su" || path == "/system/bin/su" || path == "/sbin/.magisk") {
            LOGI("VFS Intercepted: Root check -> Masking as unrooted.");
            errno = ENOENT;
            return -1;
        }
    }

    // Tehlikeli bir dosya değilse orijinal fonksiyona pasla
    return original_open(pathname, flags, mode);
}

// Kütüphane hafızaya yüklendiğinde tetiklenen JNI_OnLoad
extern "C" JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("VFS Core Engine Initializing...");

    // Gerçek libc.so'daki open fonksiyonunun bellek adresini al
    void* libc_handle = dlopen("libc.so", RTLD_NOW);
    if (libc_handle) {
        original_open = (open_func_t)dlsym(libc_handle, "open");
        
        if (original_open != nullptr) {
            LOGI("VFS: Successfully resolved original libc.so/open.");
            // NOT: Profesyonel kullanımda burada PLT/GOT Hooking veya Inline Hooking (Dobby) uygulanarak
            // sistemin orijinal 'open' çağrıları zorla 'fake_open'a yönlendirilir.
            // Bu PoC mimarisinde hook hazırlığı yapılmıştır.
        } else {
            LOGE("VFS: Failed to resolve libc.so/open.");
        }
        dlclose(libc_handle);
    }

    return JNI_VERSION_1_6;
}

