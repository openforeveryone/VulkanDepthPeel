#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <android/log.h>

static int stdoutpfd[2];
static int stderrpfd[2];
static pthread_t stdoutthr;
static pthread_t stderrthr;

static void *stdout_log_thread_main(void*)
{
    ssize_t bytesRead;
    char buf[128];
    while((bytesRead = read(stdoutpfd[0], buf, sizeof buf - 1)) > 0)
    {
        buf[bytesRead] = '\n';
        __android_log_write(ANDROID_LOG_DEBUG, "native-activity-stdout", buf);
    }
    return 0;
}

static void *stderr_log_thread_main(void*)
{
    ssize_t bytesRead;
    char buf[128];
    while((bytesRead = read(stderrpfd[0], buf, sizeof buf - 1)) > 0)
    {
        buf[bytesRead] = '\n';
        __android_log_write(ANDROID_LOG_ERROR, "native-activity-stderr", buf);
    }
    return 0;
}

void redirectStdIO()
{
    setvbuf(stdout, 0, _IOLBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);

    pipe(stdoutpfd);
    pipe(stderrpfd);
    dup2(stdoutpfd[1], STDOUT_FILENO);
    dup2(stderrpfd[1], STDERR_FILENO);

    if(pthread_create(&stdoutthr, 0, stdout_log_thread_main, 0) == -1)
        return;
    pthread_detach(stdoutthr);
    printf("Stdout Redirected");

    if(pthread_create(&stderrthr, 0, stderr_log_thread_main, 0) == -1)
        return;
    pthread_detach(stderrthr);
    fprintf(stderr, "Stderr Redirected");
}