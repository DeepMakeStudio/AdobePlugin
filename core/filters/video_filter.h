#ifndef VIDEO_FILTER_H
#define VIDEO_FILTER_H

#include <string>
#include <vector>
#include "parameter.h"
#include "param_cache.h"

class VideoHost;

class VideoFilter 
{
 public:
    VideoFilter() = default;
    virtual ~VideoFilter() = default;

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual const std::string &filterName() const = 0;
    virtual const std::string &filterCategory() const = 0;
    virtual const std::string &filterId() const = 0;
    const std::vector<ParameterPtr> &parameters() const {return m_parameters;}

    virtual bool render(VideoHost &host) = 0;
    virtual bool handleOverlayEvent(VideoHost &host, MouseEventType eventType, int x, int y) = 0;
    virtual bool handleOverlayDraw() = 0;
    virtual bool updateParams(VideoHost &host, int changedParamId) = 0;
    virtual void saveParamCache() = 0;
    virtual ParamCache &paramCache() = 0;

    virtual ParameterPtr getParamWithStringId(const std::string &stringId)
    {
        for (auto &param : m_parameters)
        {
            if (param->getStringId() == stringId)
            {
                return param;
            }
        }
        return nullptr;
    }

    virtual ParameterPtr getParam(int intId)
    {
        for (auto &param : m_parameters)
        {
            if (param->getID() == intId)
            {
                return param;
            }
        }
        return nullptr;
    }
    virtual const std::vector<ParameterPtr> getParams() const { return m_parameters; }
    virtual void addParam(ParameterPtr param) { m_parameters.push_back(param); };

    
    virtual void addPersistentParam(ParameterPtr param) = 0;
    virtual void clearPersistentParams() = 0;
    bool bIsLoggedIn = false;
protected:
    virtual void initParameters() = 0;
private:
    std::vector<ParameterPtr> m_parameters;
};

using VideoFilterPtr = std::shared_ptr<VideoFilter>;

#endif // VIDEO_FILTER_H
