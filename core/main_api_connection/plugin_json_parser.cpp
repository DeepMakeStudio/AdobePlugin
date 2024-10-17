
#include "plugin_json_parser.h"
#include "rapidjson/filereadstream.h"
#include <iostream>
#include <fstream>
#include "logger.h"

std::string PluginJsonParser::getTextFromFile(const std::string &file_path) const
{
    std::ifstream inputFile(file_path);

    if (!inputFile.is_open()) 
    {
        LogError("PluginJsonParser::getTextFromFile() Failed to open file: " + file_path);
        return "";
    }

    std::string file_data((std::istreambuf_iterator<char>(inputFile)),
                            std::istreambuf_iterator<char>());
    inputFile.close();

    return std::move(file_data);
}

bool PluginJsonParser::parseBackendConfigFile(const std::string &file_path, struct BackendConfig &config) const
{
    bool success = false;

    try 
    {
        std::string json_data = getTextFromFile(file_path);
        if(json_data.empty())
        {
            return false;
        }

        rapidjson::Document doc;
        doc.Parse(json_data.c_str());

        if (doc.HasParseError() || !doc.IsObject()) 
        {
            LogError("ConfigParser::parseBackendConfig Error parsing JSON or JSON is not an object.");
            return false;
        }

        if (doc.HasMember("Py_Environment") && doc["Py_Environment"].IsString())
            config.py_dir = doc["Py_Environment"].GetString();
        else
            throw std::runtime_error("Error fetching Py_Environment");

        if (doc.HasMember("Directory") && doc["Directory"].IsString())
            config.startup_dir = doc["Directory"].GetString();
        else
            throw std::runtime_error("Error fetching Directory");

        if (doc.HasMember("Startup_CMD") && doc["Startup_CMD"].IsString())
            config.startup_cmd = doc["Startup_CMD"].GetString();
        else
            throw std::runtime_error("Error fetching Startup_CMD");

        success = true;
    }
    catch (const std::exception &e)
    {
        std::string msg  = std::string("Exception parseBackendConfig(): ") + e.what();
        LogError(msg);
        success = false;
    }

    return success;
}

bool PluginJsonParser::parseConfigJson(const std::string &plugin_config_json, struct ArkPlugin &plugin) const
{
    bool success = false;

    try 
    {
        rapidjson::Document doc;
        doc.Parse(plugin_config_json.c_str());

        if (doc.HasParseError() || !doc.IsObject()) 
        {
            LogError("ConfigParser::parsePluginConfig Error parsing JSON or JSON is not an object.");
            return false;
        }

        const rapidjson::Value& plugin_config = *dynamic_cast<const rapidjson::Value*>(&doc);
        success = parseConfigValue(plugin_config, plugin);
    }
    catch (const std::exception &e)
    {
        std::string msg  = std::string("Exception parsePluginConfig(): ") + e.what();
        LogError(msg);
    }

    return success;
}

bool PluginJsonParser::parseLoginStatus(const std::string &response_json) const
{
    bool success = false;
    try
    {
        rapidjson::Document doc;
        doc.Parse(response_json.c_str());
        if(doc.HasParseError() || !doc.IsObject())
        {
            LogError("ConfigParser::parseLoginStatus Error parsing JSON or JSON is not an object.");
            return success;
        }
        if(doc.HasMember("logged_in") && doc["logged_in"].IsBool())
        {
             success = doc["logged_in"].GetBool();
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return success;
}

bool PluginJsonParser::parseUserInfo(const std::string &response_json, std::string &user_info) const
{
    bool success = false;
    try
    {
        rapidjson::Document doc;
        doc.Parse(response_json.c_str());
        if(doc.HasParseError() || !doc.IsObject())
        {
            LogError("ConfigParser::parseUserInfo Error parsing JSON or JSON is not an object.");
            success = false;
        }
        else if(doc.HasMember("username") && doc["username"].IsString())
        {
            user_info = doc["username"].GetString();
            success = true;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return success;
}

bool PluginJsonParser::parseConfigDoc(const rapidjson::Document &doc, struct ArkPlugin &plugin) const
{
    bool success = false;

    try
    {
        const rapidjson::Value& configInfo = doc["config"];
        success = parseConfigValue(configInfo, plugin);
    }
    catch (const std::exception &e)
    {
        std::string msg  = std::string("Exception parseConfig(): ") + e.what();
        LogError(msg);
    }

    return success;
}

bool PluginJsonParser::parseConfigValue(const rapidjson::Value& plugin_config, struct ArkPlugin &plugin) const
{
    bool success = false;

    try
    {
        if (plugin_config.HasMember("model_name"))
            plugin.config.model_name = plugin_config["model_name"].GetString();
        if (plugin_config.HasMember("model_dtype"))
            plugin.config.model_dtype = plugin_config["model_dtype"].GetString();
        if (plugin_config.HasMember("save_output"))
            plugin.config.save_output = plugin_config["save_output"].GetBool();
        success = true;
    }
    catch (const std::exception &e)
    {
        std::string msg  = std::string("Exception parsePluginConfig(): ") + e.what();
        LogError(msg);
    }

    return success;
}


bool PluginJsonParser::parsePluginInfo(const std::string &plugin_config_json, struct ArkPlugin &plugin) const
{
    bool success = false;

    try
    {
        rapidjson::Document doc;
        doc.Parse(plugin_config_json.c_str());

        if (doc.HasParseError() || !doc.IsObject())
        {
            LogError("ConfigParser::parsePluginInfo Error parsing JSON or JSON is not an object.");
            return false;
        }

        if (!parsePlugin(doc, plugin))
        {
            LogError("ConfigParser::parsePluginInfo Error parsing plugin.");
            return false;
        }

        if (!parseConfigDoc(doc, plugin))
        {
            LogError("ConfigParser::parsePluginInfo Error parsing config.");
            return false;
        }

        if (!parseEndpoints(doc, plugin))
        {
            LogError("ConfigParser::parsePluginInfo Error parsing endpoints.");
            return false;
        }
        success = true;
    }
    catch (const std::exception &e)
    {
        std::string msg  = std::string("Exception parsePluginInfo(): ") + e.what();
        LogError(msg);
    }

    return success;
}

bool PluginJsonParser::parseEndpoints(const rapidjson::Document &doc, struct ArkPlugin &plugin) const
{
    bool success = false;

    try 
    {
        plugin.endpoints.clear();

        const rapidjson::Value& endpoints = doc["endpoints"];
        for (rapidjson::Value::ConstMemberIterator it = endpoints.MemberBegin(); it != endpoints.MemberEnd(); ++it) {
            Endpoint endpoint;
            endpoint.name = it->name.GetString();
            const rapidjson::Value& endpointInfo = it->value;
            endpoint.call = endpointInfo["call"].GetString();
            if (endpointInfo.HasMember("tag"))
                endpoint.tag = endpointInfo["tag"].GetString();
            const rapidjson::Value& inputs = endpointInfo["inputs"];
            for (rapidjson::Value::ConstMemberIterator inputIt = inputs.MemberBegin(); inputIt != inputs.MemberEnd(); ++inputIt) {
                Data param_data;
                param_data.name = inputIt->name.GetString();
                param_data.parameter_data = inputIt->value.GetString();
                endpoint.inputs.push_back(param_data);
            }

            const rapidjson::Value& outputs = endpointInfo["outputs"];
            for (rapidjson::Value::ConstMemberIterator outputIt = outputs.MemberBegin(); outputIt != outputs.MemberEnd(); ++outputIt) {
                Data param_data;
                param_data.name = outputIt->name.GetString();
                param_data.parameter_data = outputIt->value.GetString();
                endpoint.outputs.push_back(param_data);
            }

            plugin.endpoints.push_back(endpoint);
            success = true;
        }
    }
    catch (const std::exception &e)
    {
        std::string msg  = std::string("Exception parseEndpoints(): ") + e.what();
        LogError(msg);
    }

    return success;
}

bool PluginJsonParser::parsePlugin(const rapidjson::Document &doc, struct ArkPlugin &plugin) const
{
    bool success = false;

    try 
    {
        const rapidjson::Value& pluginInfo = doc["plugin"];
        plugin.name = pluginInfo["Name"].GetString();
        plugin.version = pluginInfo["Version"].GetString();
        plugin.author = pluginInfo["Author"].GetString();
        plugin.description = pluginInfo["Description"].GetString();
        plugin.env = pluginInfo["env"].GetString();
        plugin.license_level = pluginInfo["license"].GetInt();
        success = true;
    }
    catch (const std::exception &e)
    {
        std::string msg  = std::string("Exception parsePlugin(): ") + e.what();
        LogError(msg);
    }

    return success;
}

bool PluginJsonParser::parseExecuteResponse(const std::string &response_json, struct ExecuteResponse &response) const
{
    bool success = false;

    try 
    {
        rapidjson::Document doc;
        doc.Parse(response_json.c_str());

        if (doc.HasParseError() || !doc.IsObject()) 
        {
            LogError("ConfigParser::parseExecuteResponse Error parsing JSON or JSON is not an object.");
            return false;
        }

        const rapidjson::Value& job_id = doc["job_id"];
        if (job_id.IsString())
        {
            response.job_id = job_id.GetString();
            success = true;
        }
        else
        {
            LogError("ConfigParser::parseExecuteResponse Error parsing job_id.");
            return false;
        }
    }
    catch (const std::exception &e)
    {
        std::string msg  = std::string("Exception parseExecuteResponse(): ") + e.what();
        LogError(msg);
    }

    return success;
}

bool PluginJsonParser::parseUploadImageResponse(const std::string &response_json, std::string &img_id) const
{
    bool success = false;

    try 
    {
        rapidjson::Document doc;
        doc.Parse(response_json.c_str());

        if (doc.HasMember("status"))
        {
            if (doc["status"] == "Success")
            {
                if (doc.HasMember("image_id"))
                {
                    img_id = doc["image_id"].GetString();
                    success = true;
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        std::string msg  = std::string("Exception parseUploadImageResponse(): ") + e.what();
        LogError(msg);
    }

    return success;
}

bool PluginJsonParser::parseSubscriptionLevel(const std::string &response_json, int& level) const
{
    try
    {
        rapidjson::Document doc;
        doc.Parse(response_json.c_str());

        if (doc.HasMember("subscription_level"))
        {
            if (doc["subscription_level"].IsInt())
            {
                level = doc["subscription_level"].GetInt();
                LogInfo("Subscription level: " + std::to_string(level));
                return true;
            }
            else if (doc["subscription_level"].IsBool())
            {
                level = 0;
                LogInfo("Subscription level: " + std::to_string(level));
                return true;
            }
        }
        else
            return false;
        
    }
    catch (const std::exception &e)
    {
        std::string msg  = std::string("Exception parseSubscriptionLevel(): ") + e.what();
        LogError(msg);
    }
    return false;
}

bool PluginJsonParser::parseJobResponse(const std::string &response_json, struct JobStatusResponse &response) const
{
    bool success = false;

    try 
    {
        rapidjson::Document doc;
        doc.Parse(response_json.c_str());

        if (doc.HasParseError() || !doc.IsObject()) 
        {
            LogError("ConfigParser::parseJobResponse Error parsing JSON or JSON is not an object.");
            return false;
        }
        if (doc.HasMember("status"))
        {
            const rapidjson::Value& status_string = doc["status"];
            if (status_string.IsString())
            {
                response.status = jobStatusFromString(status_string.GetString());
                if (response.status == JOB_STATUS_SUCCESS)
                {
                    if (doc.HasMember("output_img"))
                    {
                        const rapidjson::Value& status_string = doc["output_img"];
                        if (status_string.IsString())
                        {
                            response.img_id = status_string.GetString();
                            success = true;
                        }
                    }
                    else if (doc.HasMember("output_mask"))
                    {
                        const rapidjson::Value& status_string = doc["output_mask"];
                        if (status_string.IsString())
                        {
                            response.img_id = status_string.GetString();
                            success = true;
                        }
                    }

                }
                else
                    success = true;
            }
            else
            {
                LogError("ConfigParser::parseJobResponse Error parsing job_id.");
                return false;
            }
        }
        if (doc.HasMember("detail"))
        {
            const rapidjson::Value& status_string = doc["detail"];
            if (status_string.IsString())
            {
                response.status = jobStatusFromString(status_string.GetString());
                success = true;
            }
            else
            {
                LogError("ConfigParser::parseJobResponse Error parsing job_id.");
                return false;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::string msg  = std::string("Exception parseJobResponse(): ") + e.what();
        LogError(msg);
    }

    return success;
}

JobStatus jobStatusFromString(const std::string &status_string)
{
    LogInfo("Job status: " + status_string);
    
    if (status_string == "Job in progress")
    {
        LogInfo("Job in progress");
        return JOB_STATUS_IN_PROGRESS;
    }
    else if (status_string == "Success" || status_string == "success")
    {
        LogInfo(" Job Success");
        return JOB_STATUS_SUCCESS;
    }
    else if (status_string == "Job error")
    {
        LogError("Job error");
        return JOB_STATUS_ERROR;
    }
    else if (status_string == "Not Found")
    {
        LogError("Job not found",true);
        return JOB_STATUS_NOT_FOUND;
    }

    LOG_ASSERT(false, "Job status unknown: " + status_string);
    return JOB_STATUS_UNKNOWN;
}

std::string stringFromJobStatus(JobStatus job_status)
{
    switch (job_status)
    {
        case JOB_STATUS_IN_PROGRESS:
            return "Job in progress";
        case JOB_STATUS_SUCCESS:
            return "Success";
        case JOB_STATUS_ERROR:
        default:
            return "Job error";
    }
    return "Unknown";
}

