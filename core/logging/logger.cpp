#include "logger.h"
#include "spdlogger.h"
#include <signal.h>
#include "../video_plugin_version.h"
#include <sentry.h>
#include <filesystem>
#include <utils.h>


static sentry_options_t* options = nullptr;
static std::string userNameCached = "jane.doe@example.com"; // needs to be hooked up with login/UI branch
void InitSentryLogging()
{
    options = sentry_options_new();
    sentry_options_set_dsn(options, "https://599b7ea4d4d144eebfb73a067d6a865b@o4506430643175424.ingest.sentry.io/4506463203622912");
    sentry_options_set_database_path(options, ".sentry-native");
    sentry_options_set_release(options, "DeepMake " kVideoPluginVersionString);
    sentry_options_set_debug(options, 1);
    sentry_options_set_system_crash_reporter_enabled(options, true);
   // sentry_options_set_handler_path(options, "C:\\Program Files\\Adobe\\Adobe After Effects 2024\\Support Files\\crashpad_handler.exe");
    sentry_options_set_auto_session_tracking(options, true);
    sentry_options_set_symbolize_stacktraces(options, true);
    sentry_options_set_environment(options, "production");
    sentry_options_set_max_breadcrumbs(options, 50);
    //RegisterSignalHandlers();
    sentry_value_t user = sentry_value_new_object();
    sentry_value_set_by_key(user, "User", sentry_value_new_string(userNameCached.c_str()));
    sentry_set_user(user);
    sentry_clear_crashed_last_run();
    sentry_init(options);
}

void ShutdownSentryLogging()
{
   // UnregisterSignalHandlers();
    sentry_shutdown();
}

void LogDebug(const char *message, bool bIsSentry)
{
    std::cout << "DEBUG:" << message;
    std::string msg = "[DEBUG]:" + std::string(message);
    AkCore::SpdLogger::Instance().LogDebug(msg.c_str());
    if(bIsSentry)
    {
        CreateLogEvent(message, "DEBUG");
    }
}

void LogDebug(const std::string &message, bool bIsSentry)
{
   LogDebug(message.c_str());
}

void LogInfo(const char *message, bool bIsSentry)
{   
    std::cout << "INFO:" << message;
    std::string msg = "[INFO]:" + std::string(message);
    AkCore::SpdLogger::Instance().LogInfo(msg.c_str());
    if(bIsSentry)
    {
         CreateLogEvent(message, "INFO");
    }
}

void LogInfo(const std::string &message, bool bIsSentry)
{
    LogInfo(message.c_str());
}

void LogWarning(const char *message, bool bIsSentry)
{
    std::cout << "WARNING:" << message;
    std::string msg = "[WARNING]:" + std::string(message);
    AkCore::SpdLogger::Instance().LogWarning(msg.c_str());
    if(bIsSentry)
    {
         CreateLogEvent(message, "WARNING");
    }
}

void LogWarning(const std::string &message, bool bIsSentry)
{
    LogWarning(message.c_str());
}

void LogError(const char *message, bool bIsSentry)
{
    std::cout << "ERROR:" << message;
    std::string msg = "[ERROR]:" + std::string(message);
    AkCore::SpdLogger::Instance().LogError(msg.c_str());
    if(bIsSentry)
    {
        CreateLogEvent(message, "ERROR", true);
    }
}

void LogError(const std::string &message, bool bIsSentry)
{
    LogError(message.c_str());
}

void LogFatal(const char *message, bool bIsSentry)
{
    std::cout << "[FATAL]:" << message;
    std::string msg = "[FATAL]:" + std::string(message);
    AkCore::SpdLogger::Instance().LogFatal(msg.c_str());

    if (bIsSentry)
    {
        CreateLogEvent(message, "FATAL", true, AkCore::SpdLogger::Instance().GetCurrentLogPath());
    }
}

void LogFatal(const std::string &message, bool bIsSentry)
{
    LogFatal(message.c_str(), bIsSentry);
}

void ReportPrevCrash(const char *message)
{
    std::cout << "[FATAL]:" << message;
    std::string msg = "[FATAL]:" + std::string(message);
    AkCore::SpdLogger::Instance().LogFatal(msg.c_str());
    std::string attachment = AkCore::SpdLogger::Instance().GetPreviousLogPath();
    LogInfo("Crash detected in previous session, attaching previous sessions log file: " + attachment);
    CreateLogEvent(message, "FATAL", true, attachment);
}

void CreateLogEvent(std::string message, std::string errorLevel, bool bIncludesAttachment,std::string attachment)
{
    sentry_value_t event = sentry_value_new_event();
    sentry_value_set_by_key(event, "message", sentry_value_new_string(message.c_str()));
    sentry_value_set_by_key(event, "logger", sentry_value_new_string("DeepMake"));
    sentry_value_set_by_key(event, "platform", sentry_value_new_string(getPlatform().c_str()));
    sentry_value_set_by_key(event, "release", sentry_value_new_string(kVideoPluginVersionString));
    sentry_set_tag("Host",  getHostName().c_str());
    sentry_set_tag("user", userNameCached.c_str());
    sentry_level_t level;
    if (errorLevel == "DEBUG")
        level = SENTRY_LEVEL_DEBUG;
    else if (errorLevel == "INFO")
        level = SENTRY_LEVEL_INFO;
    else if (errorLevel == "WARNING")
        level = SENTRY_LEVEL_WARNING;
    else if (errorLevel == "ERROR")
        level = SENTRY_LEVEL_ERROR;
    else if (errorLevel == "FATAL")
        level = SENTRY_LEVEL_FATAL;
    else
        level = SENTRY_LEVEL_DEBUG;
    sentry_set_level(level);

    
    if (bIncludesAttachment && options && std::filesystem::exists(attachment))
    {
        LogInfo("Attaching log file: " + attachment);
        sentry_options_add_attachment(options, attachment.c_str());
    }
    sentry_capture_event(event);
}

void CrashHandler(int signum)
{
    sentry_value_t event = sentry_value_new_event();
    sentry_value_t exc = sentry_value_new_exception("Exception", "Unhandled Exception: Fatal Error, Application Crashed!");
    CreateLogEvent("Unhandled Exception: Fatal Error, Application Crashed!", "FATAL", true);
    sentry_event_add_exception(event, exc);
    ShutdownSentryLogging();
}

void RegisterSignalHandlers()
{
    signal(SIGSEGV, CrashHandler);//might overwrite the default handler and cause issues needs testing 
    signal(SIGABRT, CrashHandler);
}

void UnregisterSignalHandlers() 
{
    signal(SIGSEGV, SIG_DFL); 
    signal(SIGABRT, SIG_DFL);
}

void SetLogUserData(std::string userName)
{
    userName = userNameCached;
}
