#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "image_utils.h"
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;


bool imagesAreSameDimensions(const ArkImagePtr src, const ArkImagePtr dest)
{
    return (src->width() == dest->width() &&
            src->height() == dest->height() &&
            src->bitDepth() == dest->bitDepth() &&
            src->format() == dest->format() &&
            src->numChannels() == dest->numChannels() &&
            src->channelOrder() == dest->channelOrder() &&
            src->strideBytes() == dest->strideBytes());
}

bool copyAlphaToGreyScale(const ArkImagePtr src, const ArkImagePtr dest)
{
    //Currently just 8bit RGBA is supported copying to RGB with AAA channel order
    return (src->width() == dest->width() &&
            src->height() == dest->height() &&
            src->bitDepth() == dest->bitDepth() &&
            src->format() == ImageFormat::RGBA8 &&
            dest->format() == ImageFormat::RGB8 &&
            src->numChannels() == 4 &&
            dest->numChannels() == 3 &&
            dest->channelOrder() == ChannelOrder::AAA);
}

bool copyGreyScaleToAlpha(const ArkImagePtr src, const ArkImagePtr dest)
{
    //Currently just 8bit RGBA is supported copying to RGB with AAA channel order
    return (src->width() == dest->width() &&
            src->height() == dest->height() &&
            src->bitDepth() == dest->bitDepth() &&
            src->format() == ImageFormat::RGB8 &&
            dest->format() == ImageFormat::RGBA8 &&
            src->numChannels() == 3 &&
            dest->numChannels() == 4 &&
            src->channelOrder() == ChannelOrder::AAA);
}

bool imagesAreSameExceptForChannelOrder(const ArkImagePtr src, const ArkImagePtr dest)
{
    return (src->width() == dest->width() &&
            src->height() == dest->height() &&
            src->bitDepth() == dest->bitDepth() &&
            src->format() == dest->format() && 
            src->numChannels() == dest->numChannels() &&
            src->channelOrder() != dest->channelOrder());
}

void getPixels(const uint8_t *pixelPtr, int channels, ChannelOrder channelOrder, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a)
{
    if (channelOrder == ChannelOrder::BGRA)
    {
        b = *pixelPtr; pixelPtr++;
        g = *pixelPtr; pixelPtr++;
        r = *pixelPtr; pixelPtr++;
        if (channels == 4)
            a = *pixelPtr;
        else
            a = 0xff;
    }
    else if (channelOrder == ChannelOrder::RGBA)
    {
        r = *pixelPtr; pixelPtr++;
        g = *pixelPtr; pixelPtr++;
        b = *pixelPtr; pixelPtr++;
        if (channels == 4)
            a = *pixelPtr;
        else
            a = 0xff;
    }
    else if (channelOrder == ChannelOrder::ARGB)
    {
        LOG_ASSERT(channels == 4, "ARGB only supports 4 channels");
        a = *pixelPtr; pixelPtr++;
        r = *pixelPtr; pixelPtr++;
        g = *pixelPtr; pixelPtr++;
        b = *pixelPtr;
    }
    else if (channelOrder == ChannelOrder::AAA)
    {
        LOG_ASSERT(channels == 3,"all 3 channels have the same alpha value"); //all 3 channels have the same alpha value
        r = *pixelPtr; pixelPtr++;
        g = *pixelPtr; pixelPtr++;
        b = *pixelPtr;
        a = r;
    }
    else
    {
        assert(false); //todo
    }
}

void getAlpha(uint8_t *pixelPtr, ChannelOrder channelOrder, uint8_t &a)
{
    if (channelOrder == ChannelOrder::BGRA)
    {
        a = pixelPtr[3];
    }
    else if (channelOrder == ChannelOrder::RGBA)
    {
        a = pixelPtr[3];
    }
    else if (channelOrder == ChannelOrder::ARGB)
    {
        a = pixelPtr[0];
    }
    else if (channelOrder == ChannelOrder::AAA)
    {
        a = pixelPtr[0];
    }
    else
    {
        assert(false); //todo
    }
}

void setAlpha(uint8_t *pixelPtr, ImageFormat imgFormat, ChannelOrder channelOrder, uint8_t a)
{
    if ((channelOrder == ChannelOrder::RGBA || channelOrder == ChannelOrder::BGRA) &&
            imgFormat == ImageFormat::RGBA8)
    {
        pixelPtr[3] = a;
    }
    else if (channelOrder == ChannelOrder::ARGB)
    {
        pixelPtr[0] = a; 
    }
    else if (channelOrder == ChannelOrder::AAA)
    {
        pixelPtr[0] = a; 
        pixelPtr[1] = a; 
        pixelPtr[2] = a; 
    }
    else
    {
        assert(false); //todo
    }
}

void setPixels(uint8_t *pixelPtr, ImageFormat imgFormat, ChannelOrder channelOrder, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    if (channelOrder == ChannelOrder::BGRA)
    {
        *pixelPtr = b; pixelPtr++;
        *pixelPtr = g; pixelPtr++;
        *pixelPtr = r; pixelPtr++;
        if (imgFormat == ImageFormat::RGBA8)
            *pixelPtr = a;
    }
    else if (channelOrder == ChannelOrder::RGBA)
    {
        *pixelPtr = r; pixelPtr++;
        *pixelPtr = g; pixelPtr++;
        *pixelPtr = b; pixelPtr++;
        if (imgFormat == ImageFormat::RGBA8)
            *pixelPtr = a;
    }
    else if (channelOrder == ChannelOrder::ARGB)
    {
        *pixelPtr = a; pixelPtr++;
        *pixelPtr = r; pixelPtr++;
        *pixelPtr = g; pixelPtr++;
        *pixelPtr = b; pixelPtr++;
    }
    else if (channelOrder == ChannelOrder::AAA)
    {
        *pixelPtr = r; pixelPtr++;
        *pixelPtr = g; pixelPtr++;
        *pixelPtr = b; pixelPtr++;
    }
    else
    {
        assert(false); //todo
    }
}

bool copyImage(const ArkImagePtr src, ArkImagePtr dst, int downsampleX, int downsampleY)
{ 
    int src_width = src->width();
    int src_height = src->height();
    int src_stride = src->strideBytes();
    int dst_width = dst->width();
    int dst_height = dst->height();
    int dst_stride = dst->strideBytes();
    int src_pixel_bytes = src->bytesPerPixel();
    int dst_pixel_bytes = dst->bytesPerPixel();
    int src_channels = src->numChannels();
    ChannelOrder src_channel_order = src->channelOrder();
    ChannelOrder dst_channel_order = dst->channelOrder();
    ImageFormat src_image_format = src->format();
    ImageFormat dst_image_format = dst->format();
    uint8_t* srcAddress = (uint8_t*)src->data();
    uint8_t* dstAddress = (uint8_t*)dst->data();

    
    if (imagesAreSameDimensions(src, dst))
    {
        memcpy(dstAddress, srcAddress, src_stride * src_height);
        return true;
    }
    else if (imagesAreSameExceptForChannelOrder(src, dst))
    {
        uint8_t r, g, b, a;

        for (int y = 0; y < src_height; y++)
        {
            for (int x = 0; x < src_width; x++)
            {
                uint8_t *src = srcAddress + (src_stride * y) + (x * src_pixel_bytes);
                uint8_t *dst = dstAddress + (dst_stride * y) + (x * dst_pixel_bytes);
                getPixels(src, src_channels, src_channel_order, r, g, b, a);
                setPixels(dst, dst_image_format, dst_channel_order, r, g, b, a);
            }
        }
        return true;
    }
    if (copyAlphaToGreyScale(src, dst))
    {
        uint8_t a;

        for (int y = 0; y < src_height; y++)
        {
            for (int x = 0; x < src_width; x++)
            {
                uint8_t *src = srcAddress + (src_stride * y) + (x * src_pixel_bytes);
                uint8_t *dst = dstAddress + (dst_stride * y) + (x * dst_pixel_bytes);
                getAlpha(src, src_channel_order, a);
                setAlpha(dst, dst_image_format, dst_channel_order, a);
            }
        }
        return true;
    }
    if (copyGreyScaleToAlpha(src, dst))
    {
        uint8_t a;

        for (int y = 0; y < src_height; y++)
        {
            for (int x = 0; x < src_width; x++)
            {
                uint8_t *src = srcAddress + (src_stride * y) + (x * src_pixel_bytes);
                uint8_t *dst = dstAddress + (dst_stride * y) + (x * dst_pixel_bytes);
                getAlpha(src, src_channel_order, a);
                setAlpha(dst, dst_image_format, dst_channel_order, a);
            }
        }
        return true;
    }
    else if (dst_width <= src_width && dst_height <= src_height)
    {
        uint8_t r, g, b, a;

       for (int y = 0; y < dst_height; y++)
        {
            for (int x = 0; x < dst_width; x++)
            {
                int src_x_start = x * downsampleX;
                int src_y_start = y * downsampleY;
                int src_x_end = (x + 1) * downsampleX;
                int src_y_end = (y + 1) * downsampleY;

                int totalR = 0, totalG = 0, totalB = 0, totalA = 0;
                int totalPixels = 0;

                for (int src_y = src_y_start; src_y < src_y_end && src_y < src_height; src_y++)
                {
                    for (int src_x = src_x_start; src_x < src_x_end && src_x < src_width; src_x++)
                    {
                        const uint8_t* srcPixel = srcAddress + (src_stride * src_y) + (src_x * src_pixel_bytes);
                        getPixels(srcPixel, src_channels, src_channel_order, r, g, b, a);

                        totalR += r;
                        totalG += g;
                        totalB += b;
                        totalA += a;
                        totalPixels++;
                    }
                }

                if (totalPixels > 0)
                {
                    uint8_t* dstPixel = dstAddress + (dst_stride * y) + (x * dst_pixel_bytes);
                    setPixels(dstPixel, dst_image_format, dst_channel_order,
                            totalR / totalPixels, totalG / totalPixels,
                            totalB / totalPixels, totalA / totalPixels);

                }
                else
                {
                    uint8_t* dstPixel = dstAddress + (dst_stride * y) + (x * dst_pixel_bytes);
                    setPixels(dstPixel, dst_image_format, dst_channel_order, 0, 0, 0, 255);
                    
                }
            }
        }
        return true;
    }

    if (src_image_format != dst_image_format || src_stride != dst_stride)
    {
        uint8_t r, g, b, a;
        //TODO Optimize this code path and make it more flexible, this is currently
        //a quick and dirty implementation for the POC (better yet use an image processing library)
         for (int y = 0; y < dst_height; y++)
         {
             for (int x = 0; x < dst_width; x++)
             {
                 uint8_t* src = srcAddress + (src_stride * y) + (x * src_pixel_bytes);
                 uint8_t* dst = dstAddress + (dst_stride * y) + (x * dst_pixel_bytes);

                 if (y < src_height && x < src_width)
                 {
                     getPixels(src, src_channels, src_channel_order, r, g, b, a);
                     setPixels(dst, dst_image_format, dst_channel_order, r, g, b, a);
                 }
                 else
                 {
                     setPixels(dst, dst_image_format, dst_channel_order, 0, 0, 0, 255);
                 }
             }
         }
         LogWarning("copyImage: src and dst have different formats or strides, this is not optimal");
        return true;
    }

    memcpy(dstAddress, srcAddress, src_stride * src_height);

    return true;
}

bool copyAlphaToImage(const ArkImagePtr src, ArkImagePtr dst)
{
    int src_width = src->width();
    int src_height = src->height();
    int src_stride = src->strideBytes();
    int dst_stride = dst->strideBytes();
    int src_pixel_bytes = src->bytesPerPixel();
    int dst_pixel_bytes = dst->bytesPerPixel();
    ChannelOrder src_channel_order = src->channelOrder();
    ChannelOrder dst_channel_order = dst->channelOrder();
    ImageFormat dst_image_format = dst->format();
    uint8_t *srcAddress = (uint8_t*)src->data();
    uint8_t *dstAddress = (uint8_t*)dst->data();

    if (copyGreyScaleToAlpha(src, dst))
    {
        uint8_t a;

         for (int y = 0; y < src_height; y++)
         {
             for (int x = 0; x < src_width; x++)
             {
                uint8_t *src = srcAddress + (src_stride * y) + (x * src_pixel_bytes);
                uint8_t *dst = dstAddress + (dst_stride * y) + (x * dst_pixel_bytes);
                getAlpha(src, src_channel_order, a);
                setAlpha(dst, dst_image_format, dst_channel_order, a);
             }
         }
        return true;
    }
    assert(false); //unimplemented
    return false;
}

void fillImageBlack(const ArkImagePtr img)
{
    if (img && img->data())
    {
        memset(img->data(), 0, img->strideBytes() * img->height());
    }
}


void fillImage(const ArkImagePtr img, const Color &color)
{
    if (img && img->data())
    {
        const int width = img->width();
        const int height = img->height();
        const int stride = img->strideBytes();
        const int pixelBytes = img->bytesPerPixel();
        const ChannelOrder channeOrder = img->channelOrder();
        const ImageFormat imgFormat = img->format();
        uint8_t r = (color.r * 255) + .5;
        uint8_t g = (color.g * 255) + .5;
        uint8_t b = (color.b * 255) + .5;
        uint8_t a = (color.a * 255) + .5;

        uint8_t* dstAddress = (uint8_t*)img->data();

        // Iterate through each pixel and set the color.
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                uint8_t* dst = dstAddress + (stride * y) + (x * pixelBytes);
                setPixels(dst, imgFormat, channeOrder, r, g, b, a);
            }
        }
    }
}

ArkImagePtr getImage(const std::string img_text)
{
    std::shared_ptr<ImageBuffer> image_buffer;
    int width, height, channels;
    unsigned char* image = stbi_load_from_memory((unsigned char*)img_text.c_str(), img_text.size(), &width, &height, &channels, STBI_rgb);

    if (image != nullptr) 
    {
        image_buffer = std::make_shared<ImageBuffer>();
        if(!image_buffer->init(image, width, height, ImageFormat::RGB8, ChannelOrder::RGBA))
            stbi_image_free(image);  // Remember to free the allocated memory
    }
    return image_buffer;
}

std::string imageToPNG(ArkImagePtr img)
{
    ArkImagePtr imgToWrite;

    //Conver the image to RGBA if it's not already in that format
    if (img->format() != ImageFormat::RGBA8 || 
        img->format() != ImageFormat::RGB8)
    {
        shared_ptr<ImageBuffer> imgBuf = std::make_shared<ImageBuffer>();
        imgBuf->init(img->width(), img->height(), ImageFormat::RGBA8, ChannelOrder::RGBA);
        copyImage(img, imgBuf);
        imgToWrite = dynamic_pointer_cast<ArkImage>(imgBuf);
    }
    else
    {
        imgToWrite = img;
    }

    int length = 0;
    unsigned char *img_string = stbi_write_png_to_mem(static_cast<const unsigned char*>(imgToWrite->data()),
                                                      imgToWrite->strideBytes(),
                                                      imgToWrite->width(),
                                                      imgToWrite->height(),
                                                      imgToWrite->numChannels(),
                                                      &length);
    std::string ret_string((const char*)img_string, length);
    return std::move(ret_string);
}

// Function to resize an ArkImage using bilinear interpolation
// Currently only handles scaling up
ArkImage* resizeImageUp(ArkImage *inputImage, int newWidth, int newHeight)
{
    if (inputImage == nullptr)
    {
        assert(inputImage);
        return nullptr;
    }
    if (inputImage->width() > newWidth &&
        inputImage->height() > newHeight)
    {
        return nullptr;
    }
    
    int outputWidth = newWidth;
    int outputHeight = newHeight;
    int outputStrideBytes = outputWidth * inputImage->bytesPerPixel();
    ImageBuffer *outputImage = new ImageBuffer();
    outputImage->init(outputWidth, outputHeight, inputImage->format(), inputImage->channelOrder());

    // Get pointers to input and output image data
    uint8_t *inputData = static_cast<uint8_t *>(inputImage->data());
    uint8_t *outputData = static_cast<uint8_t *>(outputImage->data());

    // Calculate scaling factors
    double xScale = static_cast<double>(inputImage->width()) / newWidth;
    double yScale = static_cast<double>(inputImage->height()) / newHeight;

    // Resize using bilinear interpolation
    for (int y = 0; y < outputHeight; ++y)
    {
        for (int x = 0; x < outputWidth; ++x)
        {
            // Calculate the corresponding position in the input image
            double xInput = x * xScale;
            double yInput = y * yScale;

            // Calculate the integer and fractional parts
            int xInt = static_cast<int>(std::floor(xInput));
            int yInt = static_cast<int>(std::floor(yInput));
            double xFrac = xInput - xInt;
            double yFrac = yInput - yInt;

            // Perform bilinear interpolation
            for (int channel = 0; channel < inputImage->numChannels(); ++channel)
            {
                int topLeftIndex = (yInt * inputImage->strideBytes()) + (xInt * inputImage->bytesPerPixel()) + channel;
                int topRightIndex = topLeftIndex + inputImage->bytesPerPixel();
                int bottomLeftIndex = topLeftIndex + inputImage->strideBytes();
                int bottomRightIndex = bottomLeftIndex + inputImage->bytesPerPixel();

                // Bilinear interpolation formula
                double topInterpolation = inputData[topLeftIndex] * (1.0 - xFrac) + inputData[topRightIndex] * xFrac;
                double bottomInterpolation = inputData[bottomLeftIndex] * (1.0 - xFrac) + inputData[bottomRightIndex] * xFrac;

                double finalInterpolation = topInterpolation * (1.0 - yFrac) + bottomInterpolation * yFrac;

                // Assign the result to the output image
                int outputIndex = (y * outputStrideBytes) + (x * inputImage->bytesPerPixel()) + channel;
                outputData[outputIndex] = static_cast<uint8_t>(finalInterpolation);
            }
        }
    }
    return outputImage;
}

