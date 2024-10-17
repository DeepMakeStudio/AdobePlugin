#ifndef ARK_PLUGIN_H
#define ARK_PLUGIN_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <regex>
#include "parameter.h"
#include <logging/logger.h>

constexpr const char* kDefaultValue = "default=";
constexpr const char* kMinValue = "min=";
constexpr const char* kMaxValue = "max=";
constexpr const char* kOptional = "optional=";
constexpr const char* kTrue = "true";
constexpr const char* kFalse = "false";
constexpr const char* kNoneValue = "=None";

constexpr const char* kIntParamType = "Int(";
constexpr const char* kFloatParamType = "Float(";
constexpr const char* kBoolParamType = "Bool(";
constexpr const char* kTextParamType = "Text";
constexpr const char* kImageParamType = "Image";
constexpr const char* kImageListParamType = "List";

constexpr const char* kHelpRegex = "help='(.*?)'";
constexpr const char* kBoolValueRegex = "([^,\\s]+)";

struct EndpointParam
{
    std::string name;
    ParameterType type {ParameterType::Unknown};
    int id {-1};
    
    bool optional {false};
    std::variant<int, float, bool> defaultVal {-1};
    std::variant<int, float>  min {-1};
    std::variant<int, float>  max{-1};
    std::string string_val;
    std::string help_string;
    std::vector<std::string> image_list; // New member for storing image list
    
};

struct Data 
{
    std::string name;
    std::string parameter_data;
};

struct Endpoint
{
    std::string name;
    std::string plugin_name;
    std::string call;
    std::string tag;
    bool has_prompt {false};
    std::vector<Data> inputs;
    std::vector<Data> outputs;

    int prompt_param_id {-1};

    bool outputIsMask()
    {
        for (const auto& output : outputs)
        {
            if (output.name.find("output_mask") != std::string::npos)
                return true;
        }
        return false;
    }
    std::vector<EndpointParam> &inputParams()
    {
        if (!endpoint_params.empty())
            return endpoint_params;
        
        for (const auto& input : inputs)
        {
            EndpointParam param;
            param.name = input.name;
            param.type = paramTypeFromString(input.parameter_data);
            if (param.type == ParameterType::IntSlider)
            {
                getIntData(input, param);
           }
            else if (param.type == ParameterType::Boolean)
            {
                getBoolData(input, param);
           }
            else if (param.type == ParameterType::FloatSlider)
            {
                getFloatData(input, param);
            }
            endpoint_params.push_back(param);
        }
        return endpoint_params;
    }
    
    void getBoolData(const Data &data, EndpointParam &param)
    {
        bool boolValue = false;
        if (boolFromDataString(data.parameter_data, std::string(kDefaultValue), boolValue))
            param.defaultVal = boolValue;

        param.help_string = getHelpString(data.parameter_data);
        param.optional = getOptionalValue(data.parameter_data);
    }

    void getIntData(const Data &data, EndpointParam &param)
    {
        int intValue = 0;
        if (intFromDataString(data.parameter_data, std::string(kDefaultValue), intValue))
            param.defaultVal = intValue;
        if (intFromDataString(data.parameter_data, std::string(kMinValue), intValue))
            param.min = intValue;
        if (intFromDataString(data.parameter_data, std::string(kMaxValue), intValue))
            param.max = intValue;
        
        param.help_string = getHelpString(data.parameter_data);
        param.optional = getOptionalValue(data.parameter_data);
    }

    void getFloatData(const Data &data, EndpointParam &param)
    {
        float floatValue = 0;
        if (floatFromDataString(data.parameter_data, std::string(kDefaultValue), floatValue))
            param.defaultVal = floatValue;
        if (floatFromDataString(data.parameter_data, std::string(kMinValue), floatValue))
            param.min = floatValue;
        if (floatFromDataString(data.parameter_data, std::string(kMaxValue), floatValue))
            param.max = floatValue;
        if (floatFromDataString(data.parameter_data, std::string(kMaxValue), floatValue))
            param.max = floatValue;

        param.help_string = getHelpString(data.parameter_data);
        param.optional = getOptionalValue(data.parameter_data);
    }

    std::string getHelpString(const std::string &paramData)
    {
        std::string helpString;
        std::regex pattern(kHelpRegex);
        std::smatch match;
        if (std::regex_search(paramData, match, pattern)) 
        {
            if (match.size() > 1) {
                helpString = match.str(1);
            }
        } 
        return helpString;
    }

    bool getOptionalValue(const std::string &paramData)
    {
        bool is_optional = false;
        size_t pos = paramData.find(kOptional);
        if(pos != std::string::npos)
        {
            is_optional = paramData.substr(pos + 9, 4) == kTrue;
        }
        return is_optional;
    }

    std::string getInputImgParam()
    {
        for (const auto &param : endpoint_params)
        {
            if (param.type == ParameterType::Image)
            {
                LogInfo("Input_Img_Param" + param.name);
                return param.name;
            }
        }
        return "";
    }
    std::vector<std::string> getInputImgListParams()
    {
        std::vector<std::string> img_params;
        for (const auto &param : endpoint_params)
        {
            if (param.type == ParameterType::Image_List)
            {
                img_params.push_back(param.name);
                
            }
        }
        return img_params;
    }

    int countInputOccurrences(std::string input, std::string target)
    {
        int count = 0;
        for (auto param : endpoint_params)
        {
            if (param.name == input)
            {
                for (auto data : inputs)
                {
                    if (data.name == input)
                    {
                        while (data.parameter_data.find(target) != std::string::npos)
                        {
                            ++count;
                            data.parameter_data = data.parameter_data.substr(data.parameter_data.find(target) + target.length());
                        }
                    }
                }
            }
        }
        LogInfo(input + ":Input Requirement:Count(" + std::to_string(count) + ")");
        return count;
    }

    bool setInputId(const std::string &param_name, const std::string &img_id)
    {
        for (auto& param : endpoint_params)
        {
            if (param.name == param_name)
            {
                param.string_val = img_id;
                return true;
            }
        }
        return false;
    }

    ParameterType paramTypeFromString(const std::string param_string) const
    {
        ParameterType param_type{ParameterType::Unknown};
        if (param_string.find(kIntParamType) != std::variant_npos)
            param_type = ParameterType::IntSlider;
        if (param_string.find(kFloatParamType) != std::variant_npos)
            param_type = ParameterType::FloatSlider;
        else if (param_string.find(kTextParamType) != std::variant_npos)
            param_type = ParameterType::Text;
        else if (param_string.find(kImageListParamType) != std::string::npos)
            return ParameterType::Image_List;
        else if (param_string.find(kImageParamType) != std::variant_npos)
            param_type = ParameterType::Image;
        else if (param_string.find(kBoolParamType) != std::variant_npos)
            param_type = ParameterType::Boolean;
        return param_type;
    }
    std::string getTag()
    {
        return tag;
    }
private:

    std::vector<EndpointParam> endpoint_params;

    bool intFromDataString(const std::string &data_string, const std::string &search_string, int &out_val) const
    {
        bool success = false;
        size_t pos = data_string.find(search_string);
        if(pos != std::string::npos)
        {
            std::string sub_str = data_string.substr(pos + search_string.size());
            try
            {
                if (sub_str.find(kNoneValue) == std::string::npos)
                {
                    out_val = std::stoi(sub_str);
                    success = true;
                }
            }
            catch(...)
            {
            }
        }
        return success;
    }
        
    bool boolFromDataString(const std::string &data_string, const std::string &search_string, bool &out_val) const
    {
        size_t pos = data_string.find(search_string);
        if(pos != std::string::npos)
        {
            std::regex pattern(search_string + kBoolValueRegex);
            std::smatch match;
            if (std::regex_search(data_string, match, pattern))
            {
                if (match.size() > 1)
                {
                    std::string default_value = match.str(1);
                    if (default_value == kTrue)
                    {
                        out_val = true;
                        return true;
                    }
                    else if (default_value == kFalse)
                    {
                        out_val = false;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool floatFromDataString(const std::string &data_string, const std::string &search_string, float &out_val) const
    {
        bool success = false;
        size_t pos = data_string.find(search_string);
        if(pos != std::string::npos)
        {
            std::string sub_str = data_string.substr(pos + search_string.size());
            try
            {
                if (sub_str.find(kNoneValue) == std::string::npos)
                {
                    out_val = std::stof(sub_str);
                    success = true;
                }
            }
            catch(...)
            {
            }
        }
        return success;
    }

};

struct Config
{
    std::string model_name;
    std::string model_dtype;
    bool save_output {false};
};

struct ArkPlugin
{
    std::string name; //UI Name
    std::string plugin_name; //Internal name (used for constructing URLs)
    std::string version;
    std::string author;
    std::string description;
    std::string env;
    int license_level {0};
    Config config;
    
    void setPluginName(const std::string &in_plugin_name)
    {
        plugin_name = in_plugin_name;
        for (auto &endpoint : endpoints)
        {
            endpoint.plugin_name = plugin_name;
        }
    }
    std::vector<Endpoint> endpoints;
};

#endif // ARK_PLUGIN_H
