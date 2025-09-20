#pragma once
#include "plugin_shared.hpp"
#include <vector>
#include <unordered_map>
#include <cstddef>
#include <mutex>

class dynamic_library;
using plugin_service_key = uint32_t;

class plugin_manager : public IPluginHost {
public:
    plugin_manager() = default;
    ~plugin_manager() { unload_all();}

    std::string load_plugin(const std::string& path);
    void unload_plugin(const std::string& name);
    void unload_all();

    std::string register_service(IPlugin* pPlugin, const std::string& name, const service_t& service) override;
    void unregister_service(IPlugin* pPlugin, const std::string& serviceId) override;
    service_t query_service(const std::string& name) override;
    std::vector<std::string> loaded_plugins() const;

private:
    struct plugin_record {
        std::string path;
        dynamic_library* lib = nullptr;
        IPlugin* plugin = nullptr;
        destroy_plugin_fn destroy = nullptr;
    };

    struct service_entry {
        std::string name;
        service_t service;
    };

    mutable std::mutex mu;
    std::unordered_map<std::string, plugin_record> records;
    std::unordered_map<std::string, service_entry> services;
    std::unordered_map<std::string, std::vector<std::string>> nameIndex;
};