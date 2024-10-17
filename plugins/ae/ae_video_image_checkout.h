#pragma once

#include "ae_video_image_base.h"

class AEVideoImageCheckOut : public AEVideoImageBase
{   


public:
    AEVideoImageCheckOut(PF_ParamDef &paramDef, PF_InData *inData);
    virtual ~AEVideoImageCheckOut();

    virtual PF_LayerDef &layerDef() override;

protected:
    PF_ParamDef m_paramDef;
};
