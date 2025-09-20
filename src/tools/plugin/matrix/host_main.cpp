#include "plugin_manager/plugin_manager.h"
#include "plugin_matrix_layout.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main() {
#if defined(CHECK_RTTI)
    std::cout << "[host] typeid(std::string) address = "
        << static_cast<const void*>(&typeid(std::string)) << "\n";
#endif
    plugin_manager mgr;
    try {
#if defined(_WIN32)
        std::string pluginPath = fs::current_path() / "matrix.dll";
#else
        std::string pluginPath = fs::current_path() / "libmatrix.so";
#endif
        std::string name = mgr.load_plugin(pluginPath);
        //== 思考题, 为什么这里需要用大括号括起来??
        //   plugin_shared.hpp 打开宏定义 CHECK_RTTI 你就知道为什么了 :)
        {
            auto anyv = mgr.query_service("greeting");
            std::cout << "Service greeting = " << std::any_cast<std::string>(anyv) << "\n";
        }
        {
            auto send_to = mgr.query_service("send_to");
            auto f = std::any_cast<send_matrix_to_ipc>(send_to);
            // std::array<char, sizeof(matrix) + 5> buf;
            // matrix* m = new (buf.data()) matrix();
            // m->shape[0] = 5;
            // for(int i = 0; i < m->shape[0]; i++) m->data[i] = i * i;
            
            // std::array<char, sizeof(ipc_heap) + sizeof(matrix) + 5> buf1;
            // ipc_heap* h = new (buf1.data()) ipc_heap();
            // h->
            // f()
        }
        mgr.unload_plugin(name);
        std::cout << "Plugin unloaded\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}