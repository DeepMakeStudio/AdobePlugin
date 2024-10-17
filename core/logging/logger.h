#ifndef LOGGER_H
#define LOGGER_H


#include <string>
// custom assert with logging for sentry
#define LOG_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            LogError("Assertion failed: " message, true); \
        } \
    } while (0)

//Sentry logging 
void InitSentryLogging();
void ShutdownSentryLogging();

void LogDebug(const char *message, bool bIsSentry = false);
void LogDebug(const std::string &message, bool bIsSentry = false);

void LogInfo(const char *message, bool bIsSentry = false);
void LogInfo(const std::string &message, bool bIsSentry = false);

void LogWarning(const char *message, bool bIsSentry = false);
void LogWarning(const std::string &message, bool bIsSentry = false);

void LogError(const char *message, bool bIsSentry = false);
void LogError(const std::string &message, bool bIsSentry = false);

void LogFatal(const char *message, bool bIsSentry = false);
void LogFatal(const std::string &message, bool bIsSentry = false);

void ReportPrevCrash(const char *message);

void CreateLogEvent(std::string message, std::string errorLevel,bool bIncludesAttachment = false , std::string attachment = "");
void CrashHandler(int signum) ;

void RegisterSignalHandlers();

void UnregisterSignalHandlers();

void SetLogUserData(std::string userName);
#endif
