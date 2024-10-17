#include "ofx_video_image.h"

OFXVideoImage::OFXVideoImage(OfxPropertySetHandle image, OfxPropertySuiteV1* propHost, OfxPropertySetHandle clipHandle, OfxImageEffectSuiteV1* effectSuite)
    : m_image(image)
    , m_propHost(propHost)
    , m_clipHandle(clipHandle)
    , m_effectSuite(effectSuite)
{
}       

int OFXVideoImage::width()
{
    int width = 0;
    if(m_propHost != nullptr)
    {
        OfxRectI imgRect;
        m_propHost->propGetIntN(m_image, kOfxImagePropBounds, 4, &imgRect.x1);
        width = imgRect.x2 - imgRect.x1;
    }
    return width;
}

int OFXVideoImage::height()
{
    int height = 0;
    if(m_propHost != nullptr)
    {
        OfxRectI imgRect;
        m_propHost->propGetIntN(m_image, kOfxImagePropBounds, 4, &imgRect.x1);
        height = imgRect.y2 - imgRect.y1;
    }
    return height;
}

int OFXVideoImage::strideBytes()
{
    int strideBytes = 0;
    if(m_propHost != nullptr)
        m_propHost->propGetInt(m_image, kOfxImagePropRowBytes, 0, &strideBytes);
    return strideBytes;
}

int OFXVideoImage::bytesPerPixel()
{
    long bitDepth = 0;
	char *pixDepthString = NULL;
	m_propHost->propGetString(m_image, kOfxImageEffectPropPixelDepth, 0, &pixDepthString);

    if (pixDepthString)
    {
        if (strcmp(pixDepthString, kOfxBitDepthByte) == 0)
            bitDepth = 4;
        else if (strcmp(pixDepthString, kOfxBitDepthShort) == 0)
            bitDepth = 8;
        else if (strcmp(pixDepthString, kOfxBitDepthFloat) == 0)
            bitDepth = 16;
    }
    return bitDepth;  
}

int OFXVideoImage::numChannels()
{
    return 4;
}

ChannelOrder OFXVideoImage::channelOrder()
{
    return ChannelOrder::RGBA;
}

ImageFormat OFXVideoImage::format()
{
    return ImageFormat::RGBA8;
}

void *OFXVideoImage::data()
{
    void *dataPtr = nullptr;
    if (m_propHost)
        m_propHost->propGetPointer(m_image, kOfxImagePropData, 0, &dataPtr);
    return dataPtr;
}


