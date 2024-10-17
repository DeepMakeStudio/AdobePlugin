#include "images/image_buffer.h"
#include "images/image_utils.h"
#include "image_buffer.h"


ImageBuffer::~ImageBuffer()
{
    if(m_all_your_datas_are_belong_to_us)
        free(m_data);
}

bool ImageBuffer::init(int width, int height, ImageFormat format, ChannelOrder channelOrder)
{
    m_width = width;
    m_height = height;
    m_channelOrder = channelOrder;
    m_format = format;

    switch (m_format)
    {
    case ImageFormat::RGB8:
        m_bytesPerPixel = 3;
        break;
    case ImageFormat::RGBA8:
        m_bytesPerPixel = 4;
        break;
    case ImageFormat::RGBA16:
        m_bytesPerPixel = 8;
        break;
    case ImageFormat::RGBA32:
        m_bytesPerPixel = 16;
        break;
    default:
        m_bytesPerPixel = 0;
        break;
    }

    m_strideBytes = m_width * m_bytesPerPixel;
    m_data = malloc(m_strideBytes * m_height);
    m_all_your_datas_are_belong_to_us = true;

    return true;
}

bool ImageBuffer::init(unsigned char* buffer, int width, int height, ImageFormat format, ChannelOrder channelOrder)
{
    m_width = width;
    m_height = height;
    m_channelOrder = channelOrder;
    m_format = format;

    switch (m_format)
    {
    case ImageFormat::RGB8:
        m_bytesPerPixel = 3;
        break;
    case ImageFormat::RGBA8:
        m_bytesPerPixel = 4;
        break;
    case ImageFormat::RGBA16:
        m_bytesPerPixel = 8;
        break;
    case ImageFormat::RGBA32:
        m_bytesPerPixel = 16;
        break;
    default:
        LOG_ASSERT(false,"wrong image format, currently only support 8, 16, 32 bit depth");
        m_bytesPerPixel = 0;
        break;
    }

    m_strideBytes = m_width * m_bytesPerPixel;
    m_data = buffer;
    m_all_your_datas_are_belong_to_us = false;

    return true;
}

int ImageBuffer::width()
{
    return m_width;
}
int ImageBuffer::height()
{
    return m_height;
}
int ImageBuffer::strideBytes()
{
    return m_strideBytes;
}
int ImageBuffer::bytesPerPixel()
{
    return m_bytesPerPixel;
}

int ImageBuffer::numChannels()
{
    int channels = 0;
    switch (format())
    {
        case ImageFormat::RGB8:
            channels = 3;
           break;
        case ImageFormat::RGBA8:
        case ImageFormat::RGBA16:
        case ImageFormat::RGBA32:
             channels = 4;
            break;
        default:
            LOG_ASSERT(false,"Unknown image format, currently only support 8, 16, 32 bit depth");
            break;
    }
    return channels;
}
ChannelOrder ImageBuffer::channelOrder()
{
    return m_channelOrder;
}

void ImageBuffer::setChannelOrder(ChannelOrder channelOrder)
{
    m_channelOrder = channelOrder;
}

ImageFormat ImageBuffer::format()
{
    return m_format;
}

void *ImageBuffer::data()
{
    return m_data;
}

void ImageBuffer::fill(const Color &color)
{
    //This needs to be templetized and optimized
    //Also not quite right since it ignores channel order
    if (m_format == ImageFormat::RGBA8)
    {
        uint8_t red8 = normalizePixelValueTo8(color.r);
        uint8_t grn8 = normalizePixelValueTo8(color.g);
        uint8_t blu8 = normalizePixelValueTo8(color.b);
        uint8_t alp8 = normalizePixelValueTo8(color.a);
        
        uint8_t *data = (uint8_t *)m_data;
        for (int y = 0; y < m_height; ++y)
        {
            for (int x = 0; x < m_width; ++x)
            {
                data[0] = red8;
                data[1] = grn8;
                data[2] = blu8;
                data[3] = alp8;
                data += 4;
            }
        }
    }
    else if (m_format == ImageFormat::RGBA16)
    {
        uint8_t red16 = normalizePixelValueTo16(color.r);
        uint8_t grn16 = normalizePixelValueTo16(color.g);
        uint8_t blu16 = normalizePixelValueTo16(color.b);
        uint8_t alp16 = normalizePixelValueTo16(color.a);

        uint16_t *data = (uint16_t *)m_data;
        for (int y = 0; y < m_height; ++y)
        {
            for (int x = 0; x < m_width; ++x)
            {
                data[0] = red16;
                data[1] = grn16;
                data[2] = blu16;
                data[3] = alp16;
                data += 4;
            }
        }
    }
    else if (m_format == ImageFormat::RGBA32)
    {
        uint32_t *data = (uint32_t *)m_data;
        for (int y = 0; y < m_height; ++y)
        {
            for (int x = 0; x < m_width; ++x)
            {
                data[0] = color.r;
                data[1] = color.g;
                data[2] = color.b;
                data[3] = color.a;
                data += 4;
            }
        }
    }
    else
    {
        LOG_ASSERT(false,"Unknown image format, currently only support RGBA 8, 16, 32 bit depth");
    }
}

void ImageBuffer::setImageString(const std::string &image)
{
//TODO
}
