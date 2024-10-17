#include <gtest/gtest.h>
#include "images/image_utils.h"
#include "images/image_buffer.h"

using namespace ::testing;

class ImageUtilsTest: public Test
{
};


TEST(ImageUtilsTest, TestFillRGB8) {
    
    std::shared_ptr<ImageBuffer> src = std::make_shared<ImageBuffer>();
    src->init(2, 2, ImageFormat::RGB8, ChannelOrder::RGBA);
    fillImageBlack(src);
    
    std::shared_ptr<ImageBuffer> dest = std::make_shared<ImageBuffer>();
    dest->init(2, 2, ImageFormat::RGBA8, ChannelOrder::ARGB);

    copyImage(src, dest);
}

TEST(ImageUtilsTest, TestBGRACopy) {

    std::shared_ptr<ImageBuffer> rgbaSrc = std::make_shared<ImageBuffer>();
    rgbaSrc->init(2, 2, ImageFormat::RGBA8, ChannelOrder::RGBA);
    fillImage(rgbaSrc, Color(1.0, 0, 0));

    std::shared_ptr<ImageBuffer> bgraDst = std::make_shared<ImageBuffer>();
    bgraDst->init(2, 2, ImageFormat::RGBA8, ChannelOrder::BGRA);

    std::shared_ptr<ImageBuffer> rgbaDst = std::make_shared<ImageBuffer>();
    rgbaDst->init(2, 2, ImageFormat::RGBA8, ChannelOrder::RGBA);

    copyImage(rgbaSrc, bgraDst);
    copyImage(bgraDst, rgbaDst);
    size_t imageSizeinBytes = rgbaSrc->height() * rgbaSrc->strideBytes();

    int result = memcmp(rgbaSrc->data(), rgbaDst->data(), imageSizeinBytes);
    EXPECT_EQ(result, 0);
}

TEST(ImageUtilsTest, TestARGBCopy) {

    std::shared_ptr<ImageBuffer> rgbaSrc = std::make_shared<ImageBuffer>();
    rgbaSrc->init(2, 2, ImageFormat::RGBA8, ChannelOrder::RGBA);
    fillImage(rgbaSrc, Color(1.0, 0, 0));

    std::shared_ptr<ImageBuffer> bgraDst = std::make_shared<ImageBuffer>();
    bgraDst->init(2, 2, ImageFormat::RGBA8, ChannelOrder::ARGB);

    std::shared_ptr<ImageBuffer> rgbaDst = std::make_shared<ImageBuffer>();
    rgbaDst->init(2, 2, ImageFormat::RGBA8, ChannelOrder::RGBA);

    copyImage(rgbaSrc, bgraDst);
    copyImage(bgraDst, rgbaDst);
    size_t imageSizeinBytes = rgbaSrc->height() * rgbaSrc->strideBytes();

    int result = memcmp(rgbaSrc->data(), rgbaDst->data(), imageSizeinBytes);
    EXPECT_EQ(result, 0);
}

TEST(ImageUtilsTest, TestRGBACopy) {

    std::shared_ptr<ImageBuffer> rgbaSrc = std::make_shared<ImageBuffer>();
    rgbaSrc->init(2, 2, ImageFormat::RGBA8, ChannelOrder::RGBA);
    fillImage(rgbaSrc, Color(1.0, 0, 0));

    std::shared_ptr<ImageBuffer> rgbaDst = std::make_shared<ImageBuffer>();
    rgbaDst->init(2, 2, ImageFormat::RGBA8, ChannelOrder::RGBA);

    copyImage(rgbaSrc, rgbaDst);
    size_t imageSizeinBytes = rgbaSrc->height() * rgbaSrc->strideBytes();

    int result = memcmp(rgbaSrc->data(), rgbaDst->data(), imageSizeinBytes);
    EXPECT_EQ(result, 0);
}

TEST(ImageUtilsTest, TestRgbToRgbaCopy) {

    std::shared_ptr<ImageBuffer> rgbSrc = std::make_shared<ImageBuffer>();
    rgbSrc->init(2, 2, ImageFormat::RGB8, ChannelOrder::RGBA);
    fillImage(rgbSrc, Color(1.0, 0, 0));

    std::shared_ptr<ImageBuffer> rgbaDst = std::make_shared<ImageBuffer>();
    rgbaDst->init(2, 2, ImageFormat::RGBA8, ChannelOrder::RGBA);

    copyImage(rgbSrc, rgbaDst);

    uint8_t *img_ptr = static_cast<uint8_t*>(rgbaDst->data());

    EXPECT_EQ(img_ptr[0], 0xff);
    EXPECT_EQ(img_ptr[1], 0);
    EXPECT_EQ(img_ptr[2], 0);
    EXPECT_EQ(img_ptr[3], 0xff);

    EXPECT_EQ(img_ptr[4], 0xff);
    EXPECT_EQ(img_ptr[5], 0);
    EXPECT_EQ(img_ptr[6], 0);
    EXPECT_EQ(img_ptr[7], 0xff);
    
    EXPECT_EQ(img_ptr[8], 0xff);
    EXPECT_EQ(img_ptr[9], 0);
    EXPECT_EQ(img_ptr[10], 0);
    EXPECT_EQ(img_ptr[11], 0xff);

    EXPECT_EQ(img_ptr[12], 0xff);
    EXPECT_EQ(img_ptr[13], 0);
    EXPECT_EQ(img_ptr[14], 0);
    EXPECT_EQ(img_ptr[15], 0xff);
}

TEST(ImageUtilsTest, TestRgbaToRgbCopy) {

    std::shared_ptr<ImageBuffer> rgbaSrc = std::make_shared<ImageBuffer>();
    rgbaSrc->init(2, 2, ImageFormat::RGBA8, ChannelOrder::RGBA);
    fillImage(rgbaSrc, Color(1.0, 0, 0));

    std::shared_ptr<ImageBuffer> rgbDst = std::make_shared<ImageBuffer>();
    rgbDst->init(2, 2, ImageFormat::RGB8, ChannelOrder::RGBA);

    copyImage(rgbaSrc, rgbDst);

    uint8_t* img_ptr = static_cast<uint8_t*>(rgbDst->data());

    EXPECT_EQ(img_ptr[0], 0xff);
    EXPECT_EQ(img_ptr[1], 0);
    EXPECT_EQ(img_ptr[2], 0);

    EXPECT_EQ(img_ptr[3], 0xff);
    EXPECT_EQ(img_ptr[4], 0);
    EXPECT_EQ(img_ptr[5], 0);
    
    EXPECT_EQ(img_ptr[6], 0xff);
    EXPECT_EQ(img_ptr[7], 0);
    EXPECT_EQ(img_ptr[8], 0);

    EXPECT_EQ(img_ptr[9], 0xff);
    EXPECT_EQ(img_ptr[10], 0);
    EXPECT_EQ(img_ptr[11], 0);
}

TEST(ImageUtilsTest, TestRGBAToGreyScaleRGB) {

    uint8_t pixel0alpha = 0x0;
    uint8_t pixel1alpha = 0x80;
    uint8_t pixel2alpha = 0xaa;
    uint8_t pixel3alpha = 0xff;
    std::shared_ptr<ImageBuffer> rgbaSrc = std::make_shared<ImageBuffer>();
    rgbaSrc->init(2, 2, ImageFormat::RGBA8, ChannelOrder::RGBA);
    fillImage(rgbaSrc, Color(1.0, 0, 0));
    
    //Set the alpha channels to something more interesting that just all 0xff
    uint8_t *src_ptr = static_cast<uint8_t*>(rgbaSrc->data());
    src_ptr[3] = pixel0alpha;
    src_ptr[7] = pixel1alpha;
    src_ptr[11] = pixel2alpha;
    src_ptr[15] = pixel3alpha;

    std::shared_ptr<ImageBuffer> aaaDst = std::make_shared<ImageBuffer>();
    aaaDst->init(2, 2, ImageFormat::RGB8, ChannelOrder::AAA);

    copyImage(rgbaSrc, aaaDst);

    uint8_t* img_ptr = static_cast<uint8_t*>(aaaDst->data());

    EXPECT_EQ(img_ptr[0], pixel0alpha);
    EXPECT_EQ(img_ptr[1], pixel0alpha);
    EXPECT_EQ(img_ptr[2], pixel0alpha);

    EXPECT_EQ(img_ptr[3], pixel1alpha);
    EXPECT_EQ(img_ptr[4], pixel1alpha);
    EXPECT_EQ(img_ptr[5], pixel1alpha);
    
    EXPECT_EQ(img_ptr[6], pixel2alpha);
    EXPECT_EQ(img_ptr[7], pixel2alpha);
    EXPECT_EQ(img_ptr[8], pixel2alpha);

    EXPECT_EQ(img_ptr[9], pixel3alpha);
    EXPECT_EQ(img_ptr[10], pixel3alpha);
    EXPECT_EQ(img_ptr[11], pixel3alpha);
}

TEST(ImageUtilsTest, TestGreyScaleToRGBA) {

    std::shared_ptr<ImageBuffer> aaaSrc = std::make_shared<ImageBuffer>();
    aaaSrc->init(2, 2, ImageFormat::RGB8, ChannelOrder::AAA);
    fillImage(aaaSrc, Color(0.5, 0.5, 0.5));

    std::shared_ptr<ImageBuffer> rgbaDst = std::make_shared<ImageBuffer>();
    rgbaDst->init(2, 2, ImageFormat::RGBA8, ChannelOrder::RGBA);
    fillImage(rgbaDst, Color(1.0, 0, 0));

    copyImage(aaaSrc, rgbaDst);
    
    uint8_t* img_ptr = static_cast<uint8_t*>(rgbaDst->data());
    EXPECT_EQ(img_ptr[3], 128);
    EXPECT_EQ(img_ptr[7], 128);
    EXPECT_EQ(img_ptr[11], 128);
    EXPECT_EQ(img_ptr[15], 128);
}

TEST(ImageUtilsTest, TestGreyScaleToBGRA) {

    std::shared_ptr<ImageBuffer> aaaSrc = std::make_shared<ImageBuffer>();
    aaaSrc->init(2, 2, ImageFormat::RGB8, ChannelOrder::AAA);
    fillImage(aaaSrc, Color(0.5, 0.5, 0.5));

    std::shared_ptr<ImageBuffer> rgbaDst = std::make_shared<ImageBuffer>();
    rgbaDst->init(2, 2, ImageFormat::RGBA8, ChannelOrder::BGRA);
    fillImage(rgbaDst, Color(1.0, 0, 0));

    copyImage(aaaSrc, rgbaDst);
    
    uint8_t* img_ptr = static_cast<uint8_t*>(rgbaDst->data());
    EXPECT_EQ(img_ptr[3], 128);
    EXPECT_EQ(img_ptr[7], 128);
    EXPECT_EQ(img_ptr[11], 128);
    EXPECT_EQ(img_ptr[15], 128);
}

TEST(ImageUtilsTest, TestGreyScaleToARGB) {

    std::shared_ptr<ImageBuffer> aaaSrc = std::make_shared<ImageBuffer>();
    aaaSrc->init(2, 2, ImageFormat::RGB8, ChannelOrder::AAA);
    fillImage(aaaSrc, Color(0.5, 0.5, 0.5));

    std::shared_ptr<ImageBuffer> rgbaDst = std::make_shared<ImageBuffer>();
    rgbaDst->init(2, 2, ImageFormat::RGBA8, ChannelOrder::ARGB);
    fillImage(rgbaDst, Color(1.0, 0, 0));

    copyImage(aaaSrc, rgbaDst);
    
    uint8_t* img_ptr = static_cast<uint8_t*>(rgbaDst->data());
    EXPECT_EQ(img_ptr[0], 128);
    EXPECT_EQ(img_ptr[4], 128);
    EXPECT_EQ(img_ptr[8], 128);
    EXPECT_EQ(img_ptr[12], 128);
}
