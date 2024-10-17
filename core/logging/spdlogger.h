#ifndef LOGGER_H_
#define LOGGER_H_

#include <string>
#include "spdlog/spdlog.h"

namespace AkCore {


enum LOGGER_LEVEL {
	DEBUG,
	INFO,
	WARN,
	ERR,
	NUM_LEVELS
};

class SpdLogger
{
    public:
        static SpdLogger& Instance();
        void InitLogging();
        void ShutdownLogging();

        void LogDebug(const char *message);
        void LogInfo(const char *message);
        void LogWarning(const char *message);
        void LogError(const char *message);
        void LogFatal(const char *message);
        std::string GetDateTime();
        std::string GetLoggerFilePath();
        std::string GetCurrentLogAsString();
        std::string GetCurrentLogPath();
        std::string GetPreviousLogPath();
        
        std::string GetCurrentLogFileFormat();
    private:
	    std::shared_ptr<spdlog::logger> m_logger;
        std::string currentLog;
};

}
#endif