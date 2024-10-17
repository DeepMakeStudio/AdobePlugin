#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H
#include <rapidjson/document.h>
#include <string>
#include <vector>
#include <map>
#include "ark_plugin.h"

struct ExecuteResponse
{
    std::string job_id;
};

struct BackendConfig 
{
    std::string py_dir;
    std::string startup_dir;
    std::string startup_cmd;
};

typedef enum {
    JOB_STATUS_UNKNOWN,
    JOB_STATUS_IN_PROGRESS,
    JOB_STATUS_SUCCESS,
    JOB_STATUS_NOT_FOUND,
    JOB_STATUS_ERROR
} JobStatus;

struct JobStatusResponse
{
    JobStatus status {JOB_STATUS_UNKNOWN};
    std::string img_id;
};

JobStatus jobStatusFromString(const std::string &status_string);
std::string stringFromJobStatus(JobStatus job_status);


class PluginJsonParser
{
public:
    PluginJsonParser() = default;
    ~PluginJsonParser() = default;

    std::string getTextFromFile(const std::string &file_path) const;
    bool parseBackendConfigFile(const std::string &file_path, struct BackendConfig &config) const;
    bool parsePluginInfo(const std::string &plugin_config_json, struct ArkPlugin &plugin) const;
    bool parseConfigJson(const std::string &plugin_config_json, struct ArkPlugin &plugin) const;
    bool parseLoginStatus(const std::string &response_json) const;
    bool parseUserInfo(const std::string &response_json, std::string &user_info) const;
    bool parseExecuteResponse(const std::string &response_json, struct ExecuteResponse &response) const;
    bool parseJobResponse(const std::string &response_json, struct JobStatusResponse &response) const;
    bool parseUploadImageResponse(const std::string &response_json, std::string &img_id) const;
    bool parseSubscriptionLevel(const std::string &response_json, int& levels) const;
protected:
    bool parseEndpoints(const rapidjson::Document &doc, struct ArkPlugin &plugin) const;
    bool parsePlugin(const rapidjson::Document &doc, struct ArkPlugin &plugin) const;
    bool parseConfigValue(const rapidjson::Value& plugin_config, struct ArkPlugin &plugin) const;
    bool parseConfigDoc(const rapidjson::Document &doc, struct ArkPlugin &plugin) const;


};
#endif // CONFIG_PARSER_H
