#include "ipc_queue.h"
#include "ipc_stackframe.h"
#include "ipc_stackframe_iterator.h"
#include <iostream>
#include <cxxabi.h>

template<class T> inline std::string type_reflection() 
{
    const char* mangled_name = typeid(T).name();
    int status = -1;
    char* realname = abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);
    if (status == 0 && realname != nullptr) {
        std::string result(realname);
        std::free(realname);
        return result;
    } else {
        return mangled_name;
    }
}

void print_stack_info()
{
    auto ipc = ipc_queue::open(get_shared_file_path());
    auto loop_adapter = ipc_stackframe_loop_adapter(ipc);
    std::cout << "# stack size : " << ipc_stackframe::stackframe_count(ipc)<< std::endl;
    for(const ipc_stackframe& frame : loop_adapter){
        std::cout << "---------------------------------------\n";
        std::cout << "|-  name  : " << frame.name.data() << std::endl;
        std::visit([](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr(std::is_same_v<any_object_smaller_than_16_byes, T>){
                std::cout << "|-  type  : small stack object" << std::endl;
                std::cout << "|-  value : ";
                std::copy(arg.begin(), arg.end(), std::ostream_iterator<char>(std::cout, " "));
                std::cout << std::endl;
            }
            else if constexpr(std::is_same_v<ipc_heap_metadata, T>){
                std::cout << "|-  type  : heap-buffer" << std::endl;
                std::cout << "|-  base  : " << arg.base_offset<< std::endl;
                std::cout << "|-  size  : " << arg.heap_size << std::endl;
                std::cout << "|-  typeid: " << arg.ipc_plugin_id << std::endl;
            }
            else{
                std::cout << "|-  type  : " << type_reflection<T>() << std::endl;
                std::cout << "|-  value : " << arg << std::endl;
            }
        }, frame.v);
    }
    std::cout << "---------------------------------------\n";
}