#include "ae_video_image_checkout.h"
#include <AE_Macros.h>
#include <logger.h>

AEVideoImageCheckOut::AEVideoImageCheckOut(PF_ParamDef &paramDef, PF_InData *inData)
    : AEVideoImageBase(inData)
    , m_paramDef(paramDef)
{
}

AEVideoImageCheckOut::~AEVideoImageCheckOut()
{
    PF_Err err = PF_CHECKIN_PARAM(in_data, &m_paramDef); 
    LogInfo("attempting to check in layer");
    if (err == PF_Err_NONE)
    {
        LogInfo("Checked in layer successfully");
    }
    else
    {
        LogError("Error checking in layer:" + std::to_string(err));
    }
}

PF_LayerDef &AEVideoImageCheckOut::layerDef() 
{ 
    return m_paramDef.u.ld; 
};

