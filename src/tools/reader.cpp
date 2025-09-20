#include "ipc_queue.h"
#include "ipc_stackframe.h"
#include "ipc_stackframe_iterator.h"
#include <iostream>

int main()
{
    auto ipc = ipc_queue::open(get_shared_file_path());
    // read
    auto loop_adapter = ipc_stackframe_loop_adapter(ipc);
    std::cout << std::endl;
    for(const ipc_stackframe& frame : loop_adapter){
        std::cout << "loop bp->sp:" << std::get<int16_t>(frame.v) << std::endl;
    }
    for (auto it = loop_adapter.rbegin(); it != loop_adapter.rend(); ++it) {
        std::cout << "loop sp->bp:" << std::get<int16_t>(it->v) << std::endl;
    }
    // pop
    int n = ipc_stackframe::stackframe_count(ipc);
    std::cout << "\nqueue size : " << n << std::endl;
    for(int i = 0; i < n; i++){
        std::cout << "pop   data : " << std::get<int16_t>(ipc_stackframe::pop(ipc).v) << std::endl;
    }
    std::cout << "queue size : " << ipc_stackframe::stackframe_count(ipc)<< std::endl;
}