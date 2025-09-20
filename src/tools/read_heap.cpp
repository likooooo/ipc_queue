#include "ipc_stackframe_iterator.h"
#include "ipc_stackframe.h"
#include "ipc_heapframe.h"
#include "ipc_queue.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "ipc_spin_lock.hpp"
int main()
{
    auto ipc = ipc_queue::open(get_shared_file_path());
    auto loop_adapter = ipc_stackframe_loop_adapter(ipc);
    uint32_t index = 0;
    for(const ipc_stackframe& frame : loop_adapter){
        if(!std::holds_alternative<ipc_heap_metadata>(frame.v)) continue;
        std::cout <<"stack(" << index << ")" << std::endl;
        ipc_heap_metadata meta = std::get<ipc_heap_metadata>(frame.v);
        std::cout << "    heap offs=" << meta.base_offset << std::endl;
        std::cout << "    heap size=" << meta.heap_size << std::endl;
        const ipc_heapframe& heap = ipc_heapframe::jump_to_heap(ipc, frame);
        std::cout <<"    ";
        for(size_t i = 0; i < heap.v.size; i++){
            std::cout << uint32_t(heap.v.p[i]) << " ";
        }
        std::cout << std::endl;
        index++;
    }
    // ipc_stackframe::clear_all(ipc);
}