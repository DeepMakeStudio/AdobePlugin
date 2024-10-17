#ifndef __AI_RENDERER_VIDEO_FILTER_H__
#define __AI_RENDERER_VIDEO_FILTER_H__

#include "video_filter.h"
#include "ark_plugin.h"
#include <map>
#include "plugin_defs.h"
#include "param_cache.h"
#ifdef _WIN32   
#include <chrono>
#endif

class FilterConfig
{
    public: 
        FilterConfig() = default;
        FilterConfig(const std::string &name) 
        : m_name(name)
        { }

        ArkPlugin &plugin() { return m_plugin; }

        void addParameter(ParameterPtr parameter)
        {
            m_parameters.push_back(parameter);
        }

        const std::string &name() const { return m_name; }
        bool containsParameter(int id) const
        {
            //TODO: this is a linear search, should be a map
            for (const auto &param : m_parameters)
            {
                if (param->getID() == id)
                {
                    return true;
                }
            }
            return false;
        }
        int endpoint_param_id = -1;
        int required_license = 0;

    protected:
        ArkPlugin m_plugin;
        std::string m_name;
        std::vector<ParameterPtr> m_parameters;
};

class VideoFilterManager;
class AIRendererFilter : public VideoFilter
{
public:

 #define K_TIMEOUT_MAX_ATTEMPTS  1

    AIRendererFilter() = default;
    virtual ~AIRendererFilter() = default;
    
    bool initialize() override;
    void shutdown() override;

    const std::string &filterName() const override { return m_name; };
    const std::string &filterCategory() const override { return m_category; };
    const std::string &filterId() const override { return m_id; };
    const std::string &selectedFilterName() const { return m_selected_filter_name; };
    virtual bool render(VideoHost &host) override;
    virtual bool handleOverlayEvent(VideoHost &host, MouseEventType eventType, int x, int y) override;    
    virtual bool handleOverlayDraw() override;
    virtual bool updateParams(VideoHost &host, int changedParamID) override;
    virtual void saveParamCache() override;

    virtual void addParam(ParameterPtr param) override;
    virtual ParamCache &paramCache() override;
    virtual ParameterPtr getParamWithStringId(const std::string&stringId) override;
    virtual void addPersistentParam(ParameterPtr param) override;
    virtual void clearPersistentParams() override {m_persistent_params.clear();};
    std::map<std::string,std::string> m_imageMap;
    bool IsBackendStarted();
protected:

     
    virtual void initParameters() override;
    void hideAllParams(VideoHost &host);
    void invalidateRenderedImage(VideoHost &host);
    void fetchFilterConfigs();
    void addFilterMenu();
    void addRegenerateButton();
    void addFilterParams();
    void addMissingParams();
    bool addEndpointMenu(FilterConfig &filter_config);
    bool addEndpointParams(FilterConfig &filter_config);
    void AddConfigGroupStart(FilterConfig &filter_config);
    void AddConfigGroupEnd(FilterConfig &filter_config);
    void syncParamWithCachedParam(ParameterPtr param);
    void handleMultiImageRequest(const std::vector<std::string> &img_params, VideoHost &host, Endpoint &endpoint);
    void handleImageRequest(const std::string &img_param, Endpoint &endpoint, ArkImagePtr sourceImg);
    bool shouldShowPromptUI(int changedParamID);
    bool showPromptUI(VideoHost &host, int textParamID);

    bool getSelectedFilterConfig(VideoHost &host, FilterConfig &filter_config);
    bool getSelectedEndpoint(VideoHost &host, FilterConfig &filter_config, Endpoint &out_endpoint);
    bool getEndpointParams(VideoHost &host, Endpoint &out_endpoint, std::string &params);
    bool hasBackendStartupTimedout(int maxAttempts);
    
    //eventually these should be pulled from the filter defs for each filter
    std::string m_name { kAIRendererPluginName };
    std::string m_category { kAIRendererCategoryName };
    std::string m_id { kAIREndererMatchName };
    std::string m_selected_filter_name;
    //Temp hack to avoid calling the get_info endpoint just to build params
    static std::vector<FilterConfig> s_filter_configs; //index matches the plugin menu index(should be a map of API filter IDs)

    static ParamCache s_param_cache;
    
    std::chrono::seconds cachedElasedTime = std::chrono::seconds(10);
    void handleCrashCheck();

private:
    static int &startupAttempts()
    {
        static int attempts = 0;
        return attempts;
    }
    static void incrementStartupAttempts()
    {
        startupAttempts()++; // Increment the static variable
    }

    static bool &checkForCrash()
    {
        static bool checked = false;
        return checked;
    }

    static void setCheckForCrash(bool value)
    {
        checkForCrash() = value;
    }

    static bool &isBackendStarted()
    {
        static bool bIsBackendStarted = false;
        return bIsBackendStarted;
    }

    static void setBackendStarted(bool value)
    {
        isBackendStarted() = value;
    }

    bool bIsBackendStarted = false;
    std::vector<ParameterPtr> m_persistent_params;
};

#endif // __AI_RENDERER_VIDEO_FILTER_H__
