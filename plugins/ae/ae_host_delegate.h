#include "video_host_delegate.h"
#include "ae_video_image.h"
#include <map>
#include <AE_EffectSuites.h>
#include "ae_main.h"
#include "ae_draw_helper.h"

#define PF_CHECKOUT_LAYER 0

class AEVideoHostDelegate : public VideoHostDelegate
{   
    

public:
    AEVideoHostDelegate(PF_Cmd cmd,
        PF_InData *in_data,
        PF_OutData *out_data,
        PF_ParamDef *params[],
        PF_LayerDef *output, 
        PF_EventExtra *extra = nullptr);

    virtual ~AEVideoHostDelegate();

    void init();
    int projectWidth() const override;
    int projectHeight() const override;
    int downsampleX() const override;
    int downSampleY() const override;
    float projectFPS() const override;
    int durationFrames() const override;
    int currentFrame() const override;

    virtual bool registerParam(const ParameterPtr param) override;
    virtual int hostIndexFromParamId(int paramId) const override;
    virtual int paramIdFromHostIndex(int hostIndex) const override;

    ParamValue getParamValue(int paramId) const override;
    
    virtual std::string getTextPrompt() override;
    virtual void setTextPrompt(const std::string &prompt) override;
    
    virtual std::string getRenderedImageID() override;
    virtual void setRenderedImageID(const std::string &img_id) override;

    virtual std::string getCachedParams() override;
    virtual void setCachedParams(const std::string &params) override;

    virtual bool getCachedLicenseStatus() override;
    virtual void setCachedLicenseStatus(bool status) override;
    ArkImagePtr sourceImg() override;
    ArkImagePtr destImg() override;

    virtual ArkImagePtr getImgAtFrame(int frame) override;
    
    bool addFloatSlider(int id, 
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
        const std::string &paramName, bool start_collapsed) override;
        
    virtual bool endGroup(int id, 
        const std::string &paramName) override;

    virtual void hideShowControl(int id, bool hide) override;
    virtual void enableDisableControl(int id, bool enable) override;

    virtual DrawHelper& getDrawHelper() override;

    virtual std::string getHostName() override;

protected:
    float ConvertPF_FixedToFloat(PF_Fixed fixedValue) const;

    PF_Cmd cmd;
    PF_InData *in_data {nullptr};
    PF_OutData *out_data {nullptr};
    PF_ParamDef **params {nullptr};
    PF_LayerDef *output {nullptr};
    PF_EventExtra *extra {nullptr};

    PF_ParamUtilsSuite3 *m_param_utils_suite3 {nullptr};
    ArkImagePtr m_sourceImg {nullptr};
    ArkImagePtr m_destImg {nullptr};

    std::vector<ParameterPtr> m_params;
    std::map<int, int> m_paramIndexMap; //paramId, hostParamIndex
    
    ARK_SeqData *checkoutSequenceData();
    void checkinSequenceData();
    ARK_SeqData *in_seq_data {nullptr};
    AEDrawHelper m_drawHelper;

    ArkImagePtr checkOutImg(int frame);
};

using AEVideoHostDelegatePtr = std::shared_ptr<AEVideoHostDelegate>;
