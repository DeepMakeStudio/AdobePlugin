#include "ofx_host_delegate.h"
#include "utils.h"

OFXVideoHostDelegate::OFXVideoHostDelegate(OfxPropertySuiteV1* propHost, 
                                            OfxImageEffectSuiteV1* effectSuite, 
                                            OfxParameterSuiteV1* paramSuite,
                                            OfxImageEffectHandle instance)
: m_propHost(propHost)
, m_effectSuite(effectSuite)
, m_paramSuite(paramSuite)
, m_instance(instance)
{
   setHostName(getHostName());
   m_effectSuite->getParamSet(m_instance, &m_paramSet);
}

int OFXVideoHostDelegate::projectWidth() const
{
    return 0;
}
int OFXVideoHostDelegate::projectHeight() const
{
    return -1;
}
int OFXVideoHostDelegate::downsampleX() const
{
    return 1;
}
int OFXVideoHostDelegate::downSampleY() const
{
    return 1;
}
float OFXVideoHostDelegate::projectFPS() const
{
    return 0.0f;
}
int OFXVideoHostDelegate::durationFrames() const
{
    return -1;
}int OFXVideoHostDelegate::currentFrame() const
{
    return -1;
}
bool OFXVideoHostDelegate::registerParam(const ParameterPtr param)
{
    return false;
}
ParamValue OFXVideoHostDelegate::getParamValue(int paramFilterIndex) const
{
    return ParamValue();
}
int OFXVideoHostDelegate::hostIndexFromParamId(int paramId) const
{
    return -1;
}
    
int OFXVideoHostDelegate::paramIdFromHostIndex(int hostIndex) const
{
    return -1;
}
std::string OFXVideoHostDelegate::getTextPrompt()
{ 
    return "";
}
void OFXVideoHostDelegate::setTextPrompt(const std::string &prompt)
{}
std::string OFXVideoHostDelegate::getRenderedImageID()
{
    return "";
}
void OFXVideoHostDelegate::setRenderedImageID(const std::string &img_id)
{}
std::string OFXVideoHostDelegate::getCachedParams()
{
    return "";
}
void OFXVideoHostDelegate::setCachedParams(const std::string &params)
{ }
ArkImagePtr OFXVideoHostDelegate::sourceImg()
{ 
    return nullptr;
}
ArkImagePtr OFXVideoHostDelegate::destImg()
{ 
    return nullptr;
}

ArkImagePtr OFXVideoHostDelegate::getImgAtFrame(int frame)
{
    return ArkImagePtr();
}

bool OFXVideoHostDelegate::addFloatSlider(int id, 
    const std::string &paramName, 
    float defaultValue,
    float min,
    float max)
{
	OfxStatus retVal = kOfxStatOK;
	try
	{
		OfxPropertySetHandle props;
        std::string string_id = std::to_string(id);
		ofxSuccess(m_paramSuite->paramDefine(m_paramSet, kOfxParamTypeDouble, string_id.c_str(), &props));
        ofxSuccess(m_propHost->propSetString(props, kOfxParamPropDoubleType, 0, kOfxParamDoubleTypePlain));
        ofxSuccess(m_propHost->propSetString(props, kOfxPropLabel, 0, paramName.c_str()));
		ofxSuccess(m_propHost->propSetDouble(props, kOfxParamPropDefault, 0, defaultValue));
		ofxSuccess(m_propHost->propSetDouble(props, kOfxParamPropMin, 0, min));
		ofxSuccess(m_propHost->propSetDouble(props, kOfxParamPropMax, 0, max));
        addToGroup(props);
	}
	catch(OfxStatus err)
	{
		retVal = err;
	}
	return retVal == kOfxStatOK;
}
bool OFXVideoHostDelegate::addIntSlider(int id, 
    const std::string &paramName, 
    int defaultValue,
    int min,
    int max)
{
	OfxStatus retVal = kOfxStatOK;
	try
	{
		OfxPropertySetHandle props;
        std::string string_id = std::to_string(id);
		ofxSuccess(m_paramSuite->paramDefine(m_paramSet, kOfxParamTypeInteger, string_id.c_str(), &props));
        ofxSuccess(m_propHost->propSetString(props, kOfxPropLabel, 0, paramName.c_str()));
		ofxSuccess(m_propHost->propSetInt(props, kOfxParamPropDefault, 0, defaultValue));
		ofxSuccess(m_propHost->propSetInt(props, kOfxParamPropMin, 0, min));
		ofxSuccess(m_propHost->propSetInt(props, kOfxParamPropMax, 0, max));
        addToGroup(props);
	}
	catch(OfxStatus err)
	{
		retVal = err;
	}
	return retVal == kOfxStatOK;
}
bool OFXVideoHostDelegate::addCheckbox(int id, 
    const std::string &paramName, 
    bool defaultValue)
{
	OfxStatus retVal = kOfxStatOK;
	try
	{
		OfxPropertySetHandle props;
        std::string string_id = std::to_string(id);
		ofxSuccess(m_paramSuite->paramDefine(m_paramSet, kOfxParamTypeBoolean, string_id.c_str(), &props));
        ofxSuccess(m_propHost->propSetString(props, kOfxPropLabel, 0, paramName.c_str()));
		ofxSuccess(m_propHost->propSetInt(props, kOfxParamPropDefault, 0, defaultValue));
        addToGroup(props);
	}
	catch(OfxStatus err)
	{
		retVal = err;
	}
	return retVal == kOfxStatOK;
}
bool OFXVideoHostDelegate::addButton(int id, 
    const std::string &paramName,
	const std::string &paramStringId,
    bool bIsCustomDisplayName,
	bool bStartsHidden)
{
	OfxStatus retVal = kOfxStatOK;
	try
	{
		OfxPropertySetHandle props;
        std::string string_id = std::to_string(id);
		ofxSuccess(m_paramSuite->paramDefine(m_paramSet, kOfxParamTypePushButton, string_id.c_str(), &props));
        ofxSuccess(m_propHost->propSetString(props, kOfxPropLabel, 0, paramName.c_str()));
        addToGroup(props);
	}
	catch(OfxStatus err)
	{
		retVal = err;
	}
	return retVal == kOfxStatOK;
}
bool OFXVideoHostDelegate::addPoint(int id, 
    const std::string &paramName, 
    const Point2D &defaultValue)
{
    OfxStatus retVal = kOfxStatOK;
	try
	{
		OfxPropertySetHandle props;
        std::string string_id = std::to_string(id);
		ofxSuccess(m_paramSuite->paramDefine(m_paramSet, kOfxParamTypeDouble2D, string_id.c_str(), &props));
        ofxSuccess(m_propHost->propSetString(props, kOfxPropLabel, 0, paramName.c_str()));
		ofxSuccess(m_propHost->propSetDouble(props, kOfxParamPropDefault, 0, defaultValue.x));
		ofxSuccess(m_propHost->propSetDouble(props, kOfxParamPropDefault, 1, defaultValue.y));
        addToGroup(props);
	}
	catch(OfxStatus err)
	{
		retVal = err;
	}
	return retVal == kOfxStatOK;
}
bool OFXVideoHostDelegate::addColor(int id, 
    const std::string &paramName, 
    const Color &defaultValue)
{
    OfxStatus retVal = kOfxStatOK;
	try
	{
		OfxPropertySetHandle props;
        std::string string_id = std::to_string(id);
		ofxSuccess(m_paramSuite->paramDefine(m_paramSet, kOfxParamTypeRGB, string_id.c_str(), &props));
        ofxSuccess(m_propHost->propSetString(props, kOfxPropLabel, 0, paramName.c_str()));
		ofxSuccess(m_propHost->propSetDouble(props, kOfxParamPropDefault, 0, defaultValue.r));
		ofxSuccess(m_propHost->propSetDouble(props, kOfxParamPropDefault, 1, defaultValue.g));
		ofxSuccess(m_propHost->propSetDouble(props, kOfxParamPropDefault, 2, defaultValue.b));
        addToGroup(props);
	}
	catch(OfxStatus err)
	{
		retVal = err;
	}
	return retVal == kOfxStatOK;
}
bool OFXVideoHostDelegate::addMenu(int id, 
    const std::string &paramName, 
    const std::vector<std::string> &menuItems,
    short defaultIndex)
{
    OfxStatus retVal = kOfxStatOK;
	try
	{
		OfxPropertySetHandle props;
        std::string string_id = std::to_string(id);
		ofxSuccess(m_paramSuite->paramDefine(m_paramSet, kOfxParamTypeChoice, string_id.c_str(), &props));
        ofxSuccess(m_propHost->propSetString(props, kOfxPropLabel, 0, paramName.c_str()));
		for (int index = 0; index < menuItems.size(); index++)
			ofxSuccess(m_propHost->propSetString(props, kOfxParamPropChoiceOption, index, menuItems[index].c_str()));

		ofxSuccess(m_propHost->propSetInt(props, kOfxParamPropDefault, 0, defaultIndex));
		ofxSuccess(m_propHost->propSetInt(props, kOfxParamPropAnimates, 0, 0));
        addToGroup(props);
	}
	catch(OfxStatus err)
	{
		retVal = err;
	}
	return retVal == kOfxStatOK;
}
bool OFXVideoHostDelegate::startGroup(int id, 
    const std::string &paramName,
    bool start_collapsed)
 {
	OfxStatus retVal = kOfxStatOK;
	try
	{
		OfxPropertySetHandle props;
        std::string string_id = std::to_string(id);
		ofxSuccess(m_paramSuite->paramDefine(m_paramSet, kOfxParamTypeGroup, string_id.c_str(), &props));
        addToGroup(props);
        m_groupStack.push(paramName);
	}
	catch(OfxStatus err)
	{
		retVal = err;
	}
	return retVal == kOfxStatOK;
}   
bool OFXVideoHostDelegate::endGroup(int id, 
    const std::string &paramName)
{
    m_groupStack.pop();
	return true;
}
void OFXVideoHostDelegate::hideShowControl(int id, bool hide)
{

}
void OFXVideoHostDelegate::enableDisableControl(int id, bool enable)
{
	try
	{
		OfxPropertySetHandle props;
        std::string string_id = std::to_string(id);

        OfxParamSetHandle paramSet;
        m_effectSuite->getParamSet(m_instance, &paramSet);
        OfxParamHandle param; OfxPropertySetHandle paramProps;
        //TODO
        //m_propHost->paramGetHandle(paramSet, paramName, &param, &paramProps);
	}
	catch(OfxStatus err)
	{
	}
}
std::string OFXVideoHostDelegate::getHostName()
{
    return "OFX";
}
void OFXVideoHostDelegate::addToGroup(OfxPropertySetHandle props)
{
    if (!m_groupStack.empty())
    {
        std::string group_name = m_groupStack.top();
        ofxSuccess(m_propHost->propSetString(props, kOfxParamPropParent, 0, group_name.c_str()));
    }
}
