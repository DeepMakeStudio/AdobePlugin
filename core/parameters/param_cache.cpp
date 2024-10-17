#include "param_cache.h"
#include "main_api_connection.h"
#include <rapidjson/document.h>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"


ParamCache::ParamCache(const std::string &cache_id)
: m_cache_id(cache_id)
{
}

bool ParamCache::initialized() const
{
    size_t mapSize = param_map.size();
    bool inited = mapSize > 0;
    return inited;
}

void ParamCache::init()
{
    ApiConnection api_connection;
    std::string cache_data = api_connection.getData(m_cache_id);

    rapidjson::Document document;
    document.Parse(cache_data.c_str());

    if (document.HasParseError()) {
        std::cerr << "Parsing JSON failed with error code " << document.GetParseError() << std::endl;
        return;
    }
    rapidjson::Value& parameters = document["parameters"];
    if (!parameters.IsArray()) {
        std::cerr << "Invalid JSON format - 'parameters' is not an array" << std::endl;
        return;
    }

    for (rapidjson::SizeType i = 0; i < parameters.Size(); i++)
    {
        const rapidjson::Value &parameter = parameters[i];
        if (parameter.HasMember("stringId") && parameter.HasMember("id"))
        {
            std::string stringId = parameter["stringId"].GetString();
            int intId = parameter["id"].GetInt();
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            parameter.Accept(writer);
            param_map[stringId] = buffer.GetString();
            
            //Keep track of which int id matches to which string id
            id_to_string_id_map[intId] = stringId;
            string_id_to_id_map[stringId] = intId;
        }
    }
}

int ParamCache::getIdForStringId(const std::string &string_id)
{
    if (string_id_to_id_map.find(string_id) != string_id_to_id_map.end())
    {
        return string_id_to_id_map.at(string_id);
    }
    return getNextAvailableId();
}

int ParamCache::getNextAvailableId()
{
    while (id_to_string_id_map.find(m_current_id) != id_to_string_id_map.end())
    {
        m_current_id++;
    }
    return m_current_id;
}

bool ParamCache::hasParam(const std::string &param_id) const
{
    return param_map.find(param_id) != param_map.end();
}

ParameterPtr ParamCache::getParam(const std::string &param_id) const
{
    if (hasParam(param_id))
    {
        return ParamfromJson(param_map.at(param_id));
    }
    return nullptr;
}

void ParamCache::save()
{
    std::string json = "{ \"parameters\": [";
    bool first = true;
    for (auto& entry : param_map) 
    {
        if (first)
        {
            json += entry.second;
            first = false;
        }
        else
            json += "," + entry.second;
    }
    json += "]}";

    ApiConnection api_connection;
    api_connection.setData(m_cache_id, json);
}

void ParamCache::addParam(const ParameterPtr param)
{
    if (param)
    {
        std::string stringId = param->getStringId();
        const auto &json = param->asJson();
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        json.Accept(writer);
        param_map[stringId] = buffer.GetString();
        
        //Keep track of which int id matches to which string id
        id_to_string_id_map[param->getID()] = stringId;
        string_id_to_id_map[stringId] = param->getID();
    }
}
