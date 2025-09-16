#include <string>
#include <cstdint>
#include <memory>

struct ipc_queue_impl;
struct ipc_queue
{
    ipc_queue();
    ~ipc_queue();
    template<class T = char> T* data(){return reinterpret_cast<T*>(p);};
    template<class T = char> const T* data()const {return reinterpret_cast<const T*>(p);};
    static ipc_queue create(const std::string& filename, uint32_t bytes);
    static ipc_queue open(const std::string& filename);
    char* p;
    uint32_t size;
    ipc_queue_impl* pImpl;
};
std::string get_shared_file_path();