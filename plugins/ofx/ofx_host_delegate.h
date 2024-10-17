#include "video_host_delegate.h"
#include "ofxCore.h"
#include "ofxProperty.h"
#include "ofxImageEffect.h"
#include <stack>

class OFXVideoHostDelegate : public VideoHostDelegate
{   
public:
    OFXVideoHostDelegate(OfxPropertySuiteV1* propHost, 
                            OfxImageEffectSuiteV1* effectSuite,
                            OfxParameterSuiteV1* paramSuite,
                            OfxImageEffectHandle instance);
    virtual ~OFXVideoHostDelegate() = default;

    virtual int projectWidth() const override;
    virtual int projectHeight() const override;
    virtual int downsampleX() const override;
    virtual int downSampleY() const override;
    virtual float projectFPS() const override;
    virtual int durationFrames() const override;
    virtual int currentFrame() const override;

    virtual bool registerParam(const ParameterPtr param) override;
    virtual ParamValue getParamValue(int paramFilterIndex) const override;
    virtual int hostIndexFromParamId(int paramId) const override;
    virtual int paramIdFromHostIndex(int hostIndex) const override;

    virtual std::string getTextPrompt() override;
    virtual void setTextPrompt(const std::string &prompt) override;
    
    virtual std::string getRenderedImageID() override;
    virtual void setRenderedImageID(const std::string &img_id) override;

    virtual std::string getCachedParams() override;
    virtual void setCachedParams(const std::string &params) override;
    
    virtual ArkImagePtr sourceImg() override;
    virtual ArkImagePtr destImg() override;
    virtual ArkImagePtr getImgAtFrame(int frame) override;
    virtual bool addFloatSlider(int id, 
        const std::string &paramName, 
        float defaultValue,
        float min,
        float max) override;

    virtual bool addIntSlider(int id, 
        const std::string &paramName, 
        int defaultValue,
        int min,
        int max) override;

    virtual bool addCheckbox(int id, 
        const std::string &paramName, 
        bool defaultValue) override;

    virtual bool addButton(int id, 
        const std::string &paramName,
        const std::string &paramStringId,
        bool bIsCustomDisplayName = false,
        bool bStartsHidden = false) override;

    virtual bool addPoint(int id, 
        const std::string &paramName, 
        const Point2D &defaultValue) override;

    virtual bool addColor(int id, 
        const std::string &paramName, 
        const Color &defaultValue) override;

    virtual bool addMenu(int id, 
        const std::string &paramName, 
        const std::vector<std::string> &menuItems,
        short defaultIndex) override;

    virtual bool startGroup(int id, 
        const std::string &paramName,
        bool start_collapsed = false) override;
        
    virtual bool endGroup(int id, 
        const std::string &paramName) override;
    
    virtual void hideShowControl(int id, bool hide) override;
    virtual void enableDisableControl(int id, bool enable) override;

    virtual std::string getHostName() override;

protected:
    void addToGroup(OfxPropertySetHandle props);
    
    OfxPropertySuiteV1* m_propHost {nullptr};
    OfxImageEffectSuiteV1* m_effectSuite {nullptr};
    OfxParameterSuiteV1* m_paramSuite {nullptr};
    OfxImageEffectHandle m_instance {nullptr};
    OfxParamSetHandle m_paramSet {nullptr};

    std::stack<std::string> m_groupStack;
};

inline void ofxSuccess(OfxStatus result)
{
    if (result != kOfxStatOK) 
        throw result;
}
