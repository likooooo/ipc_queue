#pragma once
#include <string>
#include <cstdint>
#include <memory>
#include "ipc_spin_lock.hpp"

struct ipc_queue_impl;
struct ipc_queue
{
    ipc_queue_impl* pImpl;
    ipc_queue();
    ipc_queue(ipc_queue&&);
    ipc_queue& operator=(ipc_queue&&);
    ~ipc_queue();
    
    ipc_queue(const ipc_queue&) = delete;
    ipc_queue& operator=(const ipc_queue&) = delete;
    ipc_spin_lock& spin_lock() const;

    static ipc_queue create(const std::string& filename, uint32_t bytes);
    static ipc_queue create_if_not_exists(const std::string& filename, uint32_t bytes);
    static ipc_queue open(const std::string& filename);

    // {return data<uint32_t>()[-1];}
    uint32_t size() const;
    char* data();
    const char* data() const;
};

std::string get_shared_file_path();
std::string get_shared_file_path(const std::string&); 