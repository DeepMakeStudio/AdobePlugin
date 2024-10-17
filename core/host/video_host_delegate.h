#ifndef _CORE_HOST_VIDEO_HOST_DELEGATE_H
#define _CORE_HOST_VIDEO_HOST_DELEGATE_H


#include <memory>
#include "parameter.h"
#include "images/ark_image.h"
#include "draw_helper.h"
#include <string>

class VideoHostDelegate
{
public:
    virtual int projectWidth() const = 0;
    virtual int projectHeight() const = 0;
    virtual int downsampleX() const = 0;
    virtual int downSampleY() const = 0;
    //duplicate these two and return cur width/height
    virtual float projectFPS() const = 0;
    virtual int durationFrames() const = 0;
    virtual int currentFrame() const = 0;

    virtual bool registerParam(const ParameterPtr param) = 0;
    virtual ParamValue getParamValue(int paramFilterIndex) const = 0;
    virtual int hostIndexFromParamId(int paramId) const = 0;
    virtual int paramIdFromHostIndex(int hostIndex) const = 0;

    virtual std::string getTextPrompt() = 0;
    virtual void setTextPrompt(const std::string &prompt) = 0;
    
    virtual std::string getRenderedImageID() = 0;
    virtual void setRenderedImageID(const std::string &img_id) = 0;

    virtual std::string getCachedParams() = 0;
    virtual void setCachedParams(const std::string &params) = 0;
    
    virtual bool getCachedLicenseStatus() = 0;
    virtual void setCachedLicenseStatus(bool status) = 0;

    virtual ArkImagePtr sourceImg() = 0;
    virtual ArkImagePtr destImg() = 0;
    virtual ArkImagePtr getImgAtFrame(int frame) = 0;
    virtual bool addFloatSlider(int id, 
        const std::string &paramName, 
        float defaultValue,
        float min,
        float max) = 0;

    virtual bool addIntSlider(int id, 
        const std::string &paramName, 
        int defaultValue,
        int min,
        int max) = 0;

    virtual bool addCheckbox(int id, 
        const std::string &paramName, 
        bool defaultValue) = 0;

    virtual bool addButton(int id, 
        const std::string &paramName,
        const std::string &paramStringID,
        bool bIsCustomDisplayName = false,
        bool bStartsHidden = false) = 0;

    virtual bool addPoint(int id, 
        const std::string &paramName, 
        const Point2D &defaultValue) = 0;

    virtual bool addColor(int id, 
        const std::string &paramName, 
        const Color &defaultValue) = 0;

    virtual bool addMenu(int id, 
        const std::string &paramName, 
        const std::vector<std::string> &menuItems,
        short defaultIndex) = 0;

    virtual bool startGroup(int id, 
        const std::string &paramName,
        bool start_collapsed = false) = 0;
        
    virtual bool endGroup(int id, 
        const std::string &paramName) = 0;
    
    virtual void hideShowControl(int id, bool hide) = 0;
    virtual void enableDisableControl(int id, bool enable) = 0;

    virtual DrawHelper& getDrawHelper() = 0;

    virtual std::string getHostName() = 0;
};

using VideoHostDelegatePtr = std::shared_ptr<VideoHostDelegate>;

#endif // _CORE_HOST_VIDEO_HOST_DELEGATE_H
