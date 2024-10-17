

#include <gtest/gtest.h>
#include "parameter.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

using namespace ::testing;

std::string rapidjsonToString(const rapidjson::Value& value) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    value.Accept(writer);
    return buffer.GetString();
}

class ParamsParsingTest : public Test
{
};

TEST(JsonParsingTest, FloatParamTest)
{
    ParameterPtr param = createFloatParam(666,
                                          "deepmake.myplugin.float.param",
                                          "my float param",
                                          7.654f,
                                          0.0f,
                                          10.0f);

    std::string json = rapidjsonToString(param->asJson());
    ParameterPtr testParam = ParamfromJson(json);
    EXPECT_TRUE(*param == *testParam);
}

TEST(JsonParsingTest, IntParamTest)
{
    ParameterPtr param = createIntParam(666,
                                        "deepmake.myplugin.int.param",
                                        "my int param",
                                        5,
                                        -5,
                                        12);

    std::string json = rapidjsonToString(param->asJson());
    ParameterPtr testParam = ParamfromJson(json);
    EXPECT_TRUE(*param == *testParam);
}

TEST(JsonParsingTest, BoolParamTest)
{
    ParameterPtr param = createBoolParam(666,
                                         "deepmake.myplugin.bool.param",
                                         "my bool param",
                                         true);

    std::string json = rapidjsonToString(param->asJson());
    ParameterPtr testParam = ParamfromJson(json);
    EXPECT_TRUE(*param == *testParam);
}

TEST(JsonParsingTest, ButtonParamTest)
{
    ParameterPtr param = createButtonParam(666,
                                           "deepmake.myplugin.button.param",
                                           "my button param");

    std::string json = rapidjsonToString(param->asJson());
    ParameterPtr testParam = ParamfromJson(json);
    EXPECT_TRUE(*param == *testParam);
}

TEST(JsonParsingTest, PointParamTest)
{
    Point2D defaultPoint{.5, .5};
    ParameterPtr param = createPoint2DParam(666,
                                            "deepmake.myplugin.2dPoint.param",
                                            "my Point param",
                                            defaultPoint);

    std::string json = rapidjsonToString(param->asJson());
    ParameterPtr testParam = ParamfromJson(json);
    EXPECT_TRUE(*param == *testParam);
}

TEST(JsonParsingTest, ColorParamTest)
{
    Color defaultColor{1.0f, .5f, .5f, .75f};
    ParameterPtr param = createColor(666,
                                     "deepmake.myplugin.color.param",
                                     "my color param",
                                     defaultColor);

    std::string json = rapidjsonToString(param->asJson());
    ParameterPtr testParam = ParamfromJson(json);
    EXPECT_TRUE(*param == *testParam);
}

TEST(JsonParsingTest, TextParamTest)
{
    ParameterPtr param = createText(666,
                                    "deepmake.myplugin.text.param",
                                    "my text param",
                                    "My Text Value");

    std::string json = rapidjsonToString(param->asJson());
    ParameterPtr testParam = ParamfromJson(json);
    EXPECT_TRUE(*param == *testParam);
}

TEST(JsonParsingTest, MenuParamTest)
{
    std::vector<std::string> menuItems{"Item1", "Item 2", "Item 3"};
    ParameterPtr param = createMenu(666,
                                    "deepmake.myplugin.menu.param",
                                    "my menu param",
                                    menuItems,
                                    1);

    std::string json = rapidjsonToString(param->asJson());
    ParameterPtr testParam = ParamfromJson(json);
    EXPECT_TRUE(*param == *testParam);
}

TEST(JsonParsingTest, GroupStartParamTest)
{
    ParameterPtr param = createGroupStart(666,
                                          "deepmake.myplugin.group.start.param",
                                          "my group start param");

    std::string json = rapidjsonToString(param->asJson());
    ParameterPtr testParam = ParamfromJson(json);
    EXPECT_TRUE(*param == *testParam);
}

TEST(JsonParsingTest, GroupEndParamTest)
{
    ParameterPtr param = createGroupEnd(666,
                                        "deepmake.myplugin.group.end.param",
                                        "my group end param");

    std::string json = rapidjsonToString(param->asJson());
    ParameterPtr testParam = ParamfromJson(json);
    EXPECT_TRUE(*param == *testParam);
}
