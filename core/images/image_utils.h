#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include "ark_image.h"
#include "image_buffer.h"
#include <string>

bool copyImage(const ArkImagePtr src, ArkImagePtr dest, int downsampleX = 1, int downsampleY = 1);
bool copyAlphaToImage(const ArkImagePtr src, ArkImagePtr dst);
ArkImagePtr getImage(const std::string img_text);
std::string imageToPNG(ArkImagePtr img);
void fillImageBlack(const ArkImagePtr img);
void fillImage(const ArkImagePtr img, const Color &color);
ArkImage* resizeImageUp(ArkImage *inputImage, int newWidth, int newHeight);

inline uint16_t normalizePixelValueTo16(float pixelValue)
{
    return static_cast<uint16_t>(pixelValue * 65535.0f);
}

inline uint8_t normalizePixelValueTo8(float pixelValue)
{
    return static_cast<uint8_t>(pixelValue * 255.0f);
}

#endif
