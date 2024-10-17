#include <vector>
#include <string>
#include <map>
#include "plugin_json_parser.h"
#include "image_buffer.h"
#ifdef _WIN32
#include <Windows.h>
#endif

class ApiConnection
{
public:
    bool isBackendRunning() const;

    //license & subscription status
    bool getLoginStatus() const;
    std::string getUserInfo() const;
    int getUserSubscriptionLevel() const;
    int getPluginSubscriptionLevelRequirement(const std::string &plugin_name) const;

    std::vector<std::string>  getPluginList() const;
    bool getPluginConfig(const std::string &plugin_name, struct ArkPlugin &plugin) const;
    bool getPluginInfo(const std::string &plugin_name, struct ArkPlugin &plugin) const;

    bool startPlugin(const std::string &plugin_name) const;
    bool stopPlugin(const std::string &plugin_name) const;
    bool setConfig(const std::string &plugin_name, const std::string &config_text) const;

    bool setData(const std::string &id, const std::string &data) const;
    bool deleteData(const std::string &id) const;
    std::string getData(const std::string &id) const;
    bool hasShutdownGracefully() const;
    bool uploadImage(const ArkImagePtr &image, std::string &out_id) const;
    bool uploadMultipleImages(const std::vector<ArkImagePtr> &images, std::vector<std::string> &out_ids) const;
    
    bool startBackend() const;
    bool shutdownBackend() const;
    
    std::string executeEndpoint(const std::string &plugin_name, const std::string &args) const;

    std::string callEndpoint(const std::string &plugin_name, const std::string &endpoint, const std::string &body) const;

    JobStatusResponse jobStatus(const std::string &job_id) const;
    ArkImagePtr getImage(const std::string &img_id) const;
    

    void openConfigMenu(std::string pluginName);
    void openPluginManagerMenu();
    void openReportIssueMenu();
    void openSupportURL(std::string pluginName);
    void openDownloadURL();
    void openLoginMenu();
protected:
    std::string getBackendConfigPath() const;
    bool startupBackend(const BackendConfig &config) const;

    std::vector<std::string> parsePluginList(const std::string &plugin_json_list) const;
    std::map<std::string, std::string> parsePluginInfo(const std::string &plugin_config_json) const;
    std::map<std::string, std::string> parsePluginConfig(const std::string &plugin_config_json) const;
    // should this be in the config file so users can set the server IP and port if different from the default?
    std::string m_base_url = "http://localhost:8000/";

private:

    bool validateConfigJson(const std::string& json) const;// failing in the backend 
    bool validatePluginListResponse(const std::string& response) const;
    bool validateInfoResponse(const std::string& response) const;
    bool validateJobStatusResponse(const std::string& response) const;
    bool validateImageResponse(const std::string& response) const;
    bool validateEndpointResponse(const std::string& response) const;
    bool validatePluginResponse(const std::string& response) const;
    bool validateImgUploadResponse(const std::string& response) const;
    bool validateMultipleImgUploadResponse(const std::string& response) const;
};
