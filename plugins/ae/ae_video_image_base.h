#pragma once

#include "images/ark_image.h"
#include "AE_Effect.h"

class AEVideoImageBase : public ArkImage
{   
public:
    AEVideoImageBase(PF_InData *inData);
    virtual ~AEVideoImageBase() = default;

    virtual PF_LayerDef &layerDef() = 0;

    int width() override;
    int height() override;
    int strideBytes() override;
    int bytesPerPixel() override;
    int numChannels() override;
    ChannelOrder channelOrder() override;
    void setChannelOrder(ChannelOrder channelOrder) override;

    ImageFormat format() override;

    void *data() override;

protected:
    PF_InData *in_data = nullptr;
};
