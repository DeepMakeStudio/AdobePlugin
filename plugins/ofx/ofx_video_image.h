#include "images/ark_image.h"
#include "ofxImageEffect.h"

class OFXVideoImage : public ArkImage
{   
public:
	OFXVideoImage(OfxPropertySetHandle image, OfxPropertySuiteV1* propHost, OfxPropertySetHandle clipHandle, OfxImageEffectSuiteV1* effectSuite);
    virtual ~OFXVideoImage() = default;

    int width() override;
    int height() override;
    int strideBytes() override;
    int bytesPerPixel() override;

    int numChannels() override;
    ChannelOrder channelOrder() override;
    ImageFormat format() override;

    void *data() override;

protected:
    OfxPropertySetHandle m_image;
    OfxPropertySuiteV1* m_propHost;
    OfxPropertySetHandle m_clipHandle;
    OfxImageEffectSuiteV1* m_effectSuite;
};
