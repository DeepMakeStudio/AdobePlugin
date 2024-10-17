#ifndef PARAMETER_H
#define PARAMETER_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include "ark_types.h"
#include <logger.h>
#include <unordered_map>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

enum class ParameterType {
    IntSlider,
    FloatSlider,
    Menu,
    Boolean,
    Point2D,
    Color,
    Image,
    Image_List,
    Text,
    GroupStart,
    GroupEnd,
    Button,
    Unknown
};

const std::string& parameterTypeToString(ParameterType type);
ParameterType stringToParameterType(const std::string& str);

using ParamValue = std::variant<int, float, bool, Color, Point2D>;

class Parameter 
{
public:
    Parameter(int id, 
            const std::string& stringId, 
            const std::string& displayName)
        : Parameter(id, stringId, displayName, ParameterType::Unknown) 
    {
    }
    Parameter(int id, 
                const std::string& stringId, 
                const std::string& displayName,
                ParameterType type,
                bool bIsCustomDisplayName = false,
                bool bStartsHidden = false)
        : m_id(id) 
        , m_string_id(stringId)
        , m_displayName(displayName)
        , m_type(type)
        , m_bIsCustomDisplayName(bIsCustomDisplayName)
        , m_bStartsHidden(bStartsHidden)
    {
    }
    virtual ~Parameter() = default;

    bool operator==(const Parameter& other) const {
        return m_id == other.m_id &&
               m_string_id == other.m_string_id &&
               m_displayName == other.m_displayName &&
               m_type == other.m_type &&
               m_bIsCustomDisplayName == other.m_bIsCustomDisplayName &&
               m_bStartsHidden == other.m_bStartsHidden;
    }

    bool operator!=(const Parameter& other) const {
        return !(*this == other);
    }

    int getID() const 
    {
        return m_id;
    }
    std::string getStringId() const 
    {
        return m_string_id;
    }
    std::string getDisplayName() const 
    {
        return m_displayName;
    }
    ParameterType getType() const 
    {
        return m_type;
    }
    bool isCustomDisplayName() const
    {
        return m_bIsCustomDisplayName;
    }
    bool startsHidden() const
    {
        return m_bStartsHidden;
    }
    void setStartsHidden(bool bStartsHidden)
    {
        m_bStartsHidden = bStartsHidden;
    }
    void syncwithCachedParam(Parameter *paramPtr);

    virtual rapidjson::Value asJson() const;
    virtual void addJsonValues(rapidjson::Value& obj, rapidjson::Document::AllocatorType& allocator) const;
protected:  
    // to improve compatibility we should probably be storing a map of 
    // string ID to int ID so that we can make sure that the IDs are unique
    // and line up with the same int ID 
    int m_id {-1};
    std::string m_string_id;
    std::string m_displayName {"Unnamed"};
    ParameterType m_type {ParameterType::Unknown};
    bool m_bIsCustomDisplayName {false};
    bool m_bStartsHidden {false};
};

template<typename T>
class NumberParam : public Parameter 
{
public:
    NumberParam(int id, 
                const std::string& stringId, 
                const std::string& displayName, 
                const T& defaultValue,
                const T& minValue, 
                const T& maxValue)
        : Parameter(id, stringId, displayName) 
        , m_defaultValue(defaultValue)
        , m_minValue(minValue)
        , m_maxValue(maxValue) 
    {
        initParamType();
    }

    bool operator==(const NumberParam<T>& other) const {
        return static_cast<const Parameter&>(*this) == static_cast<const Parameter&>(other) &&
               m_defaultValue == other.m_defaultValue &&
               m_minValue == other.m_minValue &&
               m_maxValue == other.m_maxValue;
    }

    bool operator!=(const NumberParam<T>& other) const {
        return !(*this == other);
    }

    void initParamType()
    {
        if (std::is_same<T, int>::value)
            m_type = ParameterType::IntSlider;
        else if (std::is_same<T, float>::value)
            m_type = ParameterType::FloatSlider;
        else if (std::is_same<T, bool>::value)
            m_type = ParameterType::Boolean;
        else if (std::is_same<T, Color>::value)
            m_type = ParameterType::Color;
        else if (std::is_same<T, Point2D>::value)
            m_type = ParameterType::Point2D;
        // Add other type checks here
        else
            LOG_ASSERT(false,"Type Interface Failed");// Default to Unknown if type inference fails
    }

    T getDefaultValue() const 
    {
        return m_defaultValue;
    }
    T getMinValue() const 
    {
        return m_minValue;
    }
    T getMaxValue() const 
    {
        return m_maxValue;
    }

    virtual void addJsonValues(rapidjson::Value& obj, rapidjson::Document::AllocatorType& allocator) const override
    {
        Parameter::addJsonValues(obj, allocator);
        obj.AddMember("defaultValue", m_defaultValue, allocator);
        obj.AddMember("minValue", m_minValue, allocator);
        obj.AddMember("maxValue", m_maxValue, allocator);
    }

private:
    T m_defaultValue;
    T m_minValue;
    T m_maxValue;
};

class MenuParam : public Parameter 
{
public:
    MenuParam(int id, 
                const std::string& stringId, 
                const std::string& displayName, 
                const std::vector<std::string> &menuItems, 
                const int defaultIndex,
                const int LicenseLevel = 0)
        : Parameter(id, stringId, displayName, ParameterType::Menu) 
        , m_defaultIndex(defaultIndex)
        , m_menuItems(menuItems)
        , m_LicenseLevel(LicenseLevel)
    {
    }

    bool operator==(const MenuParam& other) const {
        return static_cast<const Parameter&>(*this) == static_cast<const Parameter&>(other) &&
               m_defaultIndex == other.m_defaultIndex &&
               m_menuItems == other.m_menuItems;
    }

    bool operator!=(const MenuParam& other) const {
        return !(*this == other);
    }
    
    void syncwithCachedParam(MenuParam *menuParam);
    void updateMenuItems(const std::vector<std::string> &newMenuItems);
    const std::vector<std::string>& getMenuItems() const
    {
        return m_menuItems;
    }
    int getDefaultIndex() const
    {
        return m_defaultIndex;
    }

    virtual void addJsonValues(rapidjson::Value& obj, rapidjson::Document::AllocatorType& allocator) const override;

private:
    int m_defaultIndex {-1};
    std::vector<std::string> m_menuItems;
    int m_LicenseLevel {0};
};

class PointParam : public Parameter
{
public:
    PointParam(int id,
                const std::string& stringId, 
                const std::string& displayName,
                const Point2D &defaultValue)
        : Parameter(id, stringId, displayName, ParameterType::Point2D)
        , m_defaultValue(defaultValue)
    {
    }

    bool operator==(const PointParam& other) const {
        return static_cast<const Parameter&>(*this) == static_cast<const Parameter&>(other) &&
               m_defaultValue.x == other.m_defaultValue.x &&
               m_defaultValue.y == other.m_defaultValue.y;
    }

    bool operator!=(const PointParam& other) const {
        return !(*this == other);
    }

    Point2D getDefaultValue() const
    {
        return m_defaultValue;
    }

    virtual void addJsonValues(rapidjson::Value& obj, rapidjson::Document::AllocatorType& allocator) const override;
private:
    Point2D m_defaultValue;
};

class ColorParam : public Parameter
{
public:
    ColorParam(int id,
                const std::string& stringId, 
                const std::string& displayName,
                const Color &defaultValue)
        : Parameter(id, stringId, displayName, ParameterType::Color)
        , m_defaultValue(defaultValue)
    {
    }

    bool operator==(const ColorParam& other) const {
        return static_cast<const Parameter&>(*this) == static_cast<const Parameter&>(other) &&
        m_defaultValue.r == other.m_defaultValue.r &&
        m_defaultValue.g == other.m_defaultValue.g &&
        m_defaultValue.b == other.m_defaultValue.b &&
        m_defaultValue.a == other.m_defaultValue.a;
    }

    bool operator!=(const ColorParam& other) const {
        return !(*this == other);
    }

    Color getDefaultValue() const
    {
        return m_defaultValue;
    }

    virtual void addJsonValues(rapidjson::Value& obj, rapidjson::Document::AllocatorType& allocator) const override;
private:
    Color m_defaultValue;
};

class TextParam : public Parameter
{
public:
    TextParam(int id,
                const std::string& stringId, 
                const std::string& displayName,
                const std::string &defaultValue,
                bool bIsCustomDisplayName = false,
                bool bStartsHidden = false)
        : Parameter(id, stringId, displayName, ParameterType::Text, bIsCustomDisplayName)
        , m_defaultValue(defaultValue)
        , m_bIsCustomDisplayName(bIsCustomDisplayName),
        m_bStartsHidden(bStartsHidden)
    {
    }

    bool operator==(const TextParam& other) const {
        return static_cast<const Parameter&>(*this) == static_cast<const Parameter&>(other) &&
               m_defaultValue == other.m_defaultValue;
    }

    bool operator!=(const TextParam& other) const {
        return !(*this == other);
    }

    std::string showDialog(const std::string &default_prompt = "");
    std::string getDefaultValue() const
    {
        return m_defaultValue;
    }

    virtual void addJsonValues(rapidjson::Value& obj, rapidjson::Document::AllocatorType& allocator) const override;
private:
    std::string m_defaultValue;
    bool m_bIsCustomDisplayName;
    bool m_bStartsHidden;
};

class GroupStartParam : public Parameter
{
public:
    GroupStartParam(int id,
                const std::string& stringId, 
                const std::string& displayName)
        : Parameter(id, stringId, displayName, ParameterType::GroupStart)
    {
    }
    bool operator==(const GroupStartParam& other) const {
        return static_cast<const Parameter&>(*this) == static_cast<const Parameter&>(other);
    }

    bool operator!=(const GroupStartParam& other) const {
        return !(*this == other);
    }
private:
};
class GroupEndParam : public Parameter
{
public:
    GroupEndParam(int id,
                const std::string& stringId, 
                const std::string& displayName)
        : Parameter(id, stringId, displayName, ParameterType::GroupEnd)
    {
    }
    bool operator==(const GroupEndParam& other) const {
        return static_cast<const Parameter&>(*this) == static_cast<const Parameter&>(other);
    }

    bool operator!=(const GroupEndParam& other) const {
        return !(*this == other);
    }
private:
};


using ParameterPtr = std::shared_ptr<Parameter>;
using MenuParamPtr = std::shared_ptr<MenuParam>;
using TextParamPtr = std::shared_ptr<TextParam>;

ParameterPtr ParamfromJson(const std::string& json);

ParameterPtr createFloatParam(int id, 
                                const std::string& stringId, 
                                const std::string& displayName, 
                                const float& defaultValue,
                                const float& minValue, 
                                const float& maxValue);
ParameterPtr createIntParam(int id, 
                                const std::string& stringId, 
                                const std::string& displayName, 
                                const int& defaultValue,
                                const int& minValue, 
                                const int& maxValue);
ParameterPtr createBoolParam(int id, 
                                const std::string& stringId, 
                                const std::string& displayName, 
                                const bool& defaultValue);
ParameterPtr createButtonParam(int id, 
                                const std::string& stringId, 
                                const std::string& displayName,
                                const bool bIsCustomDisplayName = false,
                                const bool bStartsHidden = false);
ParameterPtr createPoint2DParam(int id, 
                                const std::string& stringId, 
                                const std::string& displayName, 
                                const Point2D& defaultValue);
ParameterPtr createColor(int id, 
                        const std::string& stringId, 
                        const std::string& displayName, 
                        const Color& defaultValue);
ParameterPtr createText(int id, 
                        const std::string& stringId, 
                        const std::string& displayName, 
                        const std::string& defaultValue,
                        const bool bIsCustomDisplayName = false,
                        const bool bStartsHidden = false);
MenuParamPtr createMenu(int id,
                        const std::string& stringId, 
                        const std::string& displayName,
                        const std::vector<std::string>& menuItems,
                        const int defaultIndex);
ParameterPtr createGroupStart(int id,
                        const std::string& stringId, 
                        const std::string& displayName);
ParameterPtr createGroupEnd(int id,
                        const std::string& stringId, 
                        const std::string& displayName);

#endif // PARAMETER_H
