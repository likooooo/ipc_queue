#include "plugin_manager.h"
#include "dynamic_libary.hpp"
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <mutex>
#include <any>

std::string PluginManager::loadPlugin(const std::string& path) {
    std::lock_guard<std::mutex> lk(mu);

    auto plib = new DynamicLibrary ;
    auto& lib = *plib;
    lib.open(path);

    auto ver_sym = reinterpret_cast<plugin_api_version_fn>(lib.symbol("plugin_api_version"));
    if (!ver_sym) throw std::runtime_error("plugin missing plugin_api_version");
    if (ver_sym() != PLUGIN_API_VERSION) throw std::runtime_error("plugin api version mismatch");

    auto create_sym = reinterpret_cast<create_plugin_fn>(lib.symbol("create_plugin"));
    auto destroy_sym = reinterpret_cast<destroy_plugin_fn>(lib.symbol("destroy_plugin"));
    if (!create_sym || !destroy_sym) throw std::runtime_error("plugin missing create/destroy");

    IPlugin* plugin = create_sym();
    if (!plugin) throw std::runtime_error("create_plugin returned null");
    if (!plugin->onLoad(this)) {
        destroy_sym(plugin);
        throw std::runtime_error("plugin onLoad failed");
    }

    PluginRecord rec;
    rec.path = path;
    rec.lib = plib;
    rec.plugin = plugin;
    rec.destroy = destroy_sym;

    std::string key = plugin->name();
    if (key.empty()) key = path;
    if (records.count(key)) {
        plugin->onUnload();
        destroy_sym(plugin);
        throw std::runtime_error("plugin already loaded: " + key);
    }

    records.emplace(key, rec);
    std::cout << rec.plugin->name()<< " loaded\n";
    return key;
}

void PluginManager::unloadPlugin(const std::string& name) {
    std::lock_guard<std::mutex> lk(mu);
    auto it = records.find(name);
    if (it == records.end()) return;

    PluginRecord& rec = it->second;
    rec.plugin->onUnload();
    rec.destroy(rec.plugin);
    rec.lib->close();
    delete rec.lib;
    records.erase(it);
    std::cout << name << " unloaded\n";
}

void PluginManager::unloadAll() {
    std::vector<std::string> names;
    {
        std::lock_guard<std::mutex> lk(mu);
        for (auto &p : records) names.push_back(p.first);
    }
    for (auto &n : names) unloadPlugin(n);
}

std::string PluginManager::registerService(IPlugin* pPlugin, const std::string& name, const service_t& service) {
    std::string id = std::string(pPlugin->name()) + "#" + name;
    services.emplace(id, ServiceEntry{name, service});
    nameIndex[name].push_back(id);
    std::cout <<"*    " << pPlugin->name()<< " registered service : " << id << "\n";
    return id;
}

void PluginManager::unregisterService(IPlugin* pPlugin, const std::string& serviceId) {
    auto it = services.find(serviceId);
    if (it == services.end()) return;
    auto name = it->second.name;
    services.erase(it);
    auto &vec = nameIndex[name];
    vec.erase(std::remove_if(vec.begin(), vec.end(), [&](const auto& str){return str == serviceId;}), vec.end());
    if (vec.empty()) nameIndex.erase(name);
    std::cout <<"*    " << pPlugin->name()<< " unregistered service : " << serviceId << "\n";
}

service_t PluginManager::queryService(const std::string& name) {
    std::lock_guard<std::mutex> lk(mu);
    auto it = nameIndex.find(name);
    if (it == nameIndex.end() || it->second.empty()) return service_t();
    return services[it->second.front()].service;
}

std::vector<std::string> PluginManager::loadedPlugins() const {
    std::lock_guard<std::mutex> lk(mu);
    std::vector<std::string> out;
    for (auto &p : records) out.push_back(p.first);
    return out;
}
