#include "ipc_queue.h"
#include <string>
#include <cstdlib> 
#include <fstream>
#include <iostream>
#include <cstring>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace boost::interprocess;


inline std::filebuf create_file_with_size(const std::string& filename, uint32_t bytes)
{
    std::filebuf fbuf;
    fbuf.open(filename, std::ios_base::in | std::ios_base::out
                            | std::ios_base::trunc | std::ios_base::binary);
    fbuf.pubseekoff(bytes - 1, std::ios_base::beg);
    fbuf.sputc(0);
    return fbuf;
}
inline mapped_region mapping_file_to_shm(const std::string& filename, boost::interprocess::mode_t mode = read_only)
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
};

ipc_queue::ipc_queue() : p(nullptr), pImpl(new ipc_queue_impl){}
ipc_queue::~ipc_queue(){delete pImpl;}
ipc_queue ipc_queue::create(const std::string& filename, uint32_t bytes)
{
    ipc_queue ipc;
    ipc_queue_impl& ref = *ipc.pImpl; 
    ref.region = create_shm_with_file(filename, sizeof(uint32_t) + bytes);
    ipc.p = reinterpret_cast<char*>(ref.region.get_address());
    reinterpret_cast<uint32_t*>(ipc.p)[0] = bytes;
    ipc.p += sizeof(uint32_t);
    ipc.size = bytes;
    return ipc;
}
ipc_queue ipc_queue::open(const std::string& filename)
{
    ipc_queue ipc;
    ipc_queue_impl& ref = *ipc.pImpl; 
    ref.region = mapping_file_to_shm(filename);
    ipc.p = reinterpret_cast<char*>(ref.region.get_address());
    ipc.size = reinterpret_cast<uint32_t*>(ipc.p)[0];
    ipc.p += sizeof(uint32_t);
    return ipc;
}


std::string get_shared_file_path() 
{
    // 1. 用户自定义环境变量优先
    if (const char* env = std::getenv("MY_SHM_DIR")) {
        return std::string(env) + "/ipc_queue.bin";
    }

#ifdef _WIN32
    char tmpPath[MAX_PATH];
    DWORD len = GetTempPathA(MAX_PATH, tmpPath);
    if (len > 0 && len < MAX_PATH) {
        return std::string(tmpPath) + "ipc_queue.bin";
    } else {
        return "C:\\\\Temp\\\\ipc_queue.bin";
    }
#else
    return "/tmp/ipc_queue.bin";
#endif
}
