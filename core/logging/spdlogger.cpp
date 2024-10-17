#include "spdlogger.h"
#include <iostream>
#include <filesystem>
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <utils.h>

using std::cout;


AkCore::SpdLogger& AkCore::SpdLogger::Instance()
{
    static AkCore::SpdLogger instance;
    static bool initialized {false};
    if (!initialized)
    {
        instance.InitLogging();
        initialized = true;
    }
    return instance;
}


void AkCore::SpdLogger::InitLogging()
{
    auto max_size = 1048576 * 5; // 5 MB
    auto max_files = 3;

    try {
        // Initialization code
        
        // Get current date and time
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        //Create log file with current date and time
        std::ostringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), GetCurrentLogFileFormat().c_str());
        std::string dateTime = ss.str();
        currentLog = logFileDirectory() + dateTime + ".log";
       

        // Create rotating file sink
        auto logFileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(currentLog, max_size, max_files);

        // Create console sink
        auto logConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    
        // Vector of sinks
        std::vector<spdlog::sink_ptr> sinks{logFileSink, logConsoleSink};

        // Set pattern and level for each sink
        for(auto sink : sinks)
        {
            sink->set_pattern("%^[%I:%M:%S]%v%$");
        }

        // Create logger
        m_logger = std::make_shared<spdlog::logger>("dm_log", sinks.begin(), sinks.end());

        // Set logging level
        m_logger->set_level(spdlog::level::debug);
        
        // Flush on every message
        m_logger->flush_on(spdlog::level::debug);

        // Register logger so it can be used globally
        spdlog::register_logger(m_logger);

    // Check if logger was successfully initialized
    } 
    catch (const spdlog::spdlog_ex& ex) 
    {
        std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
    }
}

void AkCore::SpdLogger::ShutdownLogging()
{
    m_logger->flush();
    spdlog::shutdown();
}

void AkCore::SpdLogger::LogInfo(const char *message)
{
    cout << message;
    m_logger->info(message);
}

void AkCore::SpdLogger::LogDebug(const char *message)
{
    cout << message;
    m_logger->debug(message);
}

void AkCore::SpdLogger::LogWarning(const char *message)
{
    cout << message;
    m_logger->warn(message);
}

void AkCore::SpdLogger::LogError(const char *message)
{
    cout << message;
    m_logger->error(message);
}

void AkCore::SpdLogger::LogFatal(const char *message)
{
     cout << message;
    m_logger->error(message);
}

std::string AkCore::SpdLogger::GetDateTime()
{
    std::time_t time = std::time({});
    char timeString[std::size("yyyy-mm-dd[hh:mm:ss]")];
    std::strftime(std::data(timeString), std::size(timeString),
                  "%m-%d-%y", std::gmtime(&time));
    
    return timeString;
}

std::string AkCore::SpdLogger::GetLoggerFilePath()
{
   // return m_logger->sinks()[0]->filename();
    return "";
}

std::string AkCore::SpdLogger::GetCurrentLogAsString()
{
    std::filesystem::path filePath(currentLog);
    if (!std::filesystem::exists(filePath) || !std::filesystem::is_regular_file(filePath))
    {
        // Handle file not found or not a regular file
        return ""; // Return an empty string or throw an exception
    }

    std::ifstream file(filePath);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

std::string AkCore::SpdLogger::GetCurrentLogPath()
{
    return currentLog;
}

std::string AkCore::SpdLogger::GetPreviousLogPath()
{
    std::string directory_path = logFileDirectory();
    std::string current_log_path = GetCurrentLogPath(); // Function to get the path of the current log file
    std::vector<std::filesystem::path> log_files;

    // Iterate over the directory
    for (const auto &entry : std::filesystem::directory_iterator(directory_path))
    {
        if (std::filesystem::is_regular_file(entry.path()) && entry.path().extension() == ".log")
        {
            // Exclude the current log file from the list
            if (entry.path().string() != current_log_path)
            {
                log_files.push_back(entry.path());
            }
        }
    }

    // Sort log files based on their last write time
    std::sort(log_files.begin(), log_files.end(), [](const std::filesystem::path &a, const std::filesystem::path &b)
              { return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b); });

    if (log_files.size() >= 1)
    {
        return log_files[0].string(); // Return the most recent log file
    }
    else
    {
        LogInfo("There is no previous log file.");
        return "";
    }
}

std::string AkCore::SpdLogger::GetCurrentLogFileFormat()
{
     return "%Y-%m-%d-%H-%M-%S";
}
