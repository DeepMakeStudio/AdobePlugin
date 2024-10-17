

#include <gtest/gtest.h>
#include "main_api_connection/plugin_json_parser.h"
#include "parameter.h"

using namespace ::testing;

static std::string sConfigJson {"{\r\n    \"model_name\": \"runwayml/stable-diffusion-v1-5\",\r\n    \"save_output\": true,\r\n    \"model_dtype\": \"fp32\",\r\n    \"loras\": []\r\n}\r\n"};

static std::string sGetInfoJson {"{\r\n    \"plugin\": {\r\n        \"Name\": \"Stable Diffusion (diffusers)\",\r\n        \"Version\": \"0.1.0\",\r\n        \"Author\": \"DeepMake\",\r\n        \"Description\": \"Stable Diffusion generation using the Diffusers library\",\r\n        \"env\": \"sd\"\r\n    },\r\n    \"config\": {\r\n        \"model_name\": \"runwayml\/stable-diffusion-v1-5\",\r\n        \"save_output\": true,\r\n        \"model_dtype\": \"fp32\",\r\n        \"loras\": []\r\n    },\r\n    \"endpoints\": {\r\n        \"generate_image\": {\r\n            \"call\": \"execute\",\r\n            \"inputs\": {\r\n\t\t\t\t\"prompt\": \"Text\",\r\n\t\t\t\t\"seed\": \"Int(default=None, optional=true)\",\r\n\t\t\t\t\"iterations\": \"Int(default=20, min=1, optional=true)\",\r\n\t\t\t\t\"height\": \"Int(default=512, min=16, optional=true)\",\r\n\t\t\t\t\"width\": \"Int(default=512, min=16, optional=true)\",\r\n\t\t\t\t\"guidance_scale\": \"Float(default=7.5, min=0.0, optional=true)\",\r\n\t\t \t\t\"skin\": \"Bool(default=true, optional=true, help='whether to include skin in mask)\",\r\n            \t\"l_brow\": \"Bool(default=false, optional=true, help='whether to include left eyebrow in mask)\"\r\n\r\n            },\r\n            \"outputs\": {\r\n                \"output_img\": \"Image\"\r\n            }\r\n        },\r\n        \"refine_image\": {\r\n            \"call\": \"execute2\",\r\n            \"inputs\": {\r\n                \"prompt\": \"Text\",\r\n                \"img\": \"Image\",\r\n                \"iterations\": \"Int(default=20, min=1, optional=true)\",\r\n                \"height\": \"Int(default=512, min=16, optional=true)\",\r\n                \"width\": \"Int(default=512, min=16, optional=true)\"\r\n            },\r\n            \"outputs\": {\r\n                \"output_img\": \"Image\"\r\n            }\r\n        }\r\n    }\r\n}"};

static std::string sBisinetJson {"{\r\n    \"plugin\": {\r\n        \"Name\": \"Face Segmentation (BiSeNet')\",\r\n        \"Version\": \"0.1.0\",\r\n        \"Author\": \"DeepMake\",\r\n        \"Description\": \"Face segmentation using BiSeNet\",\r\n        \"env\": \"bisenet\"\r\n    },\r\n    \"config\": {\r\n        \"model_name\": \"plugin\/bisenet\/res\/cp\/79999_iter.pth\",\r\n        \"model_dtype\": \"fp32\"\r\n    },\r\n    \"endpoints\": {\r\n        \"segment_face\": {\r\n            \"call\": \"execute\",\r\n            \"inputs\": {\r\n                \"img\": \"Image\",\r\n                \"skin\": \"Bool(default=true, optional=true, help='whether to include skin in mask')\",\r\n                \"l_brow\": \"Bool(default=false, optional=true, help='whether to include left eyebrow in mask')\",\r\n                \"r_brow\": \"Bool(default=false, optional=true, help='whether to include right eyebrow in mask')\",\r\n                \"l_eye\": \"Bool(default=false, optional=true, help='whether to include left eye in mask')\",\r\n                \"r_eye\": \"Bool(default=false, optional=true, help='whether to include right eye in mask')\",\r\n                \"eye_g\": \"Bool(default=false, optional=true, help='whether to include eye glasses in mask')\",\r\n                \"l_ear\": \"Bool(default=false, optional=true, help='whether to include left ear in mask')\",\r\n                \"r_ear\": \"Bool(default=false, optional=true, help='whether to include right ear in mask')\",\r\n                \"ear_r\": \"Bool(default=false, optional=true, help='whether to include ear ring in mask')\",\r\n                \"nose\": \"Bool(default=false, optional=true, help='whether to include nose in mask')\",\r\n                \"mouth\": \"Bool(default=false, optional=true, help='whether to include mouth in mask')\",\r\n                \"u_lip\": \"Bool(default=false, optional=true, help='whether to include upper lip in mask')\",\r\n                \"l_lip\": \"Bool(default=false, optional=true, help='whether to include lower lip in mask')\",\r\n                \"neck\": \"Bool(default=false, optional=true, help='whether to include neck in mask')\",\r\n                \"neck_l\": \"Bool(default=false, optional=true, help='whether to include necklaces in mask')\",\r\n                \"cloth\": \"Bool(default=false, optional=true, help='whether to include clothing in mask')\",\r\n                \"hair\": \"Bool(default=false, optional=true, help='whether to include hair in mask')\",\r\n                \"hat\": \"Bool(default=false, optional=true, help='whether to include hats in mask')\"\r\n            },\r\n            \"outputs\": {\r\n                \"output_mask\": \"Image\"\r\n            }\r\n        }\r\n    }\r\n}"};

class JsonParsingTest: public Test
{
};


TEST(JsonParsingTest, ParseStableDiffusionConfigJson) {

    ArkPlugin plugin;
    PluginJsonParser configParser;
    bool success = configParser.parseConfigJson(sConfigJson, plugin);
    ASSERT_TRUE(success);
    
    EXPECT_EQ(plugin.config.model_name, "runwayml/stable-diffusion-v1-5");
    EXPECT_EQ(plugin.config.save_output, true);
    EXPECT_EQ(plugin.config.model_dtype, "fp32");
}

void CheckData(const Data &data_obj, const std::string &name, const std::string &data)
{
    EXPECT_EQ(data_obj.name, name);
    EXPECT_EQ(data_obj.parameter_data, data);

}

TEST(JsonParsingTest, ParseStableDiffusionInfoJson) {
    ArkPlugin plugin;
    PluginJsonParser configParser;
    bool success = configParser.parsePluginInfo(sGetInfoJson, plugin);
    ASSERT_TRUE(success);

    EXPECT_EQ(plugin.name, "Stable Diffusion (diffusers)");
    EXPECT_EQ(plugin.version, "0.1.0");
    EXPECT_EQ(plugin.author, "DeepMake");
    EXPECT_EQ(plugin.description, "Stable Diffusion generation using the Diffusers library");
    EXPECT_EQ(plugin.env, "sd");

    EXPECT_EQ(plugin.config.model_name, "runwayml/stable-diffusion-v1-5");
    EXPECT_EQ(plugin.config.save_output, true);
    EXPECT_EQ(plugin.config.model_dtype, "fp32");

    ASSERT_EQ(plugin.endpoints.size(), 2);
    
    EXPECT_EQ(plugin.endpoints[0].name, "generate_image");
    EXPECT_EQ(plugin.endpoints[0].call, "execute");
    CheckData(plugin.endpoints[0].inputs[0], "prompt", "Text");
    CheckData(plugin.endpoints[0].inputs[1], "seed", "Int(default=None, optional=true)");
    CheckData(plugin.endpoints[0].inputs[2], "iterations", "Int(default=20, min=1, optional=true)");
    CheckData(plugin.endpoints[0].inputs[3], "height", "Int(default=512, min=16, optional=true)");
    CheckData(plugin.endpoints[0].inputs[4], "width", "Int(default=512, min=16, optional=true)");
    CheckData(plugin.endpoints[0].inputs[5], "guidance_scale", "Float(default=7.5, min=0.0, optional=true)");
    CheckData(plugin.endpoints[0].outputs[0], "output_img", "Image");

    EXPECT_EQ(plugin.endpoints[1].name, "refine_image");
    EXPECT_EQ(plugin.endpoints[1].call, "execute2");
    CheckData(plugin.endpoints[1].inputs[0], "prompt", "Text");
    CheckData(plugin.endpoints[1].inputs[1], "img", "Image");
    CheckData(plugin.endpoints[1].inputs[2], "iterations", "Int(default=20, min=1, optional=true)");
    CheckData(plugin.endpoints[1].inputs[3], "height", "Int(default=512, min=16, optional=true)");
    CheckData(plugin.endpoints[1].inputs[4], "width", "Int(default=512, min=16, optional=true)");
    CheckData(plugin.endpoints[1].outputs[0], "output_img", "Image");
}

template <typename T>
void TestParam(const EndpointParam &param,
                const std::string &name,
                ParameterType paramType,
                T defaultVal,
                T min,
                bool optional)
{
    EXPECT_EQ(param.name, name);
    EXPECT_EQ(param.type, paramType);
    EXPECT_EQ(std::get<T>(param.defaultVal), defaultVal);
    EXPECT_EQ(std::get<T>(param.min), min);
    EXPECT_EQ(param.optional, optional);
}

TEST(JsonParsingTest, ParseInputs) {
    
    ArkPlugin plugin;
    PluginJsonParser configParser;
    bool success = configParser.parsePluginInfo(sGetInfoJson, plugin);
    ASSERT_TRUE(success);
    
    std::vector<EndpointParam>& params = plugin.endpoints[0].inputParams();
    
    EXPECT_EQ(params[0].name, "prompt");
    EXPECT_EQ(params[0].type, ParameterType::Text);

    EXPECT_EQ(params[1].name, "seed");
    EXPECT_EQ(params[1].type, ParameterType::IntSlider);
    EXPECT_EQ(params[1].optional, true);
    
    TestParam(params[2],
              "iterations",
              ParameterType::IntSlider,
              20,
              1,
              true);

    TestParam(params[3],
              "height",
              ParameterType::IntSlider,
              512,
              16,
              true);
    
    TestParam(params[4],
              "width",
              ParameterType::IntSlider,
              512,
              16,
              true);
    
    TestParam(params[5],
              "guidance_scale",
              ParameterType::FloatSlider,
              7.5f,
              0.0f,
              true);
}

TEST(JsonParsingTest, ParseBisinetJson) {
    ArkPlugin plugin;
    PluginJsonParser configParser;
    bool success = configParser.parsePluginInfo(sBisinetJson, plugin);
    ASSERT_TRUE(success);

    EXPECT_EQ(plugin.name, "Face Segmentation (BiSeNet')");
    EXPECT_EQ(plugin.version, "0.1.0");
    EXPECT_EQ(plugin.author, "DeepMake");
    EXPECT_EQ(plugin.description, "Face segmentation using BiSeNet");
    EXPECT_EQ(plugin.env, "bisenet");

    EXPECT_EQ(plugin.config.model_name, "plugin/bisenet/res/cp/79999_iter.pth");
    EXPECT_EQ(plugin.config.model_dtype, "fp32");

    ASSERT_EQ(plugin.endpoints.size(), 1);
    
    EXPECT_EQ(plugin.endpoints[0].name, "segment_face");
    EXPECT_EQ(plugin.endpoints[0].call, "execute");
    CheckData(plugin.endpoints[0].inputs[0], "img", "Image");
    CheckData(plugin.endpoints[0].inputs[1], "skin", "Bool(default=true, optional=true, help='whether to include skin in mask')");
    CheckData(plugin.endpoints[0].inputs[2], "l_brow", "Bool(default=false, optional=true, help='whether to include left eyebrow in mask')");
    CheckData(plugin.endpoints[0].inputs[3], "r_brow", "Bool(default=false, optional=true, help='whether to include right eyebrow in mask')");
    CheckData(plugin.endpoints[0].inputs[4], "l_eye", "Bool(default=false, optional=true, help='whether to include left eye in mask')");
    CheckData(plugin.endpoints[0].outputs[0], "output_mask", "Image");
    
    EXPECT_EQ(plugin.endpoints[0].has_prompt, false);
    EXPECT_EQ(plugin.endpoints[0].outputIsMask(), true);
}
