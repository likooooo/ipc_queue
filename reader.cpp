#include "ipc_queue.h"
#include <array>

int main()
{
    auto ipc = ipc_queue::open(get_shared_file_path());
    std::array<char, sizeof("Hello from writer")> str{"Hello from writer"};
    printf("%s", ipc.data<decltype(str)>()->data()); 
}