#ifndef CORE_UTILS_H
#define CORE_UTILS_H

#include <string>
#include <video_host.h>
#include <chrono>

#define UNUSED(x) (void)(x)



const std::string& logFileDirectory();
const std::string& tmpDirectory();
void arkSleepMS(std::chrono::seconds ms);

void openURL(const std::string url);
static std::string curHostName = "";
std::string getHostName();
void setHostName(const std::string &_hostName);
std::string createPlatformDirectory(const std::string &_dirPath, const std::string &_pathExtension, const char* _pathTerminationSymbol);
std::string renameFileInPlace(const std::string &_filePath, const std::string &_newFileName);
std::string getPlatform();
std::string executeCommand(const std::string &cmd);
#endif
