#pragma once
#include "ipc_stackframe_iterator.h"
#include "ipc_heapframe.h"
#include <cstring>

bool push_to_heap(ipc_queue& ipc, void* pbuffer, size_t size)
{
    ipc_heapframe* pheap_handle = std::get<1>(ipc_heapframe::malloc(ipc, size));
    if(nullptr == pheap_handle) return false;
    std::memcpy(pheap_handle->v.p, pbuffer, size);
    return true;
}
bool push_to_stack(ipc_queue& ipc, void* pbuffer, size_t size)
{
    size_t stack_count = size / sizeof(ipc_stackframe);
    if(size != stack_count * sizeof(ipc_stackframe)){
        //== TODO : check
        return false;
    }
}