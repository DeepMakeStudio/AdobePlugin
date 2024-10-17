#include "ae_video_image.h"
#include "AE_EffectCB.h"
#include "AE_Effect.h"
#include <AE_Macros.h>
#include <logger.h>
#include <assert.h>

AEVideoImage::AEVideoImage(PF_LayerDef &layerDef, PF_InData *inData)    
    : AEVideoImageBase(inData)
    , m_layerDef(layerDef)
{
}

PF_LayerDef &AEVideoImage::layerDef() 
{ 
    return m_layerDef; 
}

