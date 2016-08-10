///
// fsLog.c
//
// Logging functions which can have their level changed at runtime.
//
// Note: Not thread-safe right now, but this isn't being used in threads.
///

#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include "fsLog.h"

///
// Global variable used to filter which messages are printed.
///
int fsCurrentLogLevel = FS_LOG_LEVEL_INFO;

///
// Return a string with the current time.
//
// The string is invalidated on the next call to this function.
///
static char* ctimeStr(void)
{
    static char buffer[26];
    time_t timer;
    struct tm *tmInfo;

    time(&timer);
    tmInfo = localtime(&timer);
    strftime(buffer, sizeof(buffer), "%H:%M:%S", tmInfo);
    return buffer;
}

///
// Return a string representation of a logging level.
///
static const char* logLevelStr(int level)
{
    static const char *logLevelNames[] = {
        "debug", "info", "warning", "error", "fatal"
    };

    return logLevelNames[level];
}

///
// Return the vt100 colour code of this level.
///
static int logLevelColorCode(int level)
{
    static const int logLevelColors[] = {
        39, 37, 33, 31, 31
    };

    return logLevelColors[level];
}

///
// Main logging function.
///
void fsLog(int level, const char *format, ...)
{
    if (level >= fsCurrentLogLevel) {
        fprintf(stderr, "\e[%dm[%s] [%s]:\e[0m ",
                logLevelColorCode(level), ctimeStr(), logLevelStr(level));

        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);

        fprintf(stderr, "\n");
    }
}