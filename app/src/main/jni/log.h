
#ifdef __ANDROID__
#include <android/log.h>
#else
#include <stdio.h>
#endif

#ifdef __ANDROID__
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "native-activity", __VA_ARGS__))
#else
#define LOGI(...) ({fprintf(stdout, "LOGI: ");fprintf(stdout, __VA_ARGS__);fprintf(stdout, "\n");fflush(stdout);})
#define LOGW(...) ({fprintf(stderr, "LOGW: ");fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n");fflush(stderr);})
#define LOGE(...) ({fprintf(stderr, "LOGE: ");fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n");fflush(stderr);})
#endif // !ANDROID
