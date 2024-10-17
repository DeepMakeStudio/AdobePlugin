#include "main_api_connection.h"
#include "logger.h"
#include <rapidjson/document.h>
#include <cpr/cpr.h>
#include "images/image_utils.h"
#include <filesystem>
#include "utils.h"

bool ApiConnection::isBackendRunning() const
{
    bool success = false;

    cpr::Response response = cpr::Get(cpr::Url{m_base_url + "plugin/status/"});
    if(response.status_code == 200)
    {
        success = true;
    }
    return success;
}

bool ApiConnection::getLoginStatus() const
{
    cpr::Response response = cpr::Get(cpr::Url{m_base_url + "login/status"});
    PluginJsonParser parser;
    if(response.status_code == 200)
    {
        LogInfo("Request was successful!\n");
        LogInfo("Response body:" + response.text);
        return parser.parseLoginStatus(response.text);
    }
    else
    {
        std::string error_string("Login_Status request failed with status code: " + std::to_string(response.status_code));
        LogError(std::string(error_string));
    }
    return false;
}

std::string ApiConnection::getUserInfo() const
{
    cpr::Response response = cpr::Get(cpr::Url{m_base_url + "login/username"});
    PluginJsonParser parser;
    std::string username;
    if (getLoginStatus())
    {
        if (response.status_code == 200)
        {
            LogInfo("Request was successful!\n");
            LogInfo("Response body:");
            if (parser.parseUserInfo(response.text, username))
            {
                return username;
            }
        }
        else
        {
            std::string error_string("Username request failed with status code: " + std::to_string(response.status_code));
            LogError(std::string(error_string));
        }
    }
    return "Logged In";
}

int ApiConnection::getUserSubscriptionLevel() const
{
    std::string url = m_base_url + "login/subscription_level";
    cpr::Response response = cpr::Get(cpr::Url{url});
    int subscriptionLevel = 0;
    PluginJsonParser parser;

    if(response.status_code == 200)
    {
        if(parser.parseSubscriptionLevel(response.text, subscriptionLevel))
        {
            LogInfo("Request was successful!\n");
            LogInfo("Response body:");
            parser.parseSubscriptionLevel(response.text, subscriptionLevel);
            if(subscriptionLevel == 0)
            {
                LogInfo("User is not logged in");
            }
            //subscriptionLevel = 2;//test
            return subscriptionLevel;
        }
        else
        {
            LogError("Error parsing subscription level response");
        }
    }
    else
    {
        std::string error_string("Request failed with status code: " + std::to_string(response.status_code));
        LogError(std::string(error_string));
    }
    return subscriptionLevel;
}

int ApiConnection::getPluginSubscriptionLevelRequirement(const std::string &plugin_name) const// this needs to be called when plugin list is being constructed to determine if the plugin can be run
{
    if (plugin_name.size() > 0)
    {
        std::string url = m_base_url + "plugins/get_info/" + plugin_name;

        cpr::Response response = cpr::Get(cpr::Url{url});
        PluginJsonParser parser;
        ArkPlugin plugin;
        if (response.status_code == 200)
        {
            LogInfo("Request was successful!\n");
            LogInfo("Response body:");
            parser.parsePluginInfo(response.text, plugin);
            return plugin.license_level;
        }
        else
        {
            std::string error_string("Request failed with status code: " + std::to_string(response.status_code));
            LogError(std::string(error_string));
        }
    }
    return 0; // return something that will not allow the plugin to run, maybe a negative number still thinking this through
}

std::vector<std::string> ApiConnection::getPluginList() const
{
    std::vector<std::string> plugin_list;
    std::string url = m_base_url + "plugins/get_list";

    cpr::Response response = cpr::Get(cpr::Url{url});

    if (response.status_code == 200)
    {
        LogInfo("Request was successful!\n");
    
        if (validatePluginListResponse(response.text)) 
        {
            plugin_list = parsePluginList(response.text);
            LogInfo("Response body:" + response.text);
        }
        else
        {
            LogError("Invalid plugin list response.");
        }
    }
    else
    {
        LogError("Request failed with status code: " + std::to_string(response.status_code));
    }
    return std::move(plugin_list);
}

bool ApiConnection::startPlugin(const std::string &plugin_name) const
{
    bool success = false;
    std::string url = m_base_url + "plugins/start_plugin/" + plugin_name;

    cpr::Response response = cpr::Get(cpr::Url{url});

    if (response.status_code == 200)
    {
        LogInfo("Request was successful!\n");
        LogInfo("Response body:" + response.text);
        success = true;
        //We should probably cache the status and port
    }
    else
    {
        std::string error_string("Request failed with status code: " + std::to_string(response.status_code));
        LogError(std::string(error_string));
    }
    return success;
}

bool ApiConnection::setConfig(const std::string &plugin_name, const std::string &body) const
{
    bool success = false;
    std::string url = m_base_url + "plugins/set_config/" + plugin_name;
    if (validateConfigJson(body))
    {
        cpr::Response response = cpr::Put(cpr::Url{url},
                    cpr::Header{{"Content-Type", "application/json"}},
                    cpr::Header{{"accept", "application/json"}},
                    cpr::Header{{"Content-Length", std::to_string(body.size())}},
                    cpr::Header{{"Host", m_base_url}},
                    cpr::Body{body});

        if (response.status_code == 200)
        {
            LogInfo("Request was successful!\n");
            LogInfo("Response body:" + body);
            success = true;
            //We should probably cache the status and port
        }
        else
        {
            std::string error_string("Request failed with status code: " + std::to_string(response.status_code));
            LogError(std::string(error_string));
        }   
    } 
    else 
    {
        LogError("Invalid configuration JSON data.");
    }
    return success;
}

bool ApiConnection::uploadImage(const ArkImagePtr &image, std::string &out_id) const
{
    bool success = false;

    if (image == nullptr)
        return false;

    std::string img_str = imageToPNG(image);
    if (img_str.empty())
        return false;

    // Create a temporary file to store the string data
    //This can be avoided if we change the networking library
    //to something like https://github.com/jgaa/restc-cpp 
    std::string tempFileName =  tmpDirectory() + "temp_img_file.txt";
    std::ofstream tempFile(tempFileName, std::ios::binary);
    if (!tempFile.is_open()) {
        LogError("Failed to create temporary file.");
        return false;
    }
    tempFile.write(img_str.c_str(), img_str.size());
    tempFile.close();

    cpr::Multipart formData{
        {"file", cpr::File(tempFileName, "temp_data_file.txt")}
    };
    
    std::string url = m_base_url + "image/upload";
    cpr::Response response = cpr::Post(
        cpr::Url{url},
        cpr::Multipart(formData),
        cpr::Header{{"accept", "application/json"}}
    );
    std::remove(tempFileName.c_str());

    if (response.status_code == 200)
    {
        LogInfo("Request was successful!\n");
        LogInfo("Response body:" + response.text);
        if (validateImgUploadResponse(response.text)) 
        {
            PluginJsonParser parser;
            success = parser.parseUploadImageResponse(response.text, out_id);
        }
        else
        {
            LogError("Invalid image upload response.",true);
        }
    }
    else
    {
        std::string error_string("Request failed with status code: " + std::to_string(response.status_code));
        LogError(std::string(error_string));
    }
    return success;
}

bool ApiConnection::uploadMultipleImages(const std::vector<ArkImagePtr> &images, std::vector<std::string> &out_ids) const
{
    bool success = true;

    for (const auto &image : images)
    {
        std::string out_id; 
        if ((success = uploadImage(image, out_id)))
        {
            out_ids.push_back(out_id);
        }
        else
        {
            out_ids.push_back("-1");
        }
    }

    return success;
}

std::string ApiConnection::getBackendConfigPath() const
{
    std::string json_file_path;

    #ifdef _WIN32
        json_file_path = std::getenv("APPDATA");
        json_file_path += "\\DeepMake\\Config.json";
        if(!std::filesystem::exists(json_file_path))
        {
            json_file_path = std::getenv("ProgramData");
            json_file_path += "\\DeepMake\\Config.json";
        }
    #endif
    #ifdef __APPLE__    
        json_file_path = std::getenv("HOME");
        json_file_path += "/Library/Application Support/DeepMake/Config.json";
        if(!std::filesystem::exists(json_file_path))
        {
            json_file_path = "/Library/Application Support/DeepMake/Config.json";
        }
    #endif

    return json_file_path;
}

bool ApiConnection::startupBackend(const BackendConfig &config) const
{
    int result = 0;

    #ifdef _WIN32
        std::string full_startup_cmd = config.py_dir + config.startup_cmd;
        LogInfo("[Backend Start] Attempting to start backend CMD :" + full_startup_cmd);
        LPSTR cmd = const_cast<LPSTR>(full_startup_cmd.c_str());
        LPCSTR dir = const_cast<LPCSTR>(config.startup_dir.c_str());

        STARTUPINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);

        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        if (CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, dir, &si, &pi))
        {
            // Process created successfully
            WaitForSingleObject(pi.hProcess, 1000); // Wait for the process to finish
        }
        else
        {
            result = GetLastError();
        }
    #endif
    #ifdef __APPLE__
        std::string full_startup_cmd = config.py_dir + config.startup_dir + config.startup_cmd;
        LogInfo("[Backend Start] Attempting to start backend CMD :" + full_startup_cmd);
        std::string appleScriptCmd = "osascript -e 'tell application \"Terminal\" to do script \"" + full_startup_cmd + "\"'";
        result = system(appleScriptCmd.c_str());
        arkSleepMS(std::chrono::seconds(4));
    #endif

    // Do multiple checks for the backend to start and delay if necessary
    int i = 0;
    while (!isBackendRunning() && i < 300)
    {
        arkSleepMS(std::chrono::milliseconds(100));
        i++;
    }

    if(isBackendRunning())
    {
        LogInfo("[Backend Start] SUCCESS");
        return true;
    }
    else
    {
        if (result != 0)
        {
            LogError(std::string("[Backend Start]:request failed with status code: " + std::to_string(result)), true);
        }
        else if(result == 0)
        {
            LogError("[Backend Start]: Backend started but crashed for unknown reasons check backend log", true);
        }
        else
        {
            LogError("[Backend Start]: Backend failed to start for unknown reason()", true);
        }
        if(std::filesystem::exists(config.startup_dir + "startup.py"))
        {
            LogError("startup.py file found but backend not running",true);
        }
        else
        {
            LogError("Config file & startup.py file not found, backend failed to start",true);
        }
        if(std::filesystem::exists(config.py_dir))
        {
            LogError("Conda directory found, backend failed to start",true);
        }
        else
        {
            LogError("Conda directory not found, backend failed to start",true);
        }
        return false;
    }

    return false;
}

bool ApiConnection::startBackend() const
{
    if (isBackendRunning())
        return true;

    PluginJsonParser parser;

    std::string json_file_path = getBackendConfigPath();
    if(!std::filesystem::exists(json_file_path))
    {
        LogError("Config file not found during startup process",true);
        return false;
    }

    BackendConfig config;
    if (parser.parseBackendConfigFile(json_file_path, config))
    {
        return startupBackend(config);
    }
    return false;
}

bool ApiConnection::shutdownBackend() const
{
    bool success = false;
    setData("shutdown", "{\"shutdown\": \"true\"}");
    LogInfo("Backend shutdown status cached, graceful shutdown initiated");
    std::string url = m_base_url + "backend/shutdown";
    cpr::Response response = cpr::Get(cpr::Url{url});
    if (response.status_code == 200)
    {
        LogInfo("Request was successful!\n");
        LogInfo("Response body: " + response.text);
        LogInfo("Backend shutdown request was successful");
        success = true;
    }
    else
    {
        std::string error_string("Backend shutdown request failed with status code: " + std::to_string(response.status_code),true);
        LogError(std::string(error_string));
    }

    return success;
}

bool ApiConnection::stopPlugin(const std::string &plugin_name) const
{
    bool success = false;
    std::string url = m_base_url + "plugins/stop_plugin/" + plugin_name;

    cpr::Response response = cpr::Get(cpr::Url{url});

    if (response.status_code == 200)
    {
        LogInfo("Request was successful!\n");
        LogInfo("Response body: " + response.text);
        success = true;
        //We should probably cache the status and port
    }
    else
    {
        std::string error_string("Request failed with status code: " + std::to_string(response.status_code));
        LogError(std::string(error_string));
    }
    return success;
}

bool ApiConnection::getPluginConfig(const std::string &plugin_name, struct ArkPlugin &plugin) const
{
    bool success = false;
    std::string url = m_base_url + "plugins/get_config/" + plugin_name;

    cpr::Response response = cpr::Get(cpr::Url{url});
    LogInfo("Get plugin config response: " + response.text);
    if (response.status_code == 200 && validateConfigJson(response.text))
    {
        LogInfo("Request was successful!\n");
        LogInfo("Response body:" + response.text);
        PluginJsonParser parser;
        success = parser.parseConfigJson(response.text, plugin);
    }
    else
    {
        std::string error_string("Request failed with status code: " + std::to_string(response.status_code));
        LogError(std::string(error_string),true);
    }
    return success;
}

bool ApiConnection::getPluginInfo(const std::string &plugin_name, struct ArkPlugin &plugin) const
{
    bool success = false;
    std::string url = m_base_url + "plugins/get_info/" + plugin_name;

    cpr::Response response = cpr::Get(cpr::Url{url});
    LogInfo("Get plugin info response: " + response.text);
    if (response.status_code == 200 && validateInfoResponse(response.text))
    {
        LogInfo("Request was successful!\n");
        LogInfo("Response body:" + response.text);
        PluginJsonParser parser;
        success = parser.parsePluginInfo(response.text, plugin);
        plugin.setPluginName(plugin_name);
    }
    else
    {
        std::string error_string("Request failed with status code: " + std::to_string(response.status_code));
        LogError(std::string(error_string));
    }
    return success;
}

std::vector<std::string> ApiConnection::parsePluginList(const std::string &plugin_json_list) const
{
    std::vector<std::string> plugin_list;

    try
    {
        rapidjson::Document document;
        document.Parse(plugin_json_list.c_str());

        if (!document.HasParseError() && document.IsObject())
        {
            if (document.HasMember("plugins") && document["plugins"].IsArray())
            {
                const rapidjson::Value& plugins = document["plugins"];

                for (rapidjson::SizeType i = 0; i < plugins.Size(); i++)
                {
                    if (plugins[i].IsString())
                    {
                        const char* pluginName = plugins[i].GetString();
                        plugin_list.push_back(pluginName);
                    }
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        LogError(std::string("parsePluginList error: " + std::string(e.what())));
    }

    return std::move(plugin_list);
}

std::map<std::string, std::string> ApiConnection::parsePluginInfo(const std::string &plugin_config_json) const
{
    std::map<std::string, std::string> plugin_config;

    try
    {
        rapidjson::Document document;
        document.Parse(plugin_config_json.c_str());

        if (document.HasParseError() || !document.IsObject()) 
        {
            LogError("ApiConnection::parsePluginConfig Error parsing JSON or JSON is not an object.");
            return plugin_config;
        }

        for (rapidjson::Value::ConstMemberIterator it = document.MemberBegin(); it != document.MemberEnd(); ++it) 
        {
            const std::string& key = it->name.GetString();
            const rapidjson::Value& value = it->value;

            if (value.IsString()) 
            {
                plugin_config[key] = value.GetString();
            } 
            else if (value.IsInt()) 
            {
                //assert(false);
                //std::cout << value.GetInt();
            } 
            else if (value.IsDouble()) 
            {
                //assert(false);
                //std::cout << value.GetDouble();
            } 
            else if (value.IsBool())
            {
                 //assert(false);
               //std::cout << (value.GetBool() ? "true" : "false");
            } 
            else 
            {
                //assert(false);
                //std::cout << "Unknown data type";
            }
        }
    }
    catch (const std::exception& e)
    {
        LogError(std::string("parsePluginConfig error: " + std::string(e.what())));
    }

    return std::move(plugin_config);
}


std::string ApiConnection::executeEndpoint(const std::string &plugin_name, const std::string &args) const
{
    std::string ret_job_id;
    std::string url = m_base_url + "plugins/execute/" + plugin_name + "/\"" + args + "\"";

    cpr::Response response = cpr::Get(cpr::Url{url});

    if (response.status_code == 200)
    {
        PluginJsonParser parser;
        ExecuteResponse execute_response;
        if (parser.parseExecuteResponse(response.text, execute_response))
        {
            ret_job_id = execute_response.job_id;
        }
        else
        {
            LogError("Error parsing execute response");
        }
    }
    else
    {
        std::string error_string("Request failed with status code: " + std::to_string(response.status_code));
        LogError(std::string(error_string));
    }
    return std::move(ret_job_id);
}

std::string ApiConnection::callEndpoint(const std::string &plugin_name, const std::string &endpoint, const std::string &body) const
{
    std::string ret_job_id;
    std::string url = m_base_url + "plugins/call_endpoint/" + plugin_name + "/" + endpoint;

    cpr::Response response = cpr::Put(cpr::Url{url},
                   cpr::Header{{"Content-Type", "application/json"}},
                   cpr::Header{{"accept", "application/json"}},
                   cpr::Header{{"Content-Length", std::to_string(body.size())}},
                   cpr::Header{{"Host", m_base_url}},
                   cpr::Body{body});

    LogInfo("Call endpoint response: " + response.text);
    
    if (response.status_code == 200 && validateEndpointResponse(response.text))
    {
        PluginJsonParser parser;
        ExecuteResponse execute_response;
        if (parser.parseExecuteResponse(response.text, execute_response))
        {
            ret_job_id = execute_response.job_id;
        }
        else
        {
            LogError("Error parsing execute response");
        }
    }
    else
    {
        std::string error_string("Request failed with status code: " + std::to_string(response.status_code));
        LogError(std::string(error_string));
    }
    return std::move(ret_job_id);
}

JobStatusResponse ApiConnection::jobStatus(const std::string &job_id) const
{
    JobStatusResponse job_status;
    std::string url = m_base_url + "job/" + job_id;

    cpr::Response response = cpr::Get(cpr::Url{url});
    LogInfo("Job status response: " + response.text);
    if (response.status_code == 200 && validateJobStatusResponse(response.text))
    {
        PluginJsonParser parser;
        if (!parser.parseJobResponse(response.text, job_status))
        {
            LogInfo("Error parsing jobStatus response");
        }
    }
    else
    {
        std::string error_string("Job request failed with status code: " + std::to_string(response.status_code));
        LogError(std::string(error_string));
    }

    LogInfo("Job status: " + stringFromJobStatus(job_status.status));
    return job_status;
}

ArkImagePtr ApiConnection::getImage(const std::string &img_id) const
{
    ArkImagePtr img;
    std::string url = m_base_url + "image/get/" + img_id;

    cpr::Response response = cpr::Get(cpr::Url{url});
    LogInfo("Get image response: " + response.text);
    if (response.status_code == 200 && validateImageResponse(response.text))
    {
        LogInfo("SUCCESS getting image");
        size_t textSize = response.text.size();
        img = ::getImage(response.text);
    }
    else
    {
        std::string error_string("getImage failed with status code: " + std::to_string(response.status_code));
        LogError(std::string(error_string));
    }
    return img;
}
void ApiConnection::openConfigMenu(std::string pluginName)
{
    cpr::Response response = cpr::Get(cpr::Url{m_base_url += "ui/configure/" + pluginName});
    LogInfo("openConfigMenu: " + pluginName + ": " + response.text);
}
void ApiConnection::openPluginManagerMenu()
{
cpr::Response response = cpr::Get(cpr::Url{m_base_url += "ui/plugin_manager"});
LogInfo("openPluginManagerMenu: " + response.text);
}
void ApiConnection::openReportIssueMenu()
{
    cpr::Response response = cpr::Get(cpr::Url{m_base_url + "ui/report_issue"});
    LogInfo("openReportIssueMenu: " + response.text);
}
void ApiConnection::openSupportURL(std::string pluginName)
{
    openURL("https://deepmake.com/support/" + pluginName);
    LogInfo("openSupportURL: " + pluginName);
}

void ApiConnection::openDownloadURL()
{
    openURL("https://deepmake.com/download");
    LogInfo("openDownloadURL");
}
void ApiConnection::openLoginMenu()
{
     std::string url = m_base_url + "ui/login";
    cpr::Response response = cpr::Get(cpr::Url{url});
}
// Validation functions
#pragma region Validation functions


bool ApiConnection::validateConfigJson(const std::string &json) const
{
    rapidjson::Document document;
    document.Parse(json.c_str());

    if (!document.HasParseError() && document.IsObject()) {
        if (document.HasMember("model_name") && document["model_name"].IsString() &&
            document.HasMember("model_dtype") && document["model_dtype"].IsString() &&
            document.HasMember("scheduler") && document["scheduler"].IsString() &&
            document.HasMember("controlnet") && document["controlnet"].IsString()) 
        {
            LogInfo("validateConfigJson: JSON is valid");
            return true;
        } 
        else 
        {
            LogError("validateConfigJson: JSON is invalid", true);
            return false;
        }
    } 
    else 
    {
        LogError("Parsing of config Json failed", true);
        return false;
    }
}

bool ApiConnection::validatePluginListResponse(const std::string &response) const
{
    rapidjson::Document document;
    document.Parse(response.c_str());

    if (!document.HasParseError() && document.IsObject()) 
    {
        if (document.HasMember("plugins") && document["plugins"].IsArray()) 
        {
            LogInfo("validatePluginListResponse: JSON is valid");
            return true;
        } 
        else 
        {
            LogError("validatePluginListResponse: JSON is invalid");
            return false;
        }
    } 
    else 
    {
         LogError("Parsing of plugin list response Json failed", true);
        return false;
    }
}

bool ApiConnection::validateInfoResponse(const std::string &response) const
{
    try
    {
        rapidjson::Document document;
        document.Parse(response.c_str());

        LogInfo("InfoResponse: " + response);
        if (!document.HasParseError() && document.IsObject()) 
        {
            if (document.HasMember("plugin") && document["plugin"].IsObject() &&
                document["plugin"].HasMember("Name") && document["plugin"]["Name"].IsString() &&
                document["plugin"].HasMember("Version") && document["plugin"]["Version"].IsString() &&
                document["plugin"].HasMember("Author") && document["plugin"]["Author"].IsString() &&
                document["plugin"].HasMember("Description") && document["plugin"]["Description"].IsString() &&
                document["plugin"].HasMember("env") && document["plugin"]["env"].IsString() &&
                document.HasMember("config") && document["config"].IsObject() &&
                document.HasMember("endpoints") && document["endpoints"].IsObject()) 
            {
                const rapidjson::Value& endpoints = document["endpoints"];

                for (rapidjson::Value::ConstMemberIterator it = endpoints.MemberBegin(); it != endpoints.MemberEnd(); ++it) 
                {
                    const std::string& endpoint_name = it->name.GetString();
                    const rapidjson::Value& endpoint_value = it->value;

                    if (!endpoint_value.IsObject() ||
                        !endpoint_value.HasMember("call") || !endpoint_value["call"].IsString() ||
                        !endpoint_value.HasMember("inputs") || !endpoint_value["inputs"].IsObject() ||
                        !endpoint_value.HasMember("outputs") || !endpoint_value["outputs"].IsObject()) 
                    {
                        LogError("validateInfoResponse: Invalid endpoint object for endpoint: " + endpoint_name, true);
                        return false;
                    }

                    const rapidjson::Value& inputs = endpoint_value["inputs"];
                    for (rapidjson::Value::ConstMemberIterator input_it = inputs.MemberBegin(); input_it != inputs.MemberEnd(); ++input_it) 
                    {
                        if (!input_it->value.IsString()) 
                        {
                            LogError("validateInfoResponse: Invalid input type for endpoint: " + endpoint_name, true);
                            return false;
                        }
                    }

                    const rapidjson::Value& outputs = endpoint_value["outputs"];
                    for (rapidjson::Value::ConstMemberIterator output_it = outputs.MemberBegin(); output_it != outputs.MemberEnd(); ++output_it) 
                    {
                        if (!output_it->value.IsString()) 
                        {
                            LogError("validateInfoResponse: Invalid output type for endpoint: " + endpoint_name, true);
                            return false;
                        }
                    }
                }

                LogInfo("validateInfoResponse: JSON is valid");
                return true;
            } 
            else 
            {
                LogError("validateInfoResponse: JSON is invalid, fields are missing or have incorrect types", true);
                return false;
            }
        } 
        else 
        {
             LogError("Parsing of info response JSON failed", true);
            return false;
        }
    }
    catch (const std::exception& e)
    {
        LogError("validateInfoResponse: Exception occurred while parsing response JSON: " + std::string(e.what()), true);
        return false;
    }
}


bool ApiConnection::validatePluginResponse(const std::string &response) const
{
    try
    {

        rapidjson::Document document;
        document.Parse(response.c_str());
        LogInfo("PluginResponse: " + response);
        if (!document.HasParseError() && document.IsObject()) 
        {
            if (document.HasMember("plugin") && document["plugin"].IsObject() &&
                document["plugin"].HasMember("Name") && document["plugin"]["Name"].IsString() &&
                document["plugin"].HasMember("Version") && document["plugin"]["Version"].IsString() &&
                document["plugin"].HasMember("Author") && document["plugin"]["Author"].IsString() &&
                document["plugin"].HasMember("Description") && document["plugin"]["Description"].IsString() &&
                document["plugin"].HasMember("env") && document["plugin"]["env"].IsString() &&
                document.HasMember("config") && document["config"].IsObject() &&
                document.HasMember("endpoints") && document["endpoints"].IsObject()) 
            {
                
                LogInfo("validatePluginResponse: JSON is valid");
                return true;
            } 
            else 
            {
                LogError("validatePluginResponse: JSON is invalid, fields are missing or have incorrect types: \n" + response, true);
                return false;
            }
        } 
        else 
        {
             LogError("Parsing of plugin response Json failed", true);
            return false;
        }
    }
    catch (const std::exception& e)
    {
        LogError(std::string("validateJobStatusResponse error: " + std::string(e.what())), true);
        return false;
    }
}

bool ApiConnection::validateJobStatusResponse(const std::string &response) const
{
    try
    {
        rapidjson::Document document;
        document.Parse(response.c_str());
        LogInfo("JobStatusResponse: " + response);
        if (!document.HasParseError() && document.IsObject()) 
        {
            if (document.HasMember("status") && document["status"].IsString()) 
            {
                LogInfo("validateJobStatusResponse: JSON is valid");
                return true;
            } 
            else 
            {
                LogError("validateJobStatusResponse: JSON is invalid",true);
                return false;
            }
        } 
        else
        {
            LogError("Parsing of job status response Json failed", true);
            return false;
        }
    }
    catch (const std::exception& e)
    {
        LogError(std::string("validateJobStatusResponse error: " + std::string(e.what())), true);
        return false;
    }
}

bool ApiConnection::validateImageResponse(const std::string &response) const
{
   
    if (response.empty())
    {
        LogError("validateImageResponse: Empty response received");
        return false;
    }

    // Since the response is not JSON, we cannot perform JSON parsing/validation here
    LogInfo("validateImageResponse: Non-empty response received");
    return true;

}

bool ApiConnection::validateEndpointResponse(const std::string &response) const
{
    try
    {
        rapidjson::Document document;
        document.Parse(response.c_str());
        LogInfo("EndpointResponse: " + response);
        if (!document.HasParseError() && document.IsObject()) 
        {
            if (document.HasMember("job_id") && document["job_id"].IsString()) 
            {
                LogInfo("validateEndpointResponse: JSON is valid");
                return true;
            }
            else 
            {
                LogError("validateEndpointResponse: JSON is invalid",true);
                return false;
            }
        } 
        else 
        {
            LogError("Parsing of endpoint response Json failed", true);
            return false;
        }
    }
    catch (const std::exception& e)
    {
        LogError(std::string("validateEndpointResponse error: " + std::string(e.what())), true);
        return false;
    }
}

bool ApiConnection::validateImgUploadResponse(const std::string &response) const
{
    try
    {
        rapidjson::Document document;
        document.Parse(response.c_str());

        LogInfo("ImgUploadResponse: " + response);

        if (!document.HasParseError() && document.IsObject()) 
        {
            if (document.HasMember("status") && document["status"].IsString()) 
            {
                if (document.HasMember("image_id") && document["image_id"].IsString()) 
                {
                    LogInfo("validateImgUploadResponse: JSON is valid");
                    return true;
                } 
                else 
                {
                    LogError("validateImgUploadResponse: 'image_id' field is missing or not a string", true);
                    return false;
                }
            } 
            else 
            {
                LogError("validateImgUploadResponse: 'status' field is missing or not a string", true);
                return false;
            }
        } 
        else 
        {
            LogError("validateImgUploadResponse: Parsing of response JSON failed", true);
            return false;
        }
    }
    catch (const std::exception& e)
    {
        LogError("validateImgUploadResponse: Exception occurred while parsing response JSON: " + std::string(e.what()), true);
        return false;
    }
}

bool ApiConnection::validateMultipleImgUploadResponse(const std::string &response) const
{
    try
    {
        rapidjson::Document document;
        document.Parse(response.c_str());
        LogInfo("MultipleImgUploadResponse: " + response);
        if (!document.HasParseError() && document.IsObject()) 
        {
            if (document.HasMember("status") && document["status"].IsString()) 
            {
                if (document.HasMember("images") && document["images"].IsArray()) 
                {
                    const rapidjson::Value& images = document["images"];
                    for (rapidjson::SizeType i = 0; i < images.Size(); i++) 
                    {
                        if (!images[i].IsString()) 
                        {
                            LogError("validateMultipleImgUploadResponse: Image ID in the 'images' array is not a string", true);
                            return false;
                        }
                    }
                    LogInfo("validateMultipleImgUploadResponse: JSON is valid");
                    return true;
                } 
                else 
                {
                    LogError("validateMultipleImgUploadResponse: 'images' field is missing or not an array", true);
                    return false;
                }
            } 
            else 
            {
                LogError("validateMultipleImgUploadResponse: 'status' field is missing or not a string", true);
                return false;
            }
        } 
        else 
        {
            LogError("validateMultipleImgUploadResponse: Parsing of response JSON failed", true);
            return false;
        }
    }
    catch (const std::exception& e)
    {
        LogError("validateMultipleImgUploadResponse: Exception occurred while parsing response JSON: " + std::string(e.what()), true);
        return false;
    }
}

#pragma endregion


bool ApiConnection::setData(const std::string &id, const std::string &data) const
{
    bool success = false;
    try 
    {
        LogInfo("Setting Data in the backend: [" + id + "]" + ":" + data);
        std::string url = m_base_url + "data/store/" + id;

        cpr::Response response = cpr::Put(
            cpr::Url{url},
            cpr::Body{data},
            cpr::Header{{"Content-Type", "application/json"}} // Set content type header
        );

        if (response.status_code == 200)
        {
            success = true;
        }
        else
        {
            throw std::runtime_error("Job request failed with status code: " + std::to_string(response.status_code));
        }
    } 
    catch(const std::exception& e) 
    {
        LogError(std::string("Error in ApiConnection::setData: ") + e.what());
        success = false;
    }
    return success;
}

bool ApiConnection::deleteData(const std::string &id) const
{
    bool success = false;
    try
    {
        std::string url = m_base_url + "data/delete/" + id;

        cpr::Response response = cpr::Delete(
            cpr::Url{url}
        );

        if (response.status_code == 200)
        {
            success = true;
        }
        else
        {
            throw std::runtime_error("Job request failed with status code: " + std::to_string(response.status_code));
        }
    }
    catch(const std::exception& e)
    {
        LogError(std::string("Error ApiConnection::deleteData: ") + e.what());
        success = false;
    }
    return success;
}

std::string ApiConnection::getData(const std::string &id) const
{
    std::string retVal;
    std::string url = m_base_url + "data/retrieve/" + id;

    cpr::Response response = cpr::Get(cpr::Url{url});

    if (response.status_code == 200)
    {
        retVal = response.text.c_str();
    }
    else
    {
        LogError("Job request failed with status code: " + std::to_string(response.status_code));
    }

    return retVal;
}

bool ApiConnection::hasShutdownGracefully() const
{
    bool success = false;
    try
    {
        std::string response = getData("shutdown");
        rapidjson::Document document;
        if (document.Parse(response.c_str()).HasParseError())
        {
            LogError("Error parsing JSON response");
            success = false;
        }
        else if (document.HasMember("shutdown") && document["shutdown"].IsString())
        {
            std::string shutdownStatus = document["shutdown"].GetString();
            if (shutdownStatus == "true")
            {
                success = true;
            }
        }
    }
    catch (const std::exception &e)
    {
        LogError(std::string("Error in ApiConnection::hasShutdownGracefully: ") + e.what());
    }
    LogInfo("Shutdown Gracefully: " + std::string(success ? "true" : "false"));
    return success;
}
