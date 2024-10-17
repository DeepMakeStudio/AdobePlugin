#include <memory>

#include "ai_renderer_video_filter.h"
#include "main_api_connection.h"
#include "parameter.h"
#include "video_host.h"
#include "utils.h"
#include "images/image_utils.h"
#include "logger.h"
#include <iostream>
#include <thread>
#include <unordered_set>
#define PLUGIN_MENU_STRING_ID "plugin.menu"
#define PLUGIN_ENDPOINT_START_STRING_ID ".endpoint.group.start"
#define PLUGIN_ENDPOINT_END_STRING_ID ".endpoint.group.end"
#define FILTER_START_GROUP_STRING_ID ".filter.start.group"
#define FILTER_END_GROUP_STRING_ID ".endpoint.group.end"
#define FILTER_ENDPOINT_MENU_STRING_ID ".endpoint.menu"
#define SELECT_PLUGIN_MESSAGE "Select A Plugin"
#define PLUGIN_CONFIG_STRING_ID "Plugin Configuration"

std::vector<FilterConfig> AIRendererFilter::s_filter_configs;
ParamCache AIRendererFilter::s_param_cache{kAIREndererMatchName};

std::string selectedEndpointId(const std::string &plugin_name, const std::string &endpoint_name)
{
     return plugin_name + "." + endpoint_name;
}

std::string pluginStartGroupId(const std::string &plugin_name)
{
     return plugin_name + FILTER_START_GROUP_STRING_ID;
}

std::string pluginEndGroupId(const std::string &plugin_name)
{
     return plugin_name + FILTER_END_GROUP_STRING_ID;
}

std::string endpointMenuId(const std::string &plugin_name)
{
     return plugin_name + FILTER_ENDPOINT_MENU_STRING_ID;
}

bool AIRendererFilter::initialize()
{
     LogInfo("Initializing AI Renderer Filter\n");
     ApiConnection api_connection;

     if (!IsBackendStarted() && !hasBackendStartupTimedout(K_TIMEOUT_MAX_ATTEMPTS))
     {
          api_connection.startBackend();
          InitSentryLogging();

          if (!IsBackendStarted())
          {
               LogError("Error starting backend\n");
          }
     }

     if (IsBackendStarted())
     {
          if (!checkForCrash())
          {

               handleCrashCheck();
               setCheckForCrash(true);
          }
          initParameters();
          return true;
     }
     return false;
}

void AIRendererFilter::shutdown()
{
     ApiConnection api_connection;

     if (IsBackendStarted())
          api_connection.shutdownBackend();

     ShutdownSentryLogging();
}

void AIRendererFilter::initParameters()
{

     if (!s_param_cache.initialized())
     {
          s_param_cache.init();
     }

     if (s_filter_configs.empty())
     {
          fetchFilterConfigs();
     }
     m_persistent_params.clear();
     addFilterMenu();
     addFilterParams();
     addMissingParams();
}

void AIRendererFilter::addMissingParams()
{
     // build a list of the params we have
     std::unordered_set<std::string> paramStringIds;
     for (const auto &param : parameters())
     {
          paramStringIds.insert(param->getStringId());
     }
     for (const auto &cacheParam : s_param_cache.paramMap())
     {
          if (paramStringIds.find(cacheParam.first) == paramStringIds.end())
          {
               ParameterPtr param = ParamfromJson(cacheParam.second);
               if (param)
               {
                    param->setStartsHidden(true);
                    addParam(param);
               }
          }
     }
}

void AIRendererFilter::saveParamCache()
{
     s_param_cache.save();
}

void AIRendererFilter::addParam(ParameterPtr param)
{
     if (param)
     {
          VideoFilter::addParam(param);
          s_param_cache.addParam(param);
     }
}

ParamCache &AIRendererFilter::paramCache()
{
     return s_param_cache;
}

void AIRendererFilter::fetchFilterConfigs()
{
     ApiConnection api_connection;
     std::vector<std::string> plugin_list = api_connection.getPluginList();
     for (const auto &plugin_name : plugin_list)
     {
          FilterConfig filter_config(plugin_name);

          if (api_connection.getPluginInfo(plugin_name, filter_config.plugin()))
          {
               s_filter_configs.push_back(filter_config);
          }
     }
     if (!s_filter_configs.empty())
     {
          FilterConfig ChooseAFilter(SELECT_PLUGIN_MESSAGE);
          s_filter_configs.insert(s_filter_configs.begin(), ChooseAFilter);
     }
}

void AIRendererFilter::addFilterMenu()
{
     std::vector<std::string> plugin_list;

     for (const auto &filter_config : s_filter_configs)
     {
          plugin_list.push_back(filter_config.name());
     }
     if (s_filter_configs.empty())
          plugin_list.push_back("None");

     ;
     MenuParamPtr menuParam = createMenu(s_param_cache.getIdForStringId(PLUGIN_MENU_STRING_ID),
                                         PLUGIN_MENU_STRING_ID,
                                         "Plugins",
                                         plugin_list,
                                         0);

     if (s_param_cache.hasParam(PLUGIN_MENU_STRING_ID))
     {
          MenuParamPtr cachedMenu = std::dynamic_pointer_cast<MenuParam>(s_param_cache.getParam(PLUGIN_MENU_STRING_ID));
          menuParam->syncwithCachedParam(cachedMenu.get());
     }
     addParam(menuParam);
}

void AIRendererFilter::AddConfigGroupStart(FilterConfig &filter_config)
{
     std::string groupStartStringId = pluginStartGroupId(filter_config.plugin().plugin_name);
     int param_id = s_param_cache.getIdForStringId(groupStartStringId);
     ParameterPtr start_group = createGroupStart(param_id,
                                                 groupStartStringId,
                                                 filter_config.name());

     if (s_param_cache.hasParam(groupStartStringId))
     {
          ParameterPtr cachedParam = s_param_cache.getParam(groupStartStringId);
          start_group->syncwithCachedParam(cachedParam.get());
     }
     addParam(start_group);

     filter_config.addParameter(start_group);
}

void AIRendererFilter::AddConfigGroupEnd(FilterConfig &filter_config)
{
     std::string groupEndStringId = pluginEndGroupId(filter_config.plugin().plugin_name);
     int param_id = s_param_cache.getIdForStringId(groupEndStringId);
     ParameterPtr end_group = createGroupEnd(param_id,
                                             groupEndStringId,
                                             filter_config.name());
     if (s_param_cache.hasParam(groupEndStringId))
     {
          ParameterPtr cachedParam = s_param_cache.getParam(groupEndStringId);
          end_group->syncwithCachedParam(cachedParam.get());
     }
     addParam(end_group);
     filter_config.addParameter(end_group);
}

void AIRendererFilter::addFilterParams()
{
     for (auto &filter_config : s_filter_configs)
     {
          AddConfigGroupStart(filter_config);

          if (addEndpointMenu(filter_config))
          {
               addEndpointParams(filter_config);
          }

          AddConfigGroupEnd(filter_config);
     }
}

bool AIRendererFilter::addEndpointMenu(FilterConfig &filter_config)
{
     bool success = false;
     std::vector<std::string> endpoint_menu;
     for (const auto &endpoint : filter_config.plugin().endpoints)
     {
          if (endpoint.tag == "hidden" || endpoint.tag == "deprecated" || endpoint.tag == "ignore")
               continue;

          endpoint_menu.push_back(endpoint.name);
     }
     if (!endpoint_menu.empty())
     {
          std::string endpoint_menu_id = endpointMenuId(filter_config.plugin().plugin_name);
          filter_config.endpoint_param_id = s_param_cache.getIdForStringId(endpoint_menu_id);
          ParameterPtr param = createMenu(filter_config.endpoint_param_id,
                                          endpoint_menu_id,
                                          "Endpoint",
                                          endpoint_menu,
                                          0);

          if (s_param_cache.hasParam(endpoint_menu_id))
          {
               MenuParamPtr cachedMenu = std::dynamic_pointer_cast<MenuParam>(s_param_cache.getParam(endpoint_menu_id));
               param->syncwithCachedParam(cachedMenu.get());
          }

          addParam(param);
          filter_config.addParameter(param);
          success = true;
     }
     return success;
}

void AIRendererFilter::syncParamWithCachedParam(ParameterPtr param)
{
     if (param)
     {
          if (s_param_cache.hasParam(param->getStringId()))
          {
               ParameterPtr cachedParam = s_param_cache.getParam(param->getStringId());
               param->syncwithCachedParam(cachedParam.get());
          }
     }
}

void AIRendererFilter::handleMultiImageRequest(const std::vector<std::string> &img_params, VideoHost &host, Endpoint &endpoint)
{
     int currentCachedFrame = host.getDelegate()->currentFrame();
     ApiConnection api_connection;
     for (const auto &param : img_params)
     {
          LogInfo("Image Param: " + param);

          int frameOffset = endpoint.countInputOccurrences(param, "Image");

          std::vector<ArkImagePtr> images;
          std::vector<std::string> img_ids;

          int direction = (param == "img_before") ? -1 : 1;
          int startOffset = 1;

          for (int i = startOffset; i <= frameOffset; i++)
          {
               int requestedFrame = currentCachedFrame + (i * direction);
               LogInfo(param + ":frame being requested = " + std::to_string(requestedFrame) + ":Current Frame:" + std::to_string(currentCachedFrame));
               images.push_back(host.getImgAtFrame(requestedFrame));
          }

          api_connection.uploadMultipleImages(images, img_ids);

          std::string img_ids_param;
          for (auto it = img_ids.begin(); it != img_ids.end(); ++it)
          {
               if (it == img_ids.begin())
               {
                    img_ids_param += *it + "\",";
               }
               else if (std::next(it) != img_ids.end())
               {
                    img_ids_param += "\"" + *it + "\",";
               }
               else
               {
                    img_ids_param += "\"" + *it;
               }
          }
          endpoint.setInputId(param, img_ids_param);

          for (int i = 0; i < img_ids.size(); i++)
          {
               m_imageMap[param + std::to_string(i)] = img_ids[i];
          }
     }
}

void AIRendererFilter::handleImageRequest(const std::string &img_param, Endpoint &endpoint, ArkImagePtr sourceImg)
{
     ApiConnection api_connection;
     std::string img_id;

     if (api_connection.uploadImage(sourceImg, img_id))
     {
          if (!img_id.empty())
          {
               endpoint.setInputId(img_param, img_id);
          }
     }
}

bool AIRendererFilter::addEndpointParams(FilterConfig &filter_config)
{
     for (auto &endpoint : filter_config.plugin().endpoints)
     {
          std::string endpoint_id = filter_config.plugin().plugin_name + "." + endpoint.name;
          std::string start_group_id = endpoint_id + PLUGIN_ENDPOINT_START_STRING_ID;
          int startParamId = s_param_cache.getIdForStringId(start_group_id);

          ParameterPtr start_group = createGroupStart(startParamId, start_group_id, endpoint.name);
          addParam(start_group);
          filter_config.addParameter(start_group);

          std::vector<EndpointParam> &params = endpoint.inputParams();
          for (auto &endpoint_param : params)
          {
               std::string stringId = endpoint_id + "." + endpoint_param.name;
               LogInfo("checking for param: " + stringId + "\n");
               // if(s_param_cache.hasParam(stringId))
               // {
               //      ParameterPtr param = s_param_cache.getParam(stringId);
               //      if(param)
               //      {
               //           if (param->getType() == ParameterType::Text)
               //                endpoint.has_prompt = true;
               //           syncParamWithCachedParam(param);
               //           addParam(param);
               //           filter_config.addParameter(param);
               //           LogInfo("Param already exists: " + stringId + "\n");
               //           continue;
               //      }
               // }
               LogInfo("Adding Param not found in cache: " + stringId + "\n");
               if (endpoint_param.type == ParameterType::Text)
               {
                    int textParamId = s_param_cache.getIdForStringId(stringId);
                    endpoint.has_prompt = true;
                    endpoint.prompt_param_id = textParamId;
                    endpoint_param.id = textParamId;
                    ParameterPtr param = createText(textParamId,
                                                    stringId,
                                                    endpoint_param.name,
                                                    "");

                    syncParamWithCachedParam(param);
                    addParam(param);
                    filter_config.addParameter(param);
               }
               else if (endpoint_param.type == ParameterType::IntSlider)
               {
                    int intParamId = s_param_cache.getIdForStringId(stringId);
                    endpoint_param.id = intParamId;
                    int defaultVal = std::get<int>(endpoint_param.defaultVal);
                    int min = std::get<int>(endpoint_param.min);
                    int max = std::get<int>(endpoint_param.max);
                    ParameterPtr param = createIntParam(intParamId,
                                                        stringId,
                                                        endpoint_param.name,
                                                        defaultVal != -1 ? defaultVal : 0,
                                                        min != -1 ? min : 0,
                                                        max != -1 ? max : 10000); // need a better max
                    syncParamWithCachedParam(param);
                    addParam(param);
                    filter_config.addParameter(param);
               }
               else if (endpoint_param.type == ParameterType::FloatSlider)
               {
                    int floatParamId = s_param_cache.getIdForStringId(stringId);
                    endpoint_param.id = floatParamId;
                    float defaultVal = std::get<float>(endpoint_param.defaultVal);
                    float min = std::get<float>(endpoint_param.min);
                    float max = std::get<float>(endpoint_param.max);
                    ParameterPtr param = createFloatParam(floatParamId,
                                                          stringId,
                                                          endpoint_param.name,
                                                          defaultVal != -1 ? defaultVal : 0,
                                                          min != -1 ? min : 0,
                                                          max != -1 ? max : 10000); // need a better max
                    syncParamWithCachedParam(param);
                    addParam(param);
                    filter_config.addParameter(param);
               }
               else if (endpoint_param.type == ParameterType::Boolean)
               {
                    int boolParamId = s_param_cache.getIdForStringId(stringId);
                    endpoint_param.id = boolParamId;
                    bool defaultVal = std::get<bool>(endpoint_param.defaultVal);
                    ParameterPtr param = createBoolParam(boolParamId,
                                                         stringId,
                                                         endpoint_param.name,
                                                         defaultVal);
                    syncParamWithCachedParam(param);
                    addParam(param);
                    filter_config.addParameter(param);
               }
               else
               {
                    if (!(endpoint_param.type == ParameterType::Image))
                    {
                         if (endpoint.tag != "ignore" || endpoint.tag != "deprecated" || endpoint.tag != "hidden")
                              LOG_ASSERT(false, "Unkown parameter type was detected, please check input parameters in: " + filter_config.plugin().plugin_name + ", Endpoint: " + endpoint.name);
                    }
               }
          }
          std::string endgroupId = endpoint_id + PLUGIN_ENDPOINT_END_STRING_ID;
          int end_group_id = s_param_cache.getIdForStringId(endgroupId);

          ParameterPtr end_group = createGroupEnd(end_group_id, endgroupId, endpoint.name);
          syncParamWithCachedParam(end_group);
          addParam(end_group);
          filter_config.addParameter(end_group);
     }
     return true;
}

bool AIRendererFilter::getSelectedFilterConfig(VideoHost &host, FilterConfig &filter_config)
{   
     int menuId = filter_config.name() == SELECT_PLUGIN_MESSAGE ? 0 : s_param_cache.getIdForStringId(PLUGIN_MENU_STRING_ID);
     ParamValue menuValue = host.getParamValue(menuId);
     int menuIndex = std::get<int>(menuValue);
     ApiConnection api_connection;
     if (menuIndex < s_filter_configs.size())
     {
          filter_config = s_filter_configs[menuIndex];
          m_selected_filter_name = filter_config.name();
          filter_config.required_license = api_connection.getPluginSubscriptionLevelRequirement(filter_config.plugin().plugin_name);
          LogInfo("Selected filter: " + m_selected_filter_name+ "\n");
          return true;
     }
     return false;
}

bool AIRendererFilter::getSelectedEndpoint(VideoHost &host, FilterConfig &filter_config, Endpoint &out_endpoint)
{
     if (filter_config.endpoint_param_id != -1)
     {
          ParamValue menuValue = host.getParamValue(filter_config.endpoint_param_id);
          int menuIndex = std::get<int>(menuValue);
          if (menuIndex < filter_config.plugin().endpoints.size())
          {
               out_endpoint = filter_config.plugin().endpoints[menuIndex];
               LogInfo("Selected endpoint: " + out_endpoint.name + "\n");
               return true;
          }
     }
     return false;
}

bool AIRendererFilter::getEndpointParams(VideoHost &host, Endpoint &endpoint, std::string &params)
{
     bool success = false;
     params = "{";

     for (const auto &input_param : endpoint.inputParams())
     {
          if (input_param.type == ParameterType::IntSlider)
          {
               ParamValue intParamValue = host.getParamValue(input_param.id);
               int intValue = int(std::get<float>(intParamValue));
               if (params.size() > 1)
               {
                    params += ", ";
               }
               params += "\"" + input_param.name + "\": " + std::to_string(intValue);
          }
          else if (input_param.type == ParameterType::Text)
          {
               std::string prompt = host.getTextPrompt();
               if (params.size() > 1)
               {
                    params += ", ";
               }
               params += "\"prompt\": \"" + prompt + "\"";
               success = true;
          }
          else if (input_param.type == ParameterType::Image_List)
          {
               if (params.size() > 1)
               {
                    params += ", ";
               }
               params += "\"" + input_param.name + "\": [\"" + input_param.string_val + "\"]";
          }
          else if (input_param.type == ParameterType::Image)
          {
               if (params.size() > 1)
               {
                    params += ", ";
               }
               params += "\"" + input_param.name + "\": \"" + input_param.string_val + "\"";
          }
          else if (input_param.type == ParameterType::Boolean)
          {
               ParamValue paramValue = host.getParamValue(input_param.id);
               bool boolValue = bool(std::get<bool>(paramValue));
               if (params.size() > 1)
               {
                    params += ", ";
               }
               params += "\"" + input_param.name + "\": " + (boolValue ? "true" : "false");
          }
          else if (input_param.type == ParameterType::FloatSlider)
          {
               ParamValue paramValue = host.getParamValue(input_param.id);
               float floatValue = std::get<float>(paramValue);
               if (params.size() > 1)
                    params += ", ";
               params += "\"" + input_param.name + "\": " + std::to_string(floatValue);
          }
          else
          {
               const std::string &type_string = parameterTypeToString(input_param.type);
               LOG_ASSERT(false, "AIRendererFilter::getEndpointParams: Unsupported parameter type" + type_string);
          }
     }
     params += "}";
     LogInfo("Endpoint Params: " + params);
     return success;
}

bool AIRendererFilter::hasBackendStartupTimedout(int maxAttempts)
{
     if (AIRendererFilter::startupAttempts() == maxAttempts && !isBackendStarted())
     {
          LogError("Backend startup timed-out, please check the backend", false);
          AIRendererFilter::incrementStartupAttempts();
     }
     if (AIRendererFilter::startupAttempts() < maxAttempts && !isBackendStarted())
     {
          AIRendererFilter::incrementStartupAttempts();
          LogInfo("Backend startup attempt: " + std::to_string(AIRendererFilter::startupAttempts()));
          return false;
     }
     return true;
}

void AIRendererFilter::handleCrashCheck()
{

     ApiConnection api_connection;
     if (api_connection.hasShutdownGracefully())
     {
          LogInfo("No crash detected in previous session");
     }
     else
     {
          ReportPrevCrash("Crash detected in previous session");
     }

     api_connection.setData("shutdown", "{\"shutdown\": \"false\"}");
}

bool AIRendererFilter::render(VideoHost &host)
{
     ArkImagePtr sourceImg = host.sourceImg();
     ArkImagePtr destImg = host.destImg();
     int downSampleX = host.getDelegate()->downsampleX();
     int downSampleY = host.getDelegate()->downSampleY();
     if (IsBackendStarted())
     {
          FilterConfig filter_config;
          if (!getSelectedFilterConfig(host, filter_config))
          {
               LogError("Error getting selected filter config");
               return false;
          }
          if (filter_config.name() == SELECT_PLUGIN_MESSAGE)
          {
               return copyImage(sourceImg, destImg, downSampleX, downSampleY);
          }

          Endpoint endpoint;
          if (!getSelectedEndpoint(host, filter_config, endpoint))
          {
               LogError("Error getting selected filter endpoint");
               return false;
          }
          LogInfo("Endpoint Name:" + endpoint.name);
          ApiConnection api_connection;
          // if the prompt hasn't been set just return the source
          if ( endpoint.has_prompt && host.getTextPrompt().empty())
          {
               LogInfo("Prompt not set, returning source image\n");
               return copyImage(sourceImg, destImg, downSampleX, downSampleY);
          }

          std::string endpoint_params;
          std::string cached_params = host.getCachedParams();
          getEndpointParams(host, endpoint, endpoint_params);
          if (endpoint_params != cached_params)
          {
               LogInfo("Endpoint params have changed, invalidating rendered image\n");
               invalidateRenderedImage(host);
          }

          std::string render_image_id = host.getRenderedImageID();
          if (!render_image_id.empty())
          {
               ArkImagePtr img = api_connection.getImage(render_image_id);
               if (img)
               {
                    LogInfo("Using cached image");
                    copyImage(img, destImg, downSampleX, downSampleY);
                    return true;
               }
          }

          std::string img_param = endpoint.getInputImgParam();
          std::vector<std::string> img_params = endpoint.getInputImgListParams();
          if (img_params.size() > 0)
          {
               handleMultiImageRequest(img_params, host, endpoint);
               getEndpointParams(host, endpoint, endpoint_params);
          }
          else if (!img_param.empty())
          {
               handleImageRequest(img_param, endpoint, sourceImg);
               getEndpointParams(host, endpoint, endpoint_params);
          }

          std::string job_id = api_connection.callEndpoint(filter_config.name(), endpoint.name, endpoint_params);
          if (job_id.empty())
          {
               LogError("Error executing plugin");
               return false;
          }

          int loopCycleCount = 0;
          std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
          // wait for the job to complete
          JobStatusResponse job_response = api_connection.jobStatus(job_id);
          while (job_response.status == JOB_STATUS_IN_PROGRESS)
          {
               job_response = api_connection.jobStatus(job_id);
               arkSleepMS(cachedElasedTime);
               job_response = api_connection.jobStatus(job_id);
               LogInfo("Job Response: " + stringFromJobStatus(job_response.status));
               loopCycleCount += 1;

               if (loopCycleCount > 5)
               {
                    LogError(job_id + ": Job timed out", true);
                    break;
               }
          }

          if (job_response.status == JOB_STATUS_SUCCESS)
          {
               ArkImagePtr img = api_connection.getImage(job_response.img_id);
               if (img)
               {
                    host.setCachedParams(endpoint_params);
                    host.setRenderedImageID(job_response.img_id);

                    if (endpoint.outputIsMask())
                    {
                         copyImage(sourceImg, destImg);
                         if (img->width() < destImg->width() ||
                             img->height() < destImg->height())
                         {
                              ArkImage *resizedImg = resizeImageUp(img.get(), destImg->width(), destImg->height());
                              if (resizedImg)
                                   img.reset(resizedImg);
                         }
                         img->setChannelOrder(AAA);

                         copyAlphaToImage(img, destImg);
                    }
                    else
                    {
                         copyImage(img, destImg, downSampleX, downSampleY);
                    }

                    std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> timeElapsed = end_time - start_time;

                    if (timeElapsed < cachedElasedTime)
                    {
                         cachedElasedTime = std::chrono::duration_cast<std::chrono::seconds>(timeElapsed);
                    }

                    LogInfo("Job took " + std::to_string(timeElapsed.count()) + " s");
                    if (m_imageMap.size() > 0)
                    {
                         m_imageMap.clear();
                         LogInfo("Image Map Cleared");
                    }
                    return true;
               }
          }
          else
          {
               LogError("Job failed: " + stringFromJobStatus(job_response.status), true);
          }

          return false;
     }
     return false;
}

bool AIRendererFilter::handleOverlayEvent(VideoHost &host, MouseEventType eventType, int x, int y)
{
     return false;
}

bool AIRendererFilter::handleOverlayDraw()
{
     return false;
}

ParameterPtr AIRendererFilter::getParamWithStringId(const std::string &stringId)
{
     // TODO Check if not in VideoFilter Parameters
     return VideoFilter::getParamWithStringId(stringId);
}

bool AIRendererFilter::IsBackendStarted()
{
     setBackendStarted(ApiConnection().isBackendRunning());
     return isBackendStarted();
}

void AIRendererFilter::addPersistentParam(ParameterPtr param)
{
     if (param)
     {
          m_persistent_params.push_back(param);
     }
}

void AIRendererFilter::invalidateRenderedImage(VideoHost &host)
{
     host.setRenderedImageID(""); // force to refresh the image
}

bool AIRendererFilter::shouldShowPromptUI(int changedParamID)
{
     if (changedParamID != -1)
     {
          ParameterPtr param = getParam(changedParamID);
          if (param != nullptr)
          {
               if (param->getType() == ParameterType::Text)
                    return true;
          }
     }
     return false;
}

bool AIRendererFilter::showPromptUI(VideoHost &host, int textParamID)
{
     ParameterPtr param = VideoFilter::getParam(textParamID);
     if (param == nullptr)
          return false;

     LOG_ASSERT(param->getType() == ParameterType::Text, "Error getting text parameter");

     if (param->getType() != ParameterType::Text)
          return false;

     std::shared_ptr<TextParam> textParam = std::dynamic_pointer_cast<TextParam>(param);
     std::string prompt_string = host.getTextPrompt();
     std::string new_prompt_string = textParam->showDialog(prompt_string);

     if (new_prompt_string != prompt_string)
     {
          host.setTextPrompt(new_prompt_string);
          invalidateRenderedImage(host);
          return true;
     }
     return false;
}

void AIRendererFilter::hideAllParams(VideoHost &host)
{
     for (const auto &param : getParams())
     {
          // always show the plugin menu
          bool show = param->getStringId() == PLUGIN_MENU_STRING_ID;
          host.hideShowControl(param->getID(), !show);
     }
}

bool AIRendererFilter::updateParams(VideoHost &host, int changedParamID)
{
     if (IsBackendStarted())
     {
          FilterConfig selected_filter_config;
          if (!getSelectedFilterConfig(host, selected_filter_config))
          {
               LogError("Error getting selected filter config");
               hideAllParams(host);
               return false;
          }

          LogInfo("selected filter name: " + selected_filter_config.name());
          if (changedParamID == -1)
          {
               for (auto param : m_persistent_params)
               {
                    if (selected_filter_config.name() == SELECT_PLUGIN_MESSAGE && param->getStringId() == PLUGIN_CONFIG_STRING_ID)
                    {
                         host.hideShowControl(param->getID(), true);
                    }
                    else
                         host.hideShowControl(param->getID(), param->startsHidden());
               }
          }
          if (shouldShowPromptUI(changedParamID))
          {
               return showPromptUI(host, changedParamID);
          }

          // If a param has changed, clear the cached image
          invalidateRenderedImage(host);

          Endpoint selected_endpoint;
          if (!getSelectedEndpoint(host, selected_filter_config, selected_endpoint))
          {
               LogError("Error getting selected filter endpoint");
               hideAllParams(host);
               return false;
          }

          ApiConnection api_connection;
          int license_status = api_connection.getUserSubscriptionLevel();
          bool effect_is_licensed = true;

          if (selected_filter_config.required_license > license_status)
          {
               effect_is_licensed = false;
          }
         
          if (selected_filter_config.name() != SELECT_PLUGIN_MESSAGE)
          {
               std::string selected_endpoint_id = selectedEndpointId(selected_filter_config.plugin().plugin_name, selected_endpoint.name);
               std::string plugin_start_group_id = pluginStartGroupId(selected_filter_config.plugin().plugin_name);
               std::string plugin_end_group_id = pluginEndGroupId(selected_filter_config.plugin().plugin_name);
               std::string endpoint_menu_id = endpointMenuId(selected_filter_config.plugin().plugin_name);

               int menuId = s_param_cache.getIdForStringId(PLUGIN_MENU_STRING_ID);
               ParamValue menuValue = host.getParamValue(menuId);
               int menuIndex = std::get<int>(menuValue);

               // assert(menuIndex < s_filter_configs.size());
               if (menuIndex < s_filter_configs.size())
               {
                    FilterConfig filter_config = s_filter_configs[menuIndex];

                    for (const auto &param : getParams())
                    {
                         bool is_effect_param = param->getStringId().find(selected_endpoint_id + ".") != std::string::npos ||
                                                param->getStringId() == endpoint_menu_id;

                         bool show = (param->getStringId() == PLUGIN_MENU_STRING_ID ||
                                      param->getStringId() == plugin_start_group_id ||
                                      param->getStringId() == plugin_end_group_id ||
                                      is_effect_param ||
                                      param->getStringId().find(selected_endpoint_id + ".") != std::string::npos) &&
                                      !param->startsHidden();

                         host.hideShowControl(param->getID(), !show);
                         
                         if (is_effect_param)
                         {
                              host.enableDisableControl(param->getID(), effect_is_licensed);
                         }
                      
                         LogInfo("Updating Param: " + param->getDisplayName() + " : " + std::to_string(param->getID()));
                    }
                    return true;
               }
          }
          else
          {
               for (const auto &param : getParams())
               {
                    if (param->getStringId() == PLUGIN_CONFIG_STRING_ID)
                    {
                         host.hideShowControl(param->getID(), true);
                    }
               }
               return true;
          }
     }
     return false;
}
