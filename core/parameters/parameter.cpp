#include "parameter.h"
#include "utils.h"
#include <cstdio>
#include <iostream>
#include "logger.h"
#include <stdexcept>

#ifdef _WIN32
    std::string sCommand = "C:\\Program Files\\DeepMake\\Prompt\\bin\\appPrompt.exe";
#else
    std::string sCommand = "/Applications/appPrompt.app/Contents/MacOS/appPrompt";
#endif

static const std::string sIntSliderStr = "IntSlider";
static const std::string sFloatSliderStr = "FloatSlider";
static const std::string sMenuStr = "Menu";
static const std::string sBooleanStr = "Boolean";
static const std::string sPoint2DStr = "Point2D";
static const std::string sColorStr = "Color";
static const std::string sImageStr = "Image";
static const std::string sTextStr = "Text";
static const std::string sGroupStartStr = "GroupStart";
static const std::string sGroupEndStr = "GroupEnd";
static const std::string sButtonStr = "Button";
static const std::string sUnknownStr = "Unknown";

const std::unordered_map<std::string, ParameterType> typeStringMap = {
    {sIntSliderStr, ParameterType::IntSlider},
    {sFloatSliderStr, ParameterType::FloatSlider},
    {sMenuStr, ParameterType::Menu},
    {sBooleanStr, ParameterType::Boolean},
    {sPoint2DStr, ParameterType::Point2D},
    {sColorStr, ParameterType::Color},
    {sImageStr, ParameterType::Image},
    {sTextStr, ParameterType::Text},
    {sGroupStartStr, ParameterType::GroupStart},
    {sGroupEndStr, ParameterType::GroupEnd},
    {sButtonStr, ParameterType::Button},
    {sUnknownStr, ParameterType::Unknown}
};

const std::string& parameterTypeToString(ParameterType type) 
{
    switch (type) 
    {
        case ParameterType::IntSlider: return sIntSliderStr;
        case ParameterType::FloatSlider: return sFloatSliderStr;
        case ParameterType::Menu: return sMenuStr;
        case ParameterType::Boolean: return sBooleanStr;
        case ParameterType::Point2D: return sPoint2DStr;
        case ParameterType::Color: return sColorStr;
        case ParameterType::Image: return sImageStr;
        case ParameterType::Text: return sTextStr;
        case ParameterType::GroupStart: return sGroupStartStr;
        case ParameterType::GroupEnd: return sGroupEndStr;
        case ParameterType::Button: return sButtonStr;
        default: return sUnknownStr;
    }
}

ParameterType stringToParameterType(const std::string& str) 
{
    auto it = typeStringMap.find(str);
    if (it != typeStringMap.end()) 
    {
        return it->second;
    } 
    else 
    {
        return ParameterType::Unknown;
    }
}

void Parameter::syncwithCachedParam(Parameter *cachedParam)
{
    if (cachedParam)
    {
        //String id should already be the same
        LOG_ASSERT(m_id == cachedParam->m_id, "Int ID mismatch");
        LOG_ASSERT(m_string_id == cachedParam->m_string_id, "String ID mismatch");
    }
}

rapidjson::Value Parameter::asJson() const
{
    static rapidjson::MemoryPoolAllocator<> sAllocator;
    rapidjson::Value obj(rapidjson::kObjectType);
    addJsonValues(obj, sAllocator);
    return obj;
}

void Parameter::addJsonValues(rapidjson::Value& obj, rapidjson::Document::AllocatorType& allocator) const
{
    obj.AddMember("id", m_id, allocator);
    obj.AddMember("stringId", rapidjson::StringRef(m_string_id.c_str()), allocator);
    obj.AddMember("displayName", rapidjson::StringRef(m_displayName.c_str()), allocator);
    obj.AddMember("type", rapidjson::StringRef(parameterTypeToString(m_type).c_str()), allocator);
}

void MenuParam::syncwithCachedParam(MenuParam *cachedParam)
{
    
    if (cachedParam)
    {
        Parameter::syncwithCachedParam(cachedParam);
        if (cachedParam->getMenuItems().size() > m_menuItems.size())
        {
            const auto cachedMenuItems = cachedParam->getMenuItems();
            for (const auto &menuItem : cachedMenuItems)
            {
                if (std::find(m_menuItems.begin(), m_menuItems.end(), menuItem) == m_menuItems.end())
                {
                    if (menuItem.find("(missing)") == std::string::npos)
                    {
                        m_menuItems.push_back(menuItem + "(missing)");
                        LogWarning("Menu item missing: " + menuItem);
                    }
                    if (cachedMenuItems.size() == m_menuItems.size())
                    {
                        // No need to add all items we just need the count to be consistent.
                        return;
                    }
                }
            }
        }
    }
}

void MenuParam::updateMenuItems(const std::vector<std::string> &newMenuItems)
{
    m_menuItems = newMenuItems;
}

void MenuParam::addJsonValues(rapidjson::Value& obj, rapidjson::Document::AllocatorType& allocator) const
{
    Parameter::addJsonValues(obj, allocator);
    obj.AddMember("defaultIndex", m_defaultIndex, allocator);

    rapidjson::Value menuItemsArray(rapidjson::kArrayType);
    for(const std::string& menuItem : m_menuItems) {
        rapidjson::Value menuItemValue;
        menuItemValue.SetString(menuItem.c_str(), static_cast<rapidjson::SizeType>(menuItem.length()), allocator);
        menuItemsArray.PushBack(menuItemValue, allocator);
    }
    obj.AddMember("menuItems", menuItemsArray, allocator);
}

void PointParam::addJsonValues(rapidjson::Value& obj, rapidjson::Document::AllocatorType& allocator) const
{
    Parameter::addJsonValues(obj, allocator);
    obj.AddMember("x", m_defaultValue.x, allocator);
    obj.AddMember("y", m_defaultValue.y, allocator);
}

void ColorParam::addJsonValues(rapidjson::Value& obj, rapidjson::Document::AllocatorType& allocator) const
{
    Parameter::addJsonValues(obj, allocator);
    obj.AddMember("r", m_defaultValue.r, allocator);
    obj.AddMember("g", m_defaultValue.g, allocator);
    obj.AddMember("b", m_defaultValue.b, allocator);
    obj.AddMember("a", m_defaultValue.a, allocator);
}

void TextParam::addJsonValues(rapidjson::Value& obj, rapidjson::Document::AllocatorType& allocator) const
{
    Parameter::addJsonValues(obj, allocator);
    obj.AddMember("defaultValue", rapidjson::StringRef(m_defaultValue.c_str()), allocator);
}

std::string TextParam::showDialog(const std::string &default_prompt)
{
    std::string command = sCommand;
    if (!default_prompt.empty())
    {
        command += " -p ";
        command += "\"" + default_prompt + "\"";
    }   

    std::string retString = executeCommand(command);
    //Remove all newlines
    retString.erase(std::remove(retString.begin(), retString.end(), '\r'), retString.end());
    retString.erase(std::remove(retString.begin(), retString.end(), '\n'), retString.end());
    retString.erase(std::remove(retString.begin(), retString.end(), '\r'), retString.end()); //Just in case I got it backward
    return retString;
}

ParameterPtr createFloatParam(int id, 
                                const std::string& stringId, 
                                const std::string& displayName, 
                                const float& defaultValue,
                                const float& minValue, 
                                const float& maxValue)
{
    return std::make_shared< NumberParam<float> >(id, stringId, displayName, defaultValue, minValue, maxValue);
}
ParameterPtr createIntParam(int id, 
                                const std::string& stringId, 
                                const std::string& displayName, 
                                const int& defaultValue,
                                const int& minValue, 
                                const int& maxValue)
{
    return std::make_shared< NumberParam<int> >(id, stringId, displayName, defaultValue, minValue, maxValue);
}
ParameterPtr createBoolParam(int id, 
                                const std::string& stringId, 
                                const std::string& displayName, 
                                const bool& defaultValue)
{
    return std::make_shared< NumberParam<bool> >(id, stringId, displayName, defaultValue, 0, 1);
}

ParameterPtr createButtonParam(int id, 
                                const std::string& stringId, 
                                const std::string& displayName,
                                bool bIsCustomDisplayName,
                                bool bStartsHidden)
{
    return std::make_shared<Parameter>(id, stringId, displayName, ParameterType::Button, bIsCustomDisplayName, bStartsHidden);
}

ParameterPtr createPoint2DParam(int id, 
                                const std::string& stringId, 
                                const std::string& displayName, 
                                const Point2D& defaultValue)
{
    return std::make_shared< PointParam >(id, stringId, displayName, defaultValue);
}

ParameterPtr createColor(int id, 
                        const std::string& stringId, 
                        const std::string& displayName, 
                        const Color& defaultValue)
{
    return std::make_shared< ColorParam >(id, stringId, displayName, defaultValue);
}

ParameterPtr createText(int id, 
                        const std::string& stringId, 
                        const std::string& displayName, 
                        const std::string& defaultValue,
                        bool bIsCustomDisplayName,
                        bool bStartsHidden)
{
    return std::make_shared< TextParam >(id, stringId, displayName, defaultValue,bIsCustomDisplayName,bStartsHidden);
}

MenuParamPtr createMenu(int id, 
                        const std::string& stringId, 
                        const std::string& displayName,
                        const std::vector<std::string>& menuItems,
                        const int defaultIndex)
{
    return std::make_shared< MenuParam >(id, stringId, displayName, menuItems, defaultIndex);
}

ParameterPtr createGroupStart(int id,
                        const std::string& stringId, 
                        const std::string& displayName)
{
    return std::make_shared< GroupStartParam >(id, stringId, displayName);
}

ParameterPtr createGroupEnd(int id,
                        const std::string& stringId, 
                        const std::string& displayName)
{
    return std::make_shared< GroupEndParam >(id, stringId, displayName);
}

std::string getStringValue(rapidjson::Document &doc, const char *name)
{
    if (!doc.HasMember(name) || !doc[name].IsString())
    {
        LogError("getStringValue() JSON does not have a " + std::string(name));
        throw std::invalid_argument("getStringValue() JSON does not have a " + std::string(name));
    }
    return doc[name].GetString();
}

float getFloatValue(rapidjson::Document &doc, const char *name)
{
    if (!doc.HasMember(name) || !doc[name].IsFloat()) 
    {
        LogError("getFloatValue() JSON does not have a " + std::string(name));
        throw std::invalid_argument("getFloatValue() JSON does not have a " + std::string(name));
    }
    return doc[name].GetFloat();
}

int getIntValue(rapidjson::Document &doc, const char *name)
{
    if (!doc.HasMember(name) || !doc[name].IsInt())
    {
        LogError("getIntValue() JSON does not have a " + std::string(name));
        throw std::invalid_argument("getIntValue() JSON does not have a " + std::string(name));
    }
    return doc[name].GetInt();
}

ParameterType getParamTypeValue(rapidjson::Document &doc, const char *name)
{
    if (!doc.HasMember(name) || !doc[name].IsString())
    {
        LogError("getParamTypeValue() JSON does not have a " + std::string(name));
        throw std::invalid_argument("getParamTypeValue() JSON does not have a " + std::string(name));
    }
    return stringToParameterType(doc[name].GetString());
}

bool getParamBoolValue(rapidjson::Document &doc, const char *name)
{
    if (!doc.HasMember(name) || !doc[name].IsBool())
    {
        LogError("getParamTypeValue() JSON does not have a " + std::string(name));
        throw std::invalid_argument("getParamTypeValue() JSON does not have a " + std::string(name));
    }
    return doc[name].GetBool();
}

std::vector<std::string> getStringArrayValue(rapidjson::Document &doc, const char *name)
{
    if (!doc.HasMember(name) || !doc[name].IsArray())
    {
        LogError("getStringArrayValue() JSON does not have a " + std::string(name));
        throw std::invalid_argument("getStringArrayValue() JSON does not have a " + std::string(name));
    }
    
    std::vector<std::string> items;
    for (const auto& menuItem : doc[name].GetArray())
    {
        if (menuItem.IsString())
        {
            items.push_back(menuItem.GetString());
        }
    }
    return items;
}


ParameterPtr ParamfromJson(const std::string& json)
{
    ParameterPtr param;
    try
    {
        rapidjson::Document doc;
        doc.Parse(json.c_str());

        if (!doc.IsObject())
        {
            LogError("ParamfromJson() JSON is not an object: " + json);
            return nullptr;
        }

        int paramId = getIntValue(doc, "id");
        std::string stringId = getStringValue(doc, "stringId");
        std::string displayName = getStringValue(doc, "displayName");
        ParameterType paramType = getParamTypeValue(doc, "type");

        if (paramType == ParameterType::IntSlider)
        {
            int defaultValue = getIntValue(doc, "defaultValue");
            int min = getIntValue(doc, "minValue");
            int max = getIntValue(doc, "maxValue");
            param = createIntParam(paramId, stringId, displayName, defaultValue, min, max);
        }
        else if(paramType == ParameterType::FloatSlider)
        {
            float defaultValue = getFloatValue(doc, "defaultValue");
            float min = getFloatValue(doc, "minValue");
            float max = getFloatValue(doc, "maxValue");
            param = createFloatParam(paramId, stringId, displayName, defaultValue, min, max);
        }
        else if(paramType == ParameterType::Menu)
        {
            int defaultIndex = getIntValue(doc, "defaultIndex");
            std::vector<std::string> menuItems = getStringArrayValue(doc, "menuItems");
            param = createMenu(paramId, stringId, displayName, menuItems, defaultIndex);
        }
        else if(paramType == ParameterType::GroupStart)
        {
            param = createGroupStart(paramId, stringId, displayName);
        }
        else if(paramType == ParameterType::GroupEnd)
        {
            param = createGroupEnd(paramId, stringId, displayName);
        }
        else if(paramType == ParameterType::Boolean)
        {
            bool defaultValue = getParamBoolValue(doc, "defaultValue");
            param = createBoolParam(paramId, stringId, displayName, defaultValue);
        }
        else if(paramType == ParameterType::Button)
        {
            param = createButtonParam(paramId, stringId, displayName,false,false);
        }
        else if(paramType == ParameterType::Text)
        {
            std::string defaultValue = getStringValue(doc, "defaultValue");
            param = createText(paramId, stringId, displayName, defaultValue, false,false);
        }
        else if(paramType == ParameterType::Point2D)
        {
            float x = getFloatValue(doc, "x");
            float y = getFloatValue(doc, "y");
            Point2D defaultPoint{x, y};
            param = createPoint2DParam(paramId, stringId, displayName, defaultPoint);
        }
        else if(paramType == ParameterType::Color)
        {
            float r = getFloatValue(doc, "r");
            float g = getFloatValue(doc, "g");
            float b = getFloatValue(doc, "b");
            float a = getFloatValue(doc, "a");
            Color defaultColor{r, g, b, a};
            param = createColor(paramId, stringId, displayName, defaultColor);
        }
    }
    catch (const std::exception& e)
    {
        LogError("ParamfromJson() exception: " + std::string(e.what()));
    }
    catch (...)
    {
        LogError("ParamfromJson() unknown exception");
    }
    
    return param;
}
