#include "main_api_connection.h"
#include "video_host.h"
#include "logger.h"
#include "cpr/cpr.h"
#include "ai_renderer_video_filter.h"
#include <utils.h>

#define K_SELECT_A_PLUGIN "Select A Plugin"
#define K_NONE "None"
#define K_SUPPORT_URL "https://deepmake.com/support/"

VideoHost::VideoHost(VideoHostDelegatePtr delegate, VideoFilterPtr filter)
    : m_delegate(delegate), m_filter(filter)
{
}

 bool VideoHost::render()
 {
    LOG_ASSERT(m_delegate != nullptr, "delegate is null");
    LOG_ASSERT(m_filter != nullptr, "Filter is null");
    return m_filter->render(*this);
}

bool VideoHost::handleOverlayEvent(MouseEventType eventType, int x, int y)
{
    LOG_ASSERT(m_delegate != nullptr, "delegate is null");
    LOG_ASSERT(m_filter != nullptr, "Filter is null");
    return m_filter->handleOverlayEvent(*this, eventType, x, y);
}

bool VideoHost::handleOverlayDraw()
{
    LOG_ASSERT(m_delegate != nullptr, "delegate is null");
    LOG_ASSERT(m_filter != nullptr, "Filter is null");
    return m_filter->handleOverlayDraw();
}

ParamValue VideoHost::getParamValue(int paramId) const
{
    LOG_ASSERT(m_delegate != nullptr, "delegate is null");
    return m_delegate->getParamValue(paramId);
}

int VideoHost::paramIDFromHostIndex(int hostIndex) const
{
    LOG_ASSERT(m_delegate != nullptr, "delegate is null");
    return m_delegate->paramIdFromHostIndex(hostIndex);
}

bool VideoHost::registerParams(bool addToHost)
{
    LOG_ASSERT(m_delegate != nullptr, "delegate is null");
    LOG_ASSERT(m_filter != nullptr, "Filter is null");
    ApiConnection api_connection;
    int emplaceIndex = 1;
    ParameterPtr persistentParam;
    std::shared_ptr<AIRendererFilter> rendererFilter = std::dynamic_pointer_cast<AIRendererFilter>(m_filter);

    if (m_delegate && m_filter)
    {
        m_filter->clearPersistentParams();
        std::vector<ParameterPtr> filterParams = m_filter->parameters();
        if (cpr::Get(cpr::Url{"https://deepmake.com/version"}).error.code == cpr::ErrorCode::OK)
        {
            if (kVideoPluginVersionString != cpr::Get(cpr::Url{"https://deepmake.com/version"}).text)
            {
                createPersistentBtnParam(m_updateBtnID, "New Version Detected", "Update", true, false, filterParams, filterParams.size());
            }
        }

        if (rendererFilter && rendererFilter->IsBackendStarted())
        {
            createPersistentBtnParam(m_pluginManagerBtnID, "Plugin Manager", "Manage", true, false, filterParams, 0);
            emplaceIndex += 1;
            if (!bIsLoggedIn())
            {
                cachedUsername = api_connection.getUserInfo();
                setIsLoggedIn(api_connection.getLoginStatus());
            }
            LogInfo("logged in = " + std::to_string(bIsLoggedIn()));

            cachedUsername = cachedUsername.empty() ? "Couldnt Retrieve User" : cachedUsername;
            bool show = bIsLoggedIn();
            //LogInfo("should show: " + show ? "User is Logged in" : "User is not Logged in");

            createPersistentBtnParam(m_logginBtnID, "Please Login", "Login", true, show, filterParams, 0);
            LogInfo("adding login button to params");

            createPersistentBtnParam(m_logoutBtnID, "Logout", "Logout", true, !show, filterParams,0);
            LogInfo("adding logout button to params");
            emplaceIndex += 2;

           // createPersistentBtnParam(m_userTextIndex, "User:", cachedUsername, true, !show, filterParams, 0);
           // emplaceIndex += 1;

            createPersistentBtnParam(m_configMenuBtnID, "Plugin Configuration", "Configure " + selectedFilterName, true, false, filterParams, emplaceIndex);

            createPersistentBtnParam(m_issueReportBtnID, "Report Issue", "Report Now", true, false, filterParams, filterParams.size());
        }
        createPersistentBtnParam(m_supporFilterBtnID, "DeepMake Support", selectedFilterName + " Support", true, false, filterParams, filterParams.size());

        std::unordered_set<std::string> filterHostParams;
        for (const auto &param : filterParams)
        {
            m_delegate->registerParam(param);
            if (addToHost)
            {
                addParameterToHost(param);
                filterHostParams.insert(param->getStringId());
                LogInfo("Registered Param: " + param->getDisplayName() + " ID: " + std::to_string(param->getID()));
            }
        }

        ParamCache &paramCache = m_filter->paramCache();
        const std::unordered_map<std::string, std::string> &paramMap = paramCache.paramMap();

        for (auto &it : paramMap)
        {
            if (filterHostParams.find(it.first) == filterHostParams.end())
            {
                ParameterPtr param = ParamfromJson(it.second);
                if (param)
                {
                    if (addToHost)
                        addParameterToHost(param);
                    filterHostParams.insert(param->getStringId());
                }
            }
        }
        if (addToHost)
            m_filter->saveParamCache();

        return true;
    }
    return false;
}

bool VideoHost::updateParams(int changedParamHostIndex)
{
    LOG_ASSERT(m_filter != nullptr, "Filter is null");
    std::shared_ptr<AIRendererFilter> rendererFilter = std::dynamic_pointer_cast<AIRendererFilter>(m_filter);

    if (m_filter)
    {
        registerParams(false);
        int param_id = paramIDFromHostIndex(changedParamHostIndex);

        if (rendererFilter && rendererFilter->IsBackendStarted())
        {
            ApiConnection api_connection;
            if (param_id == m_updateBtnID)
            {
                api_connection.openDownloadURL();
                return true;
            }
            if (param_id == m_pluginManagerBtnID)
            {
                api_connection.openPluginManagerMenu();
                LogInfo("Plugin Manager Button Pressed");
                return true;
            }
            if (param_id == m_configMenuBtnID)
            {
                api_connection.openConfigMenu(rendererFilter->selectedFilterName());
                return true;
            }
            if (param_id == m_logoutBtnID)
            {
                handleLogoutButtonPressed();
                return true;
            }
            if (param_id == m_logginBtnID)
            {
                handleLoginButtonPressed();
                return true;
            }
            if (param_id == m_issueReportBtnID)
            {
                api_connection.openReportIssueMenu();
                return true;
            }
        }
        if (param_id == m_supporFilterBtnID)
        {
            if (rendererFilter->selectedFilterName() != K_SELECT_A_PLUGIN && rendererFilter->selectedFilterName() != K_NONE)
            {
                openURL(K_SUPPORT_URL + rendererFilter->selectedFilterName());
                LogInfo("attempting to open URL: https://deepmake.com/support/" + rendererFilter->selectedFilterName());
            }
            else
            {
                openURL(K_SUPPORT_URL);
                LogInfo("attempting to open URL: https://deepmake.com/support/");
            }
        }
        return m_filter->updateParams(*this, param_id);
    }
    return false;
}

void VideoHost::shutdown()
{
    LOG_ASSERT(m_filter != nullptr, "Filter is null");
    if (m_filter)
    {
        LOGGER_H::LogDebug("VideoHost::shutdown()");
        m_filter->shutdown();
    }
}

ArkImagePtr VideoHost::getImgAtFrame(float curTimeStep)
{
    if (m_delegate)
    {
        return m_delegate->getImgAtFrame(curTimeStep);
    }
    return ArkImagePtr();
}
std::string VideoHost::getTextPrompt()
{
    std::string ret_string;
    LOG_ASSERT(m_delegate != nullptr, "delegate is null");
    if (m_delegate)
    {
        ret_string = m_delegate->getTextPrompt();
    }
    return ret_string;
}

void VideoHost::setTextPrompt(const std::string &prompt)
{
    LOG_ASSERT(m_delegate != nullptr, "delegate is null");
    if (m_delegate)
    {
        m_delegate->setTextPrompt(prompt);
    }
}

std::string VideoHost::getRenderedImageID()
{
    std::string img_id;
    LOG_ASSERT(m_delegate != nullptr, "delegate is null");
    if (m_delegate)
    {
        img_id = m_delegate->getRenderedImageID();
    }
    return img_id;
}

void VideoHost::setRenderedImageID(const std::string &img_id)
{
    LOG_ASSERT(m_delegate != nullptr, "delegate is null");
    if (m_delegate)
    {
        m_delegate->setRenderedImageID(img_id);
    }
}

std::string VideoHost::getCachedParams()
{
    std::string params;
    LOG_ASSERT(m_delegate != nullptr, "delegate is null");
    if (m_delegate)
    {
        params = m_delegate->getCachedParams();
    }
    return params;
}

void VideoHost::setCachedParams(const std::string &params)
{
    LOG_ASSERT(m_delegate != nullptr, "delegate is null");
    if (m_delegate)
    {
        m_delegate->setCachedParams(params);
    }
}

bool VideoHost::getCachedLicenseStatus() const
{
    bool status = 0;
    LOG_ASSERT(m_delegate != nullptr, "delegate is null");
    if (m_delegate)
    {
        status = m_delegate->getCachedLicenseStatus();
    }
    return status;
}

void VideoHost::setCachedLicenseStatus(bool status)
{
    LOG_ASSERT(m_delegate != nullptr, "delegate is null");
    if (m_delegate)
    {
        m_delegate->setCachedLicenseStatus(status);
    }
}

void VideoHost::enableDisableControl(int id, bool enable)
{
    LOG_ASSERT(m_delegate != nullptr, "delegate is null");
    if (m_delegate)
    {
        m_delegate->enableDisableControl(id, enable);
    }
}

void VideoHost::hideShowControl(int id, bool hide)
{
    LOG_ASSERT(m_delegate != nullptr, "delegate is null");
    if (m_delegate)
    {
        m_delegate->hideShowControl(id, hide);
    }
}

bool VideoHost::addParameterToHost(const ParameterPtr param)
{
    bool success {false};
    LOG_ASSERT(param != nullptr, "param is null");
    ParameterType paramType = param->getType();

    if (paramType == ParameterType::IntSlider)
    {
        success = addIntSlider(param);
    }
    else if (paramType == ParameterType::FloatSlider)
    {
        success = addFloatSlider(param);
    }
    else if (paramType == ParameterType::Boolean)
    {
        success = addBool(param);
    }
    else if (paramType == ParameterType::Button)
    {
        success = addButton(param);
    }
    else if (paramType == ParameterType::Text)
    {
        success = addText(param);
    }
    else if (paramType == ParameterType::Point2D)
    {
        success = addPoint(param);
    }
    else if (paramType == ParameterType::Color)
    {
        success = addColor(param);
    }
    else if (paramType == ParameterType::Menu)
    {
        success = addMenu(param);
    }
    else if (paramType == ParameterType::GroupStart)
    {
        success = addGroupStart(param);
    }
    else if (paramType == ParameterType::GroupEnd)
    {
        success = addGroupEnd(param);
    }
    else if (paramType == ParameterType::Menu)
    {
        success = addMenu(param);
    }
    else
    {
        LogError("Failed to Add Unknown Param Type: " + param->getDisplayName());
    }
    return success;
}

bool VideoHost::addIntSlider(const ParameterPtr param)
{
    bool success{false};
    if (auto intParam = std::dynamic_pointer_cast<NumberParam<int>>(param))
    {
        if (m_delegate->addIntSlider(param->getID(),
                                     param->getDisplayName(),
                                     intParam->getDefaultValue(),
                                     intParam->getMinValue(),
                                     intParam->getMaxValue()))
        {
            LogInfo("Added Int Slider: " + param->getDisplayName());
            m_numOfParams++;
            success = true;
        }
        else
        {
            LogError("Failed to Add Int Slider1: " + param->getDisplayName());
        }
    }
    else
    {
        LogError("Failed to Add Int Slider2: " + param->getDisplayName());
    }
    return success;
}

bool VideoHost::addFloatSlider(const ParameterPtr param)
{
    bool success{false};
    if (auto floatParam = std::dynamic_pointer_cast<NumberParam<float>>(param))
    {
        if (m_delegate->addFloatSlider(param->getID(),
                                       param->getDisplayName(),
                                       floatParam->getDefaultValue(),
                                       floatParam->getMinValue(),
                                       floatParam->getMaxValue()))
        {
            LogInfo("Added Float Slider: " + param->getDisplayName());
            m_numOfParams++;
            success = true;
        }
        else
        {
            LogError("Failed to Add Float Slider1: " + param->getDisplayName());
        }
    }
    else
    {
        LogError("Failed to Add Float Slider2: " + param->getDisplayName());
    }
    return success;
}

bool VideoHost::addBool(const ParameterPtr param)
{
    bool success{false};
    if (auto boolParam = std::dynamic_pointer_cast<NumberParam<bool>>(param))
    {
        if (m_delegate->addCheckbox(param->getID(),
                                    param->getDisplayName(),
                                    boolParam->getDefaultValue()))
        {
            LogInfo("Added Checkbox: " + param->getDisplayName());
            m_numOfParams++;
            success = true;
        }
        else
        {
            LogError("Failed to Add Checkbox1: " + param->getDisplayName());
        }
    }
    else
    {
        LogError("Failed to Add Checkbox2: " + param->getDisplayName());
    }
    return success;
}

bool VideoHost::addButton(const ParameterPtr param)
{
    bool success{false};
    if (param)
    {
        if (m_delegate->addButton(param->getID(),
                                  param->getDisplayName(), param->getStringId(), param->isCustomDisplayName(), param->startsHidden()))
        {
            m_numOfParams++;
            success = true;
        }
        else
        {
            LogError("Failed to Add Button1: " + param->getDisplayName());
        }
    }
    else
    {
        LogError("Failed to Add Button2: " + param->getDisplayName());
    }
    return success;
}

bool VideoHost::addText(const ParameterPtr param)
{
    addButton(param);
    return true;
}

bool VideoHost::addPoint(const ParameterPtr param)
{
    bool success{false};
    if (auto pointParam = std::dynamic_pointer_cast<PointParam>(param))
    {
        if (m_delegate->addPoint(param->getID(),
                                 param->getDisplayName(),
                                 pointParam->getDefaultValue()))
        {
            LogInfo("Added Point: " + param->getDisplayName());
            m_numOfParams++;
            success = true;
        }
        else
        {
            LogError("Failed to Add Checkbox1: " + param->getDisplayName());
        }
    }
    else
    {
        LogError("Failed to Add Checkbox2: " + param->getDisplayName());
    }
    return success;
}

bool VideoHost::addColor(const ParameterPtr param)
{
    bool success{false};
    if (auto colorParam = std::dynamic_pointer_cast<ColorParam>(param))
    {
        if (m_delegate->addColor(param->getID(),
                                 param->getDisplayName(),
                                 colorParam->getDefaultValue()))
        {
            LogInfo("Added Color: " + param->getDisplayName());
            m_numOfParams++;
            success = true;
        }
        else
        {
            LogError("Failed to Add Checkbox1: " + param->getDisplayName());
        }
    }
    else
    {
        LogError("Failed to Add Checkbox2: " + param->getDisplayName());
    }
    return success;
}

bool VideoHost::addMenu(const ParameterPtr param)
{
    bool success{false};
    if (auto menuParam = std::dynamic_pointer_cast<MenuParam>(param))
    {
        if (m_delegate->addMenu(param->getID(),
                                param->getDisplayName(),
                                menuParam->getMenuItems(),
                                (short)menuParam->getDefaultIndex()))
        {
            LogInfo("Added Point: " + param->getDisplayName());
            m_numOfParams++;
            success = true;
        }
        else
        {
            LogError("Failed to Add Point: " + param->getDisplayName());
        }
    }
    else
    {
        LogError("Failed to Add Point: " + param->getDisplayName());
    }
    return success;
}

bool VideoHost::addGroupStart(const ParameterPtr param)
{
    bool success{false};
    if (m_delegate->startGroup(param->getID(),
                               param->getDisplayName()))
    {
        LogInfo("Added GroupStart: " + param->getDisplayName());
        m_numOfParams++;
        success = true;
    }
    else
    {
        LogError("Failed to GroupStart: " + param->getDisplayName());
    }
    return success;
}

bool VideoHost::addGroupEnd(const ParameterPtr param)
{
    bool success{false};
    if (m_delegate->endGroup(param->getID(),
                             param->getDisplayName()))
    {
        LogInfo("Added GroupEnd: " + param->getDisplayName());
        m_numOfParams++;
        success = true;
    }
    else
    {
        LogError("Failed to GroupEnd: " + param->getDisplayName());
    }
    return success;
}

void VideoHost::handleLoginButtonPressed()
{
    // Declare ApiConnection outside the lambda
    ApiConnection api_connection;

    // Start the login process asynchronously
    std::future<bool> loginFuture = std::async(std::launch::async, [&](){
        api_connection.openLoginMenu();
        int checkCount = 0;
        while (!api_connection.getLoginStatus() && checkCount < 60)
        {
            checkCount++;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        return api_connection.getLoginStatus();
    });

    // Wait for the login process to complete or timeout
    if (loginFuture.wait_for(std::chrono::seconds(60)) == std::future_status::ready)
    {
        bool isLoggedIn = loginFuture.get();
        if (isLoggedIn)
        {
           // cachedUsername = api_connection.getUserInfo();
            arkSleepMS(std::chrono::seconds(1));
            setIsLoggedIn(true);
            m_filter->bIsLoggedIn = true;
            m_filter->updateParams(*this,0);
            hideShowControl(m_logginBtnID, true);
            hideShowControl(m_logoutBtnID, false);
            SetLogUserData(api_connection.getUserInfo());
            // hideShowControl(m_userTextIndex, false);
        }
        else
        {
            LogError("Login Failed", true);
        }
    }
    else
    {
        LogError("Login Timed Out", true);
    }
}

void VideoHost::handleLogoutButtonPressed()
{
    cpr::Response response = cpr::Get(cpr::Url{m_base_url + "login/logout"});
    if (response.status_code == 200)
    {
        LogInfo("Logout Successful = " + std::to_string(bIsLoggedIn()));
        setIsLoggedIn(false);
        m_filter->bIsLoggedIn = false;
        m_filter->updateParams(*this,0);
        hideShowControl(m_logoutBtnID, true);
        // hideShowControl(m_userTextIndex, true);
        hideShowControl(m_logginBtnID, false);
    }

    else
    {
        LogError("Logout Failed = " + std::to_string(bIsLoggedIn()));
    }
}

void VideoHost::createPersistentBtnParam(const int paramId, std::string DisplayName, std::string stringId,
                                         bool bhasCustomDisplayName, bool bstartsHidden,
                                         std::vector<ParameterPtr> &filterParams, int emplaceIndex)
{
    ParameterPtr persistentParam = createButtonParam(paramId, DisplayName, stringId, bhasCustomDisplayName, bstartsHidden);
    filterParams.emplace(filterParams.begin() + emplaceIndex, persistentParam);
    m_filter->addPersistentParam(persistentParam);
}
