#pragma once
#include "ipc_stackframe_iterator.h"
#include "stack_operation_layout.h"
#include "ipc_heapframe.h"
#include <cstring>

uint32_t push_heap(ipc_queue& ipc, const heap_pack* pbuffer)
{
    if(nullptr == pbuffer) return -1;
    if(!std::holds_alternative<ipc_heap_metadata>(pbuffer->stack_frame.v)) return 1;
    const ipc_heap_metadata& meta = pbuffer->stack_frame.get<ipc_heap_metadata>();
    //== TODO : check plugin id here
    if(0 != meta.ipc_plugin_id) return 2; 
    auto [pstack_handle, pheap_handle] = ipc_heapframe::malloc(ipc, meta.heap_size);
    if(nullptr == pheap_handle) return 3;
    std::memcpy(pheap_handle->v.p, pbuffer, meta.heap_size);
    pstack_handle->get<ipc_heap_metadata>().ipc_plugin_id = meta.ipc_plugin_id;
    pstack_handle->name = pbuffer->stack_frame.name;
    return 0;
}
uint32_t push_stack(ipc_queue& ipc, const stack_pack* pbuffer)
{
    if(nullptr == pbuffer) return -1;
    if(std::holds_alternative<ipc_heap_metadata>(pbuffer->v)) return 1;
    ipc_stackframe::push(ipc) = *pbuffer;
    return 0;
}
uint32_t push(ipc_queue& ipc, const vector_object_pack* pVec)
{
    if(nullptr == pVec) return -1;
    uint32_t pushed_count = 0;
    const char* p = pVec->data; 
    for(uint32_t i = 0; i < pVec->size; i++, pushed_count++){
        if(0 == push_stack(ipc, reinterpret_cast<const stack_pack*>(p))){
            p += sizeof(stack_pack);
        }
        else if(0 == push_heap(ipc, reinterpret_cast<const heap_pack*>(p))){
            p += sizeof(heap_pack) + ipc_stackframe::stack_pointer(ipc).get<ipc_heap_metadata>().heap_size;
        }
        else{
            std::cerr << "push failed at SP-" << pushed_count << std::endl;
            return 1;
        }
    }
    if((p - pVec->data) != pVec->bytes){
        std::cerr << "ignore data after SP-" << pushed_count << std::endl;
        return 2;
    }
    return 0;
}