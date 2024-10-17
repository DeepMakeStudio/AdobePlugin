#include "utils.h"
#include <filesystem>
#include <cstdio>
#include <iostream>
#include "logger.h"
#include<thread>
#include<chrono>

#ifdef WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif


const std::string& logFileDirectory()
{
    static std::string log_dir_path;
    std::string path; 

    if (log_dir_path.empty())
    {
        #if defined(__APPLE__)
            path = std::getenv("HOME");
            log_dir_path = createPlatformDirectory(path + "/Library/Application Support/DeepMake/Logs/",curHostName,"/");
        #endif

        #if defined(WIN32)
            path = std::getenv("APPDATA");
            std::cout << "path: " << path << std::endl;
            log_dir_path = createPlatformDirectory(path + "\\DeepMake\\Logs\\",curHostName,"\\");
        #endif

        #if defined(__linux__)
            log_dir_path = CreatePlatformDirectory("/var/logs/",curHostName,"/");  
        #endif
    }
    LOG_ASSERT(!log_dir_path.empty(), "Log directory path is empty");
    return log_dir_path;
}

const std::string& tmpDirectory()
{
    static std::string tmp_dir_path;
    if (tmp_dir_path.empty())
    {
        std::filesystem::path tmp_dir = std::filesystem::temp_directory_path();
        if (!std::filesystem::exists(tmp_dir)) {
            std::filesystem::create_directory(tmp_dir);
        }
        tmp_dir_path = tmp_dir.string();
    }
    LOG_ASSERT(!tmp_dir_path.empty(), );
    return tmp_dir_path;
}

void arkSleepMS(std::chrono::seconds ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
void openURL(const std::string url)
{
    #ifdef WIN32
        system(("start " + url).c_str());
    #endif
    #ifdef __linux__
        system(("xdg-open " + url).c_str());
    #endif
    #ifdef __APPLE__
        system(("open " + url).c_str());
    #endif
        LogInfo("URL opened in browser:" + url);
    
}
std::string getHostName()
{
    return curHostName;
}

void setHostName(const std::string &_hostName)
{
    curHostName = _hostName;
}

std::string createPlatformDirectory(const std::string &_dirPath, const std::string &_pathExtension, const char* _pathTerminationSymbol)
{
    if (!std::filesystem::exists(_dirPath)) 
    {
        std::filesystem::create_directory(_dirPath);
    }
    if (!std::filesystem::exists(_dirPath + _pathExtension)) 
    {
        std::filesystem::create_directory(_dirPath + _pathExtension);
    } 

    return _dirPath + _pathExtension + _pathTerminationSymbol;
}

std::string renameFileInPlace(const std::string &_filePath, const std::string &_newFileName)
{
    try
    {
        // Get the parent directory path
        std::filesystem::path parentPath = std::filesystem::path(_filePath).parent_path();

        // Construct the new file path
        std::filesystem::path newFilePath = parentPath / _newFileName;

        // Rename the file
        std::filesystem::rename(_filePath, newFilePath);

        return newFilePath.string();
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return "";
    }
}

std::string getPlatform()
{
    #ifdef _WIN32
        return "Windows";
    #elif __APPLE__
        return "MacOS";
    #elif __linux__
        return "Linux";
    #endif
    LOG_ASSERT(false, "Unknown platform");
    return "Unknown";
}

#ifdef _WIN32
std::string executeCommand(const std::string &cmd)
{
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = true;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE hChildStdoutRd, hChildStdoutWr;
    if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0)) {
        return "CreatePipe failed";
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = hChildStdoutWr;
    si.hStdOutput = hChildStdoutWr;
    si.dwFlags |= STARTF_USESTDHANDLES;

    if (!CreateProcess(NULL, const_cast<char*>(cmd.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        return "CreateProcess failed";
    }

    CloseHandle(hChildStdoutWr);

    std::string result;
    CHAR buffer[4096];
    DWORD bytesRead;

    while (true) {
        if (!ReadFile(hChildStdoutRd, buffer, sizeof(buffer), &bytesRead, NULL) || bytesRead == 0) {
            break;
        }
        result.append(buffer, bytesRead);
    }

    CloseHandle(hChildStdoutRd);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return result;
}
#else
std::string executeCommand(const std::string &cmd) 
{
    FILE* pipe = popen(cmd.c_str(), "r");

    bool success = false;
    char buffer[256];

    if (pipe)
    {
        while (!feof(pipe))
        {
            if (fgets(buffer, 128, pipe) != nullptr)
            {
                success = true;
            }
        }
        #ifdef _WIN32
            _pclose(pipe);
        #else
            pclose(pipe);
        #endif    
    }

    if (!success)
        return "";

    return buffer;
}
#endif
