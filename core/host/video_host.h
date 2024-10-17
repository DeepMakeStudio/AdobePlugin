#ifndef VIDEO_HOST_H
#define VIDEO_HOST_H

#include "video_host_delegate.h"
#include "video_filter.h"
#include "draw_helper.h"
#include "../video_plugin_version.h"

class VideoHost
{   
public:
    VideoHost(VideoHostDelegatePtr delegate, VideoFilterPtr filter);

    bool registerParams(bool addToHost);
    bool updateParams(int changedParamId);
    void shutdown();
    bool render();
    bool handleOverlayEvent(MouseEventType eventType, int x, int y);
    bool handleOverlayDraw();

    int numOfParams() const {return m_numOfParams;}
    int paramIDFromHostIndex(int hostIndex) const;
    ParamValue getParamValue(int paramId) const;
    ArkImagePtr sourceImg() const {return m_delegate->sourceImg();}
    ArkImagePtr destImg() const {return m_delegate->destImg();}
    
    ArkImagePtr getImgAtFrame(float curTimeStep);
    std::string getTextPrompt();
    void setTextPrompt(const std::string &prompt);
    
    std::string getRenderedImageID();
    void setRenderedImageID(const std::string &img_id);
    
    std::string getCachedParams();
    void setCachedParams(const std::string &params);

    bool getCachedLicenseStatus() const;
    void setCachedLicenseStatus(bool status);

    void enableDisableControl(int id, bool enable);
    void hideShowControl(int id, bool hide);
    VideoHostDelegatePtr getDelegate() {return m_delegate;}
    DrawHelper& getDrawHelper() {return m_delegate->getDrawHelper();}
    std::string m_base_url = "http://localhost:8000/";//I think this should be in the config file
    
protected:
    bool addParameterToHost(const ParameterPtr param);
    bool addIntSlider(const ParameterPtr param);
    bool addFloatSlider(const ParameterPtr param);
    bool addBool(const ParameterPtr param);
    bool addButton(const ParameterPtr param);
    bool addText(const ParameterPtr param);
    bool addMenu(const ParameterPtr param);
    bool addPoint(const ParameterPtr param);
    bool addColor(const ParameterPtr param);
    bool addGroupStart(const ParameterPtr param);
    bool addGroupEnd(const ParameterPtr param);

    void handleLoginButtonPressed();
    void handleLogoutButtonPressed();
    
    void createPersistentBtnParam(const int paramId, std::string DisplayName, std::string stringId, 
                                         bool bhasCustomDisplayName, bool bstartsHidden, 
                                        std::vector<ParameterPtr> &filterParams, int emplaceIndex);
    const int m_pluginManagerBtnID = 100000;
    const int m_logginBtnID = 100200;
    const int m_logoutBtnID = 100300;
    const int m_userTextID = 100400;
    const int m_configMenuBtnID = 100500;
    const int m_supporFilterBtnID = 100600;
    const int m_issueReportBtnID = 100700;
    const int m_updateBtnID = 100800;

    VideoHostDelegatePtr m_delegate;
    VideoFilterPtr m_filter;
    int m_numOfParams {0};
    std::string cachedUsername = "TestUser@Deepmake.com";
    std::string selectedFilterName;
    static bool &bIsLoggedIn()
    {
        static bool bIsLoggedIn = false;
        return bIsLoggedIn;
    }

    static void setIsLoggedIn(bool value)
    {
        bIsLoggedIn() = value;
    }
};

#endif // VIDEO_HOST_H
