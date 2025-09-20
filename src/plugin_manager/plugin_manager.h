#pragma once
#include "plugin_shared.hpp"
#include <vector>
#include <unordered_map>
#include <cstddef>
#include <mutex>

class DynamicLibrary;

class PluginManager : public IPluginHost {
public:
    PluginManager() = default;
    ~PluginManager() { unloadAll();}

    std::string loadPlugin(const std::string& path);
    void unloadPlugin(const std::string& name);
    void unloadAll();

    std::string registerService(IPlugin* pPlugin, const std::string& name, const service_t& service) override;
    void unregisterService(IPlugin* pPlugin, const std::string& serviceId) override;
    service_t queryService(const std::string& name) override;
    std::vector<std::string> loadedPlugins() const;

private:
    struct PluginRecord {
        std::string path;
        DynamicLibrary* lib = nullptr;
        IPlugin* plugin = nullptr;
        destroy_plugin_fn destroy = nullptr;
    };

    struct ServiceEntry {
        std::string name;
        service_t service;
    };

    mutable std::mutex mu;
    std::unordered_map<std::string, PluginRecord> records;
    std::unordered_map<std::string, ServiceEntry> services;
    std::unordered_map<std::string, std::vector<std::string>> nameIndex;
};