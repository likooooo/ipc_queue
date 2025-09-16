#include "ipc_queue.h"
#include <array>

int main()
{
    auto ipc = ipc_queue::create(get_shared_file_path(), 1024);
    std::array<char, sizeof("Hello from writer")> str{"Hello from writer"};
    ipc.data<decltype(str)>()[0] = str; 
}