#include "ipc_heap.h"
#include "plugin_manager/plugin_shared.hpp"
#include <vector>

#if ENABLE_STACK_INFO
    void print_stack_info();
#endif
class plugin_basic : public IPlugin 
{
public:
    const char* name() const override { return "plugin_basic"; }
    bool onLoad(IPluginHost* host) override 
    {
        host_ = host;
#if ENABLE_STACK_INFO
        serviceId_.push_back(host->register_service(this, "print_stack_info", print_stack_info));
#endif
        return true;
    }
    void onUnload() override 
    {
        for(const auto& service_name:serviceId_){
            host_->unregister_service(this, service_name);
        }
    }
private:
    IPluginHost* host_ = nullptr;
    std::vector<std::string> serviceId_;
};

extern "C" IPlugin* create_plugin() { return new plugin_basic(); }
extern "C" void destroy_plugin(IPlugin* p) { delete p; }
extern "C" int plugin_api_version() { return PLUGIN_API_VERSION; }