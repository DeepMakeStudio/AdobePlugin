#include <AEConfig.h>
#ifdef AE_OS_WIN
	#include <Windows.h>
#endif

#include <entry.h>

#include <AE_Effect.h>
#include <A.h>
#include <AE_EffectCB.h>	
#include <AE_Macros.h>
#include <AE_EffectCBSuites.h>
#include <Param_Utils.h>
#include <AEGP_SuiteHandler.h>
#include "ae_host_delegate.h"
#include "utils.h"
#include <string>
#include <logger.h>
#include "ae_main.h"
#include "ae_video_image_checkout.h"

AEVideoHostDelegate::AEVideoHostDelegate(PF_Cmd cmd,
    PF_InData *in_data,
    PF_OutData *out_data,
    PF_ParamDef *params[],
    PF_LayerDef *output,
    PF_EventExtra *extra)
        : cmd(cmd)
        , in_data(in_data)
        , out_data(out_data)
        , params(params)
        , output(output)
        , extra(extra)
        , m_drawHelper(in_data, out_data, extra)
{
    init();
}

void AEVideoHostDelegate::init()
{
    setHostName(getHostName());

    if (cmd == PF_Cmd_RENDER)
    {
        m_sourceImg = std::make_shared<AEVideoImage>(params[0]->u.ld, in_data);
        m_destImg = std::make_shared<AEVideoImage>(*output, in_data);
    }   
    if (in_data && in_data->pica_basicP)
    {
        SPErr err = (in_data->pica_basicP->AcquireSuite)(kPFParamUtilsSuite, kPFParamUtilsSuiteVersion3, (const void**) &m_param_utils_suite3);
        if ( err != kSPNoError ) {
            LOG_ASSERT(false,"Failed to aquire suite"); //failed to acquire suite
        }

        for (auto param : m_params)
        {
            if (param->startsHidden())
            {
                hideShowControl(param->getID(), true);
            }
        }
      
    }
}

AEVideoHostDelegate::~AEVideoHostDelegate()
{
    checkinSequenceData();
    if (in_data && in_data->pica_basicP && m_param_utils_suite3)
    {
        SPErr err = (in_data->pica_basicP->ReleaseSuite)(kPFParamUtilsSuite, kPFParamUtilsSuiteVersion3);
        if ( err != kSPNoError ) {
            LOG_ASSERT(false, "Failed to release suite"); //failed to release suite
        }
    }
}

bool AEVideoHostDelegate::registerParam(const ParameterPtr param)
{
    //could eventually add a check to see if the param is supported in this host
    m_params.push_back(param);
    m_paramIndexMap[param->getID()] = (int)m_paramIndexMap.size() + 1; //+1 because in AE param[0] is the source image
    return true;
}

int AEVideoHostDelegate::projectWidth() const
{
    return in_data->width; //source layer width
}

int AEVideoHostDelegate::projectHeight() const
{
    return in_data->height; //source layer height
}

int AEVideoHostDelegate::downsampleX() const
{
    return in_data->downsample_x.den;
}

int AEVideoHostDelegate::downSampleY() const
{
    return in_data->downsample_y.den;
}

float AEVideoHostDelegate::projectFPS() const //should this be a ratio? 
{
    //time_scale - The units per second that current_time, time_step, local_time_step and total_time are in.
    float fps = float(in_data->time_scale) / float(in_data->local_time_step);
    return fps;
}

int AEVideoHostDelegate::durationFrames() const
{
    return in_data->total_time / in_data->time_step;
}

int AEVideoHostDelegate::currentFrame() const
{
    if (in_data->time_step == 0)
    {
        LogError("Time step is 0");
        return in_data->current_time;
    }
    return in_data->current_time / in_data->time_step;
}

int AEVideoHostDelegate::hostIndexFromParamId(int paramId) const
{
    int hostParamIndex = -1;

    if (m_paramIndexMap.find(paramId) == m_paramIndexMap.end())
    {
        LOG_ASSERT(false,"param not found: " + std::to_string(paramId)); //param not found
        return -1;
    }
    hostParamIndex = m_paramIndexMap.at(paramId);
    if (hostParamIndex < 0 || hostParamIndex > (int)m_params.size())
    {
        LOG_ASSERT(false,"param not found: " + std::to_string(paramId)); //param not found
        return -1;
    }
    
    return hostParamIndex;
}

int AEVideoHostDelegate::paramIdFromHostIndex(int hostIndex) const
{
    int hostParamIndex = -1;

    for (const auto &mapItem : m_paramIndexMap)
    {
        if (mapItem.second == hostIndex)
            return mapItem.first;
    }
    return -1;
}


ParamValue AEVideoHostDelegate::getParamValue(int paramId) const
{
    ParamValue value;
    int hostParamIndex = hostIndexFromParamId(paramId);
    if (hostParamIndex < 0)
    {
        LOG_ASSERT(false,"param not found: " + std::to_string(paramId)); //param not found
        return value;
    }

    PF_ParamDef *param = params[hostParamIndex];
    if (param == nullptr)
    {
        LOG_ASSERT(false,"param not found: " + std::to_string(paramId)); //param not found
        return value;
    }

    switch (param->param_type)
    {
        case PF_Param_BUTTON:
            break;
        case PF_Param_CHECKBOX:
        {
            value = static_cast<bool>(param->u.bd.value);
            break;
        }
        case PF_Param_COLOR:
        {
            value = Color(param->u.cd.value.red / 255.0f,
                            param->u.cd.value.green / 255.0f,
                            param->u.cd.value.blue / 255.0f);
            break;
        }
        case PF_Param_FLOAT_SLIDER:
        {
            value = (float)param->u.fs_d.value;
            break;
        }
        case PF_Param_POPUP:
        {
            value = param->u.pd.value - 1; //menu indexes are 1 based
            break;
        }
        case PF_Param_POINT:
        {
            value = Point2D(ConvertPF_FixedToFloat(param->u.td.x_value), 
                            ConvertPF_FixedToFloat(param->u.td.y_value));
            break;
        }
        default:
        {
            LOG_ASSERT(false,"Param type not supported"); //param type not supported
            break;
        }
    }
    return value;
}

ARK_SeqData *AEVideoHostDelegate::checkoutSequenceData()
{
    if (in_seq_data == nullptr &&
        in_data &&
        in_data->sequence_data)
    {
        in_seq_data = reinterpret_cast<ARK_SeqData*>(PF_LOCK_HANDLE(in_data->sequence_data));
    }
    return in_seq_data;
}

void AEVideoHostDelegate::checkinSequenceData()
{
    if (in_seq_data)
    {
        in_seq_data = nullptr;
        PF_UNLOCK_HANDLE(in_data->sequence_data);
    }
}

ArkImagePtr AEVideoHostDelegate::checkOutImg(int frame)
{
    // Calculate frame time in seconds
    if(in_data->time_scale == 0)
    {
        LogError("Time scale is 0");
        return nullptr;
    }
    A_long frameTime = frame * in_data->time_step;

    // Checkout the image at the calculated frame time
    PF_ParamDef checkout;
    PF_Err err = PF_CHECKOUT_PARAM(in_data,
                                   PF_CHECKOUT_LAYER,
                                   frameTime,
                                   in_data->time_step,
                                   in_data->time_scale,
                                   &checkout);

    // Log information about the checkout process
    LogInfo("Checkout Layer Def at time: " + std::to_string(frameTime) +
            " FPS = " + std::to_string(projectFPS()) +
            " Time Scale = " + std::to_string(in_data->time_scale) +
            " Time Step = " + std::to_string(in_data->time_step));

    // Handle the checkout result
    if (err == PF_Err_NONE)
    {
        // Create and checkout the image
        ArkImagePtr img = std::make_shared<AEVideoImageCheckOut>(checkout, in_data);
        LogInfo("Checkouted out image at frame: " + std::to_string(frame) +
                ":" + std::to_string(durationFrames()));
        return img;
    }
    else
    {
        LogError("Failed to checkout image:" + std::to_string(err));
        return nullptr; // Return nullptr if checkout failed
    }
}

std::string AEVideoHostDelegate::getTextPrompt()
{
    std::string prompt;
    
    ARK_SeqData* seq_data = checkoutSequenceData();
    if (seq_data)
    {
        prompt = seq_data->prompt;
    }
    return prompt;
}

void AEVideoHostDelegate::setTextPrompt(const std::string &prompt)
{
    ARK_SeqData* seq_data = checkoutSequenceData();
    LOG_ASSERT(prompt.size() < kPromptSize,"Prompt size too large");
    if (seq_data && prompt.size() < kPromptSize)
    {
        strcpy(seq_data->prompt, prompt.c_str());
    }
}

std::string AEVideoHostDelegate::getRenderedImageID()
{
    std::string img_id;
    ARK_SeqData* seq_data = checkoutSequenceData();
    if (seq_data)
    {
        img_id = seq_data->rendered_image_id;
    }
    return img_id;
}

void AEVideoHostDelegate::setRenderedImageID(const std::string &img_id)
{
    ARK_SeqData* seq_data = checkoutSequenceData();
    LOG_ASSERT(img_id.size() < kImageIDSize,"Image ID size too large");
    if (seq_data && img_id.size() < kImageIDSize)
    {
        strcpy(seq_data->rendered_image_id, img_id.c_str());
    }
}

std::string AEVideoHostDelegate::getCachedParams()
{
    std::string params;
    ARK_SeqData* seq_data = checkoutSequenceData();
    if (seq_data)
    {
        params = seq_data->cached_params;
    }
    return params;
}

void AEVideoHostDelegate::setCachedParams(const std::string &params)
{
    ARK_SeqData* seq_data = checkoutSequenceData();
    LOG_ASSERT(params.size() < kCachedParamSize,"Cached param size too large");
    if (seq_data && params.size() < kCachedParamSize)
    {
        strcpy(seq_data->cached_params, params.c_str());
    }
}

bool AEVideoHostDelegate::getCachedLicenseStatus()
{
    bool status = false;
    ARK_SeqData* seq_data = checkoutSequenceData();
    if (seq_data)
    {
          status = seq_data->license_status;
    }
    return status;
}

void AEVideoHostDelegate::setCachedLicenseStatus(bool status)
{
    ARK_SeqData* seq_data = checkoutSequenceData();
    if (seq_data && status)
    {
        seq_data->license_status = status;
    }
    checkinSequenceData();
}

ArkImagePtr AEVideoHostDelegate::sourceImg()
{
    LOG_ASSERT(m_sourceImg != nullptr,"Source image NULL");
    return m_sourceImg;
}

ArkImagePtr AEVideoHostDelegate::destImg()
{
    LOG_ASSERT(m_destImg != nullptr,"Dest image NULL");
    return m_destImg;
}

ArkImagePtr AEVideoHostDelegate::getImgAtFrame(int frame)
{
    LogInfo("Getting image at frame: " + std::to_string(frame));
    if (frame < 0 || frame >= durationFrames()) 
    {
        LogInfo("Frame time out of range. Returning null pointer.");
        return nullptr;
    }
    
    // Call checkoutImg to retrieve the image at the specified frame
    ArkImagePtr img = checkOutImg(frame);
    LogInfo("Frame number: " + std::to_string(frame));
    return img;
}

bool AEVideoHostDelegate::addFloatSlider(int id,
       const std::string &paramName,
       float defaultValue,
       float min,
       float max)
{
	PF_Err			err = PF_Err_NONE;
	PF_ParamDef		def;
   AEFX_CLR_STRUCT(def);

#ifdef AE_OS_WIN
	#pragma warning( push )
	#pragma warning( disable : 4127)
#endif

   PF_ADD_FLOAT_SLIDERX(paramName.c_str(),//NAME
                   min, //VALID_MIN
                   max, //VALID_MAX
                   min, //SLIDER_MIN
                   max, //SLIDER_MAX
                   defaultValue, //DFLT
                   2, //PREC
                   0, //DISP
                   0, //FLAGS
                   id); //ID

#ifdef AE_OS_WIN
	#pragma warning( pop )
#endif

   return err == PF_Err_NONE;
}

bool AEVideoHostDelegate::addIntSlider(int id, 
    const std::string &paramName, 
    int defaultValue,
    int min,
    int max)
{
    //Combind with float param
    PF_Err			err = PF_Err_NONE;
	PF_ParamDef		def;	
    AEFX_CLR_STRUCT(def);

#ifdef AE_OS_WIN
	#pragma warning( push )
	#pragma warning( disable : 4127)
#endif

    PF_ADD_FLOAT_SLIDERX(paramName.c_str(),//NAME
                    float(min), //VALID_MIN
                    float(max), //VALID_MAX
                    float(min), //SLIDER_MIN
                    float(max), //SLIDER_MAX
                    float(defaultValue), //DFLT
                    0, //PREC
                    0, //DISP 
                    0, //FLAGS 
                    id); //ID

#ifdef AE_OS_WIN
	#pragma warning( pop ) 
#endif
    
    return err == PF_Err_NONE;
}

bool AEVideoHostDelegate::addCheckbox(int id, 
        const std::string &paramName, 
        bool defaultValue)
{
    PF_Err			err = PF_Err_NONE;
	PF_ParamDef		def;	
    AEFX_CLR_STRUCT(def);

#ifdef AE_OS_WIN
	#pragma warning( push )
	#pragma warning( disable : 4127)
#endif

    PF_ADD_CHECKBOXX(paramName.c_str(),
                    defaultValue,
                    0, //display flags 
                    id);
#ifdef AE_OS_WIN
	#pragma warning( pop ) 
#endif
    
    return err == PF_Err_NONE;
}

bool AEVideoHostDelegate::addButton(int id, 
        const std::string &paramName,
        const std::string &paramStringID,
        bool bIsCustomDisplayName,
        bool bStartsHidden)
{
    PF_Err			err = PF_Err_NONE;
	PF_ParamDef		def;	
    AEFX_CLR_STRUCT(def);

#ifdef AE_OS_WIN
	#pragma warning( push )
	#pragma warning( disable : 4127)
#endif

    PF_ADD_BUTTON(bIsCustomDisplayName? paramStringID.c_str() : paramName.c_str(),
                    paramName.c_str(), 
                    0, 
                    PF_ParamFlag_SUPERVISE, 
                    id);
#ifdef AE_OS_WIN
	#pragma warning( pop ) 
#endif
    return err == PF_Err_NONE;
}

bool AEVideoHostDelegate::addPoint(int id, 
        const std::string &paramName, 
        const Point2D &defaultValue)
{
    PF_Err			err = PF_Err_NONE;
	PF_ParamDef		def;	
    AEFX_CLR_STRUCT(def);

#ifdef AE_OS_WIN
	#pragma warning( push )
	#pragma warning( disable : 4127)
#endif
    err = PF_AddPointControl(in_data,
                paramName.c_str(),
                defaultValue.x,
                defaultValue.y,
				false,
                0,
                0,
                id);
#ifdef AE_OS_WIN
	#pragma warning( pop ) 
#endif
    
    return err == PF_Err_NONE;
}

bool AEVideoHostDelegate::addColor(int id, 
        const std::string &paramName, 
        const Color &defaultValue)
{
    PF_Err			err = PF_Err_NONE;
	PF_ParamDef		def;	
    AEFX_CLR_STRUCT(def);

#ifdef AE_OS_WIN
	#pragma warning( push )
	#pragma warning( disable : 4127)
#endif
    PF_ADD_COLOR(paramName.c_str(), 
        (unsigned char)defaultValue.r * 255, 
        (unsigned char)defaultValue.g * 255, 
        (unsigned char)defaultValue.b * 255,
        id);
#ifdef AE_OS_WIN
	#pragma warning( pop ) 
#endif
    
    return err == PF_Err_NONE;
}

bool AEVideoHostDelegate::addMenu(int id, 
    const std::string &paramName, 
    const std::vector<std::string> &menuItems,
    short defaultIndex)
{
    PF_Err			err = PF_Err_NONE;
	PF_ParamDef		def;	
    AEFX_CLR_STRUCT(def);

#ifdef AE_OS_WIN
	#pragma warning( push )
	#pragma warning( disable : 4127)
#endif

    //Create the menu string that AE expects
    std::string menuString;
    for (const auto &item : menuItems)
    {
        if (!menuString.empty())
            menuString += '|';
        menuString += item;
    }

    //PF_ParamFlag_SUPERVISE tells AE that we want to be called
    //when this parameter changes
    def.flags = PF_ParamFlag_SUPERVISE;
    PF_ADD_POPUP(paramName.c_str(), 
                (short)menuItems.size(),
                defaultIndex,
                menuString.c_str(),
                id);
#ifdef AE_OS_WIN
	#pragma warning( pop ) 
#endif
    
    return err == PF_Err_NONE;
}

bool AEVideoHostDelegate::startGroup(int id, const std::string &paramName, bool start_collapsed)
{
    PF_Err            err = PF_Err_NONE;
    PF_ParamDef        def;
    AEFX_CLR_STRUCT(def);

    if (start_collapsed)
        def.flags = PF_ParamFlag_COLLAPSE_TWIRLY;
        
#ifdef AE_OS_WIN
    #pragma warning( push )
    #pragma warning( disable : 4127)
#endif

    PF_ADD_TOPIC(paramName.c_str(), id);

#ifdef AE_OS_WIN
    #pragma warning( pop )
#endif
    
    return err == PF_Err_NONE;
}
        
bool AEVideoHostDelegate::endGroup(int id, const std::string &paramName)
{
    UNUSED(paramName);
    PF_Err            err = PF_Err_NONE;
    PF_ParamDef        def;
    AEFX_CLR_STRUCT(def);

#ifdef AE_OS_WIN
    #pragma warning( push )
    #pragma warning( disable : 4127)
#endif

    PF_END_TOPIC(id);

#ifdef AE_OS_WIN
    #pragma warning( pop )
#endif
    
    return err == PF_Err_NONE;
}

float AEVideoHostDelegate::ConvertPF_FixedToFloat(PF_Fixed fixedValue) const
{
    return (float)fixedValue / 65536.0f;
}

void AEVideoHostDelegate::hideShowControl(int id, bool hide)
{
    int hostParamIndex = hostIndexFromParamId(id);
    LOG_ASSERT(hostParamIndex > 0,"param not found"); //param not found

    PF_Err err = PF_Err_NONE;
    PF_Err err2 = PF_Err_NONE;
    AEGP_SuiteHandler suites(in_data->pica_basicP);

    ARK_GlobalDataP globP = reinterpret_cast<ARK_GlobalDataP>(DH(out_data->global_data));
    AEGP_EffectRefH meH = NULL;
    AEGP_StreamRefH param_streamH = NULL;

    ERR(suites.PFInterfaceSuite1()->AEGP_GetNewEffectForEffect(globP->plugin_id, in_data->effect_ref, &meH));

    ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(globP->plugin_id, meH, hostParamIndex, &param_streamH));

    ERR(suites.DynamicStreamSuite2()->AEGP_SetDynamicStreamFlag(param_streamH, AEGP_DynStreamFlag_HIDDEN, FALSE, hide));
    
    if (meH){
        ERR2(suites.EffectSuite2()->AEGP_DisposeEffect(meH));
    }
    if (param_streamH){
        ERR2(suites.StreamSuite2()->AEGP_DisposeStream(param_streamH));
    }

    LOG_ASSERT(err == PF_Err_NONE,"Failed to hide/show control"); //failed to hide/show control
}

void AEVideoHostDelegate::enableDisableControl(int id, bool enable)
{
    int hostParamIndex = hostIndexFromParamId(id);
    LOG_ASSERT(hostParamIndex > 0,"param not found"); //param not found
    LOG_ASSERT(m_param_utils_suite3 != nullptr,"Param utils suite NULL"); //param utils suite NULL

    if (hostParamIndex > 0 && 
            m_param_utils_suite3 != nullptr &&
            m_param_utils_suite3->PF_UpdateParamUI != nullptr)
    {
        if (enable) 
        {
            params[hostParamIndex]->ui_flags = 0; // Enable the bit
        }
        else 
        {
            params[hostParamIndex]->ui_flags = PF_PUI_DISABLED;  // Disable the bit
        }

        m_param_utils_suite3->PF_UpdateParamUI(in_data->effect_ref, 
                                                hostParamIndex,
                                                params[hostParamIndex]);
    }
}

DrawHelper& AEVideoHostDelegate::getDrawHelper()
{
    return m_drawHelper;
}

std::string AEVideoHostDelegate::getHostName()
{
    return "After_Effects";
}
