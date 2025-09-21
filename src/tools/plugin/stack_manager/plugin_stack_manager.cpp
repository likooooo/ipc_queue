#include "ipc_queue.h"
#include "ipc_stackframe.h"
#include "ipc_stackframe_iterator.h"
#include "plugin_manager/plugin_shared.hpp"
#include <iostream>
#include <cxxabi.h>
#include <vector>

class stack_manager : public IPlugin 
{
public:
    const char* name() const override { return "stack_manager"; }
    bool onLoad(IPluginHost* host) override 
    {
        IPlugin::host_ = host;
#if ENABLE_STACK_INFO
        serviceId_.push_back(host->register_service(this, "print_stack_info", print_stack_info));
#endif
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

extern "C" IPlugin* create_plugin() { return new stack_manager(); }
extern "C" void destroy_plugin(IPlugin* p) { delete p; }
extern "C" int plugin_api_version() { return PLUGIN_API_VERSION; }