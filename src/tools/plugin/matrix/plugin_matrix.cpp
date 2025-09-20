#include "ipc_heap.h"
#include "plugin_matrix_layout.h"
#include "plugin_manager/plugin_shared.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <complex>
#include <cstring>

uint32_t sizeof_pixel(uint32_t pixel_type)
{
    switch (pixel_type)
    {
    case 'f':
        return sizeof(float);
    case 'd':
        return sizeof(double);
    case 'c':
        return sizeof(std::complex<float>);
    case 'z':
        return sizeof(std::complex<double>);
    default:
        std::cerr << "undefined matrix pixel type " << pixel_type << std::endl;
        return pixel_type;
    }
}
bool send_to(ipc_heap& heap, const matrix& m)
{
    // heap.user_type == matrix_type
    char* p = heap.p;
    reinterpret_cast<matrix*>(p)[0] = m;
    uint32_t bytes = sizeof_pixel(m.pixel_type);
    for(size_t i = 0; i < m.shape.size(); i++){
        bytes *= m.shape.at(i);
    }
    heap.size = bytes;
    std::memcpy(p + sizeof(m), m.data, bytes);
    return true;
}
bool receive_from(const ipc_heap& heap, matrix& m)
{
    const char* p = heap.p;
    m = reinterpret_cast<const matrix*>(p)[0];
    uint32_t bytes = sizeof_pixel(m.pixel_type);
    for(size_t i = 0; i < m.shape.size(); i++){
        bytes *= m.shape.at(i);
    }
    std::memcpy(m.data, p + sizeof(m), bytes);
    // TODO : check buffer
    return true;
}

class plugin_matrix : public IPlugin 
{
public:
    const char* name() const override { return "plugin_matrix"; }
    bool onLoad(IPluginHost* host) override {
#if defined(CHECK_RTTI)
        std::cout << "[plugin] typeid(std::string) address = "
            << static_cast<const void*>(&typeid(std::string)) << "\n";
#endif
        host_ = host;
        service_t s = std::string("hello from plugin");
        serviceId_.push_back(host_->register_service(this, "greeting", s));
        serviceId_.push_back(host_->register_service(this, "send_to", (send_matrix_to_ipc)send_to));
        serviceId_.push_back(host_->register_service(this, "receive_from", (load_matrix_from_ipc)receive_from));
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

extern "C" IPlugin* create_plugin() { return new plugin_matrix(); }
extern "C" void destroy_plugin(IPlugin* p) { delete p; }
extern "C" int plugin_api_version() { return PLUGIN_API_VERSION; }