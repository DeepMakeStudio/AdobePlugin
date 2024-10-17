#include "ae_video_image_base.h"
#include "AE_EffectCB.h"
#include "AE_Effect.h"
#include <AE_Macros.h>
#include <logger.h>
#include <assert.h>


AEVideoImageBase::AEVideoImageBase(PF_InData *inData)    
    : in_data(inData)
{
}

int AEVideoImageBase::width()
{
    return layerDef().width;
}

int AEVideoImageBase::height()
{
    return layerDef().height;
}

int AEVideoImageBase::strideBytes()
{
    return layerDef().rowbytes;
}

int AEVideoImageBase::bytesPerPixel()
{
    if (layerDef().world_flags & PF_WorldFlag_DEEP)
        return 8;
    return 4;
}

int AEVideoImageBase::numChannels()
{
    return 4;
}

ChannelOrder AEVideoImageBase::channelOrder()
{
    return ChannelOrder::ARGB;
}

void AEVideoImageBase::setChannelOrder(ChannelOrder channelOrder)
{
    assert(false);
}

ImageFormat AEVideoImageBase::format()
{
    if (layerDef().world_flags & PF_WorldFlag_DEEP)
        return ImageFormat::RGBA16;

    return ImageFormat::RGBA8;
}

void *AEVideoImageBase::data()
{
    PF_Err     err = PF_Err_NONE;
    if (layerDef().world_flags & PF_WorldFlag_DEEP)
    {
        PF_Pixel16 *deep_pixelP = NULL;
        PF_Err     err = PF_Err_NONE;
        err = PF_GET_PIXEL_DATA16(&layerDef(), NULL, &deep_pixelP);
        return deep_pixelP;
    }
    
    PF_Pixel8 *pixelP = NULL;
    err = PF_GET_PIXEL_DATA8(&layerDef(), NULL, &pixelP);

    return pixelP;
}

