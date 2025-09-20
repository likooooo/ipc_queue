#include "ipc_queue.h"
#include "ipc_stackframe.h"
#include "ipc_heapframe.h"
#include <iostream>
int main()
{
    auto ipc = ipc_queue::create_if_not_exists(get_shared_file_path(), 1024 * 1024);
    using T = std::array<uint8_t, 8>;
    T buf;
    for(size_t id = 1; id < 3; id++)
    {
        for(size_t i = 0; i < buf.size(); i++){
            buf.at(i) = i * id;
        }
        auto [ps, ph] = ipc_heapframe::malloc(ipc, sizeof(T));
        reinterpret_cast<T*>(ph->v.p)[0] = buf;
    }
}