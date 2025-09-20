#include "plugin_manager/plugin_manager.h"
#include "ipc_queue.h"
#include "ipc_stackframe.h"
#include "ipc_heapframe.h"
#include <iostream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

void load_all_plugin(const fs::path& directory, plugin_manager& mgr) 
{
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: The provided path is not a valid directory." << std::endl;
        return;
    }
    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            std::string extension = entry.path().extension().string();
            
            for (char& c : extension) {
                c = std::tolower(c);
            }
            if (extension == ".dll" || extension == ".so") {
                mgr.load_plugin(entry.path().string());
            }
        }
    }
}

template<size_t N> constexpr any_object_smaller_than_16_byes 
pack_str(const char (&t)[N]) 
{
    any_object_smaller_than_16_byes obj{};
    static_assert(N <= sizeof(obj), "String is too large to fit in the object.");
    for (size_t i = 0; i < N; ++i) {
        obj[i] = t[i];
    }
    return obj;
}

int main()
{
    plugin_manager mgr;
    load_all_plugin(fs::absolute("./plugin"), mgr);
    const uint32_t max_memory_size = 1024;
    auto ipc = ipc_queue::create_if_not_exists(get_shared_file_path(), max_memory_size);
    any_object_smaller_than_16_byes key = pack_str("private key");
    if(key != ipc_stackframe::base_pointer(ipc).try_get<any_object_smaller_than_16_byes>()){
        ipc_stackframe::clear_all(ipc);
        ipc_stackframe::push(ipc).v = key;
    }
    
    std::cout << std::endl << std::endl << "simulation-toykit is RUNNING\n";
    std::string service_name = "print_stack_info";
    auto func = mgr.query_service(service_name);
    if(func.has_value()){
        std::any_cast<void(*)()>(func)();
    }
    else{
        std::cout << "service not found " << service_name << std::endl;
    }
    std::cout << "simulation-toykit CLOSED"<< std::endl << std::endl<< std::endl;
    // 1. init ipc queue
    // 2. load all plugin
    // 3. run server
    // 4. other send msg to server, do somethings
}