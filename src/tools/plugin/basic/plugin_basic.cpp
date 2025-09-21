#include "ipc_heap.h"
#include "plugin_manager/plugin_shared.hpp"
#include <vector>

void print_stack_info();
class plugin_basic : public IPlugin 
{
public:
    const char* name() const override { return "plugin_basic"; }
    bool onLoad(IPluginHost* host) override 
    {
        IPlugin::host_ = host;
        serviceId_.push_back(host->register_service(this, "print_stack_info", print_stack_info));
        return true;
    }
    void onUnload() override 
    {
        for(const auto& service_name:serviceId_){
            IPlugin::host_->unregister_service(this, service_name);
        }
    }
private:
    std::vector<std::string> serviceId_;
};

extern "C" IPlugin* create_plugin() { return new plugin_basic(); }
extern "C" void destroy_plugin(IPlugin* p) { delete p; }
extern "C" int plugin_api_version() { return PLUGIN_API_VERSION; }