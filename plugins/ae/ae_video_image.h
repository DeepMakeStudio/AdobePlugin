#pragma once
#include "ae_video_image_base.h"


class AEVideoImage : public AEVideoImageBase
{   


public:
    AEVideoImage(PF_LayerDef &layerDef, PF_InData *inData);

    virtual PF_LayerDef &layerDef() override;

protected:
    PF_LayerDef m_layerDef;
};
