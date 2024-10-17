#ifndef IMAGE_BUFFER_H
#define IMAGE_BUFFER_H
#include <memory>
#include <string>
#include "images/ark_image.h"
#include "ark_types.h"

class ImageBuffer : public ArkImage
{   
public:
    ImageBuffer() = default;
    virtual ~ImageBuffer();
    bool init(int width, int height, ImageFormat format, ChannelOrder channelOrder);
    bool init(unsigned char* buffer, int width, int height, ImageFormat format, ChannelOrder channelOrder);

    virtual int width();
    virtual int height();
    virtual int strideBytes();
    virtual int bytesPerPixel();

    virtual int numChannels();
    virtual void setChannelOrder(ChannelOrder channelOrder);
    virtual ChannelOrder channelOrder();
    virtual ImageFormat format();

    virtual void *data();
    virtual void fill(const Color &color);
    virtual void setImageString(const std::string &image);

protected:
    int m_width{0};
    int m_height{0};
    int m_strideBytes{0};
    int m_bytesPerPixel{0};
    ChannelOrder m_channelOrder {ChannelOrder::UnknownChannelOrder};
    ImageFormat m_format {ImageFormat::UnknownImageFormat};
    bool m_all_your_datas_are_belong_to_us {false};
    void *m_data{0};
};


#endif // IMAGE_BUFFER_H
