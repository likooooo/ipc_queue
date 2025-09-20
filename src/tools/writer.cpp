#include "ipc_queue.h"
#include "ipc_stackframe.h"
#include "ipc_heapframe.h"
#include <iostream>
int main()
{
    auto ipc = ipc_queue::create(get_shared_file_path(), 1024);
    ipc_stackframe::clear_all(ipc);
    for(int i = 0; i < 5; i++){
        ipc_stackframe& frame_real = ipc_stackframe::push(ipc);
        frame_real.v = int16_t(i);
        std::cout << "push  data : " << frame_real.get<int16_t>() << std::endl; 
    }
    std::cout << "queue size : " << ipc_stackframe::stackframe_count(ipc)<< std::endl; 
}