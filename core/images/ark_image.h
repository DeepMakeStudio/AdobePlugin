#ifndef ARK_IMAGE_H
#define ARK_IMAGE_H
#include <../logging/logger.h>
#include <memory>
enum class ImageFormat {
    RGB8,
    RGBA8,
    RGBA16,
    RGBA32,
    UnknownImageFormat
};

enum ChannelOrder {
    RGBA,
    BGRA,
    ARGB,
    ABGR,
    AAA, //grey scale/alpha
    UnknownChannelOrder
};

class ArkImage
{   
public:

    virtual ~ArkImage() {};
    virtual int width() = 0;
    virtual int height() = 0;
    virtual int strideBytes() = 0;
    virtual int bytesPerPixel() = 0;

    int bitDepth()
    {
        int bitDepth = 0;
        switch (format())
        {
            case ImageFormat::RGB8:
            case ImageFormat::RGBA8:
                bitDepth = 8;
                break;
            case ImageFormat::RGBA16:
                bitDepth = 16;
                break;
            case ImageFormat::RGBA32:
                bitDepth = 32;
                break;
            default:
                LOG_ASSERT(false,"Unknown image format, currently only support 8, 16, 32 bit depth");
                break;
        }
        return bitDepth;
    }

    virtual int numChannels() = 0;
    virtual ChannelOrder channelOrder() = 0;
    virtual void setChannelOrder(ChannelOrder channelOrder) = 0;

    virtual ImageFormat format() = 0;

    virtual void *data() = 0;

};

using ArkImagePtr = std::shared_ptr<ArkImage>;

#endif // ARK_IMAGE_H
