#include "ipc_queue.h"
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <filesystem>
#include <iostream>
#include <cstdlib> 
#include <fstream>
#include <cstring>
#include <string>
#include <array>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace boost::interprocess;
namespace fs = std::filesystem;

inline std::filebuf create_file_with_size(const std::string& filename, uint32_t bytes)
{
    std::filebuf fbuf;
    fbuf.open(filename, std::ios_base::in | std::ios_base::out
                            | std::ios_base::trunc | std::ios_base::binary);
    fbuf.pubseekoff(bytes - 1, std::ios_base::beg);
    fbuf.sputc(0);
    return fbuf;
}
inline mapped_region mapping_file_to_shm(const std::string& filename, boost::interprocess::mode_t mode = read_write)
{
    file_mapping m_file(filename.c_str(), mode);
    mapped_region region = mapped_region(m_file, mode);
    return region;
}
inline mapped_region create_shm_with_file(const std::string& filename, uint32_t bytes)
{
    create_file_with_size(filename, bytes).close();
    return mapping_file_to_shm(filename, read_write);
}

struct ipc_queue_impl
{
    mapped_region region;
    mutable ipc_spin_lock spin_lock;
};

union inner_header
{
    uint32_t size;
    std::array<char, 16> data;
};
static_assert(sizeof(ipc_spin_lock) <= (sizeof(inner_header::data) - sizeof(inner_header::size)));

ipc_queue::ipc_queue() : pImpl(new ipc_queue_impl){}
ipc_queue::ipc_queue(ipc_queue&& other) 
{
    this->pImpl = other.pImpl;
    other.pImpl = nullptr;
}
ipc_queue& ipc_queue::operator=(ipc_queue&& other)
{
    this->pImpl = other.pImpl;
    other.pImpl = nullptr;
    return *this;
}
ipc_queue::~ipc_queue(){delete pImpl;}
ipc_queue ipc_queue::create(const std::string& filename, uint32_t bytes)
{
    ipc_queue ipc;
    ipc_queue_impl& ref = *ipc.pImpl; 
    ref.region = create_shm_with_file(filename, sizeof(inner_header) + bytes);
    reinterpret_cast<inner_header*>(ref.region.get_address())->size = bytes;
    
    void* p = &(reinterpret_cast<ipc_spin_lock*>(ipc.data())[-1]);
    ref.spin_lock = ipc_spin_lock(p);
    ref.spin_lock.unlock();
    return ipc;
}
ipc_queue ipc_queue::create_if_not_exists(const std::string& filename, uint32_t bytes)
{
    bool create = false;
    uint32_t filesize = 0;
    if(!fs::exists(filename)){
        create = true;
        std::cout << "create new IPC Queue. max bytes = " << bytes <<std::endl;
    }
    else if((filesize = fs::file_size(filename)) != bytes + sizeof(inner_header)){
        //== TODO : maybe need copydata here??
        create = true;
        std::cout << "realloc IPC Queue. from  " << filesize << "(bytes) to " << (bytes + sizeof(inner_header)) << "(bytes)" <<std::endl;
    }
    return create ? ipc_queue::create(filename, bytes) : ipc_queue::open(filename);
}
ipc_queue ipc_queue::open(const std::string& filename)
{
    ipc_queue ipc;
    ipc_queue_impl& ref = *ipc.pImpl; 
    ref.region = mapping_file_to_shm(filename);
    void* p = &(reinterpret_cast<ipc_spin_lock*>(ipc.data())[-1]);
    ref.spin_lock = ipc_spin_lock(p);
    return ipc;
}

uint32_t ipc_queue::size() const
{
    return reinterpret_cast<const inner_header*>(data())[-1].size;
}
char* ipc_queue::data()
{
    ipc_queue_impl& ref = *pImpl; 
    char* p = reinterpret_cast<char*>(ref.region.get_address());
    return p + sizeof(inner_header);
}
const char* ipc_queue::data() const
{
    const ipc_queue_impl& ref = *pImpl; 
    const char* p = reinterpret_cast<const char*>(ref.region.get_address());
    return p + sizeof(inner_header);
}
ipc_spin_lock& ipc_queue::spin_lock() const
{
    return this->pImpl->spin_lock;
}
std::string get_shared_file_path(const std::string& name)
{
    // 1. 优先使用用户自定义环境变量
    if (const char* env = std::getenv("IPC_QUEUE_WORK_DIR")) {
        fs::path p(env);
        p /= name;
        return p.string();
    }

#ifdef _WIN32
    // 在 Windows 上，使用 GetTempPathA 获取临时目录
    char tmpPath[MAX_PATH];
    DWORD len = GetTempPathA(MAX_PATH, tmpPath);
    if (len > 0 && len < MAX_PATH) {
        fs::path p(tmpPath);
        p /= name;
        return p.string();
    } else {
        // Fallback to a default path if GetTempPathA fails
        fs::path p("C:\\Temp");
        p /= name;
        return p.string();
    }
#else
    // 在非 Windows 系统上，通常使用 /tmp
    fs::path p("/tmp");
    p /= name;
    return p.string();
#endif
}
std::string get_shared_file_path() 
{
    return get_shared_file_path("ipc_queue.bin");
}
