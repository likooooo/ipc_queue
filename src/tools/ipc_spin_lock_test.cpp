#include "ipc_queue.h"
#include "ipc_stackframe.h"
#include "ipc_heapframe.h"
#include <iostream>
#include <thread>

constexpr uint32_t stack_count_per_job = 3;
using heap_type = std::array<uint8_t, 8>;
ipc_queue ipc;
std::mutex m;

void test(size_t offset)
{
    std::lock_guard<ipc_spin_lock> lg(ipc.spin_lock());
    // std::lock_guard<std::mutex> lg(m);
    heap_type buf;
    for(size_t id = offset; id < offset + stack_count_per_job; id++)
    {
        for(size_t i = 0; i < buf.size(); i++){
            buf.at(i) = i * id;
        }
        auto [ps, ph] = ipc_heapframe::malloc(ipc, sizeof(heap_type));

        std::string name = std::to_string(id);
        assert(name.size() <= ps->name.size());
        std::copy(name.begin(), name.begin() + std::min(name.size(), ps->name.size() - 1), ps->name.data());
        reinterpret_cast<heap_type*>(ph->v.p)[0] = buf;
    }
}
int main()
{
    constexpr size_t job_count = 32;
    ipc = ipc_queue::create(get_shared_file_path(), 
        stack_count_per_job * job_count * (sizeof(ipc_heapframe) + sizeof(heap_type) + std::array<size_t, 2>{0, sizeof(ipc_heapframe::canary)}.at(ipc_heapframe::enable_canary) + sizeof(ipc_stackframe)) + sizeof(ipc_stackframe::stack_inner_meta)
    );
    ipc_stackframe::clear_all(ipc);
    std::array<std::thread, job_count> jobs;
    for(size_t i = 0; i < jobs.size(); i++){
        jobs.at(i) = std::thread(test, i);
        // jobs.at(i).join();
    }
    std::for_each(jobs.begin(), jobs.end(),[](std::thread& job){if(job.joinable())job.join();});

    //== checkdata
    auto safe_cast = [](const char* str1){
        try {
            return std::stoi(str1);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error: Invalid argument for std::stoi. " << e.what() << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: The number is too large or too small. " << e.what() << std::endl;
        }
        printf("ptr=%p, %c\n", str1, str1[0]);
        return -1;
    };
    bool check_failed = false;
    auto loop_adapter = ipc_stackframe_loop_adapter(ipc);
    for (auto it = loop_adapter.rbegin(); it != loop_adapter.rend();) {
        int name = safe_cast(it->name.data());
        it++; 
        for(uint32_t i = 1; i < stack_count_per_job; i++, it++){
            int temp = safe_cast(it->name.data());
            if(1 != (name - temp)){
                check_failed = true;
                break;
            }
            name =temp;
        }
    }
    if(check_failed){
        for (auto it = loop_adapter.rbegin(); it != loop_adapter.rend();) {
            for(uint32_t i = 0; i < stack_count_per_job; i++, it++){
                std::cout << it->name.data() << "(" << std::to_string(ipc_heapframe::jump_to_heap(ipc, *it).v.p[0])<< ") ";
            }
            std::cout << std::endl;
        }
        return 1;
    }
    else{
        std::cout <<"Test success ipc spin lock"<< std::endl;
    }
    return 0;
}