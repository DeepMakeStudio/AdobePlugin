#ifndef PARAM_CACHE_H
#define PARAM_CACHE_H

#include "video_host_delegate.h"
#include "parameter.h"

#define DEFAULT_STARTING_ID 10000

class ParamCache
{   
public:
    ParamCache(const std::string &cache_id);

    bool initialized() const;
    void init();
    void save();
    void addParam(const ParameterPtr param);
    bool hasParam(const std::string &param_id) const;
    ParameterPtr getParam(const std::string &param_id) const;
    void resetIds() { m_current_id = DEFAULT_STARTING_ID; };
    int getNextAvailableId();

    int getIdForStringId(const std::string &string_id);

    const std::unordered_map<std::string, std::string> & paramMap() const { return param_map; }

protected:

    int m_current_id {DEFAULT_STARTING_ID};
    std::string m_cache_id;
    std::unordered_map<std::string, std::string> param_map;
    std::unordered_map<int, std::string> id_to_string_id_map;
    std::unordered_map<std::string, int> string_id_to_id_map;
};

#endif
