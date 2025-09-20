#pragma once
#include <string>
#include <any>
// #define CHECK_RTTI

using service_t = std::any;
class IPlugin;

class IPluginHost {
public:
    virtual ~IPluginHost() = default;
    virtual std::string registerService(IPlugin* pPlugin, const std::string& name, const service_t& service) = 0;
    virtual void unregisterService(IPlugin* pPlugin, const std::string& serviceId) = 0;
    virtual service_t queryService(const std::string& name) = 0;
};

class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual const char* name() const = 0;
    virtual bool onLoad(IPluginHost* host) = 0;
    virtual void onUnload() = 0;
};

using create_plugin_fn = IPlugin* (*)();
using destroy_plugin_fn = void (*)(IPlugin*);
using plugin_api_version_fn = int (*)();

constexpr int PLUGIN_API_VERSION = 1;