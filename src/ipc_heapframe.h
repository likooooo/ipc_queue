#pragma once
#include "ipc_stackframe_iterator.h"
#include <iostream>
#include <algorithm>
#include <tuple>

struct ipc_heapframe
{
    constexpr static bool enable_canary = true;
    constexpr static uint32_t canary = 0xcacacaca;
    ipc_heap v;
    char* get()
    {
        return v.p;
    }
    const char* get()const
    {
        return v.p;
    }
    static uint32_t& get_canary(ipc_heapframe& heap)
    {
        char* p = reinterpret_cast<char*>(&heap) + sizeof(ipc_heap) + heap.v.size;
        return reinterpret_cast<uint32_t*>(p)[0];
    }
    static const uint32_t& get_canary(const ipc_heapframe& heap)
    {
        const char* p = reinterpret_cast<const char*>(&heap) + sizeof(ipc_heap) + heap.v.size;
        return reinterpret_cast<const uint32_t*>(p)[0];
    }
    static bool is_canary_alive(const ipc_heapframe& heap)
    {
        bool is_alive = get_canary(heap) == canary;
        
        // ipc_heap_metadata meta = std::get<ipc_heap_metadata>(sp->v);
        // std::cerr << "The previous heapframe is damaged."
        //     "\n    plugin_id=" << meta.ipc_plugin_id << 
        //     "\n    size     =" << from_heap_index(queue, meta).size <<
        //     "\n    found    =" << found << std::endl;

        return is_alive;
    }
    static ipc_heapframe& move_heap_next(ipc_heapframe& heap)
    {
        char* p = reinterpret_cast<char*>(&heap) + sizeof(ipc_heap) + heap.v.size;
        if (enable_canary){
            if(!is_canary_alive(heap)){
                get_canary(heap) = canary;
            }
        }
        if constexpr(enable_canary){
            p += sizeof(canary);
        }
        return *reinterpret_cast<ipc_heapframe*>(p);
    }

    //== Note: The time complexity is O(n)
    static std::tuple<ipc_stackframe*, bool> stack_pointer_impl(ipc_queue& queue)
    {
        ipc_stackframe_loop_adapter loop(queue);
        auto it = std::find_if(loop.rbegin(), loop.rend(), [](ipc_stackframe& frame){
            return std::holds_alternative<ipc_heap_metadata>(frame.v); 
        });
        return std::make_tuple<ipc_stackframe*, bool>(&(*it),  loop.rend() != it);
    }
    static std::tuple<const ipc_stackframe*, bool> stack_pointer_impl(const ipc_queue& queue)
     {
        //== I don't want to write ipc_stackframe_const_loop_adapter
        ipc_stackframe_loop_adapter loop(const_cast<ipc_queue&>(queue));
        auto it = std::find_if(loop.crbegin(), loop.crend(), [](const ipc_stackframe& frame){
            return std::holds_alternative<ipc_heap_metadata>(frame.v); 
        });
        return std::make_tuple<const ipc_stackframe*, bool>(&(*it),  loop.crend() != it);
    }

    static ipc_heapframe& base_pointer(ipc_queue& queue)
    {
        return reinterpret_cast<ipc_heapframe*>(queue.data())[0];
    }
    static const ipc_heapframe& base_pointer(const ipc_queue& queue)
    {
        return reinterpret_cast<const ipc_heapframe*>(queue.data())[0];
    }
    static ipc_heapframe& stack_pointer(ipc_queue& queue)
    {
        auto [pStackframe, found] = stack_pointer_impl(queue);
        return found ? jump_to_heap(queue, *pStackframe) : base_pointer(queue);
    }
    static const ipc_heapframe& stack_pointer(const ipc_queue& queue)
    {
        auto [pStackframe, found] = stack_pointer_impl(queue);
        return found ? jump_to_heap(queue, *pStackframe) : base_pointer(queue);
    }
    static ipc_heapframe& jump_to_heap(ipc_queue& queue, ipc_stackframe& stack)
    {
        char* p = reinterpret_cast<char*>(&base_pointer(queue)) + stack.get<ipc_heap_metadata>().base_offset;
        return *reinterpret_cast<ipc_heapframe*>(p);
    }
    static const ipc_heapframe& jump_to_heap(const ipc_queue& queue, const ipc_stackframe& stack)
    {
        const char* p = reinterpret_cast<const char*>(&base_pointer(queue)) + stack.get<ipc_heap_metadata>().base_offset;
        return *reinterpret_cast<const ipc_heapframe*>(p);
    }
    static bool is_out_of_memory(uint32_t n)
    {
        return false;
    }
    static std::tuple<ipc_stackframe*, ipc_heapframe*> malloc(ipc_queue& queue, uint32_t n)
    {
        // TODO: out-of-memory check
        //==========================
        if(is_out_of_memory(n)){
            std::cerr << "out of memory. bytes=" << n << std::endl;
            return std::tuple<ipc_stackframe*, ipc_heapframe*>(nullptr, nullptr);
        }
        ipc_heapframe* heap_frame = nullptr;
        ipc_stackframe_loop_adapter loop(queue);
        auto it =std::find_if(loop.rbegin(), loop.rend(), [](ipc_stackframe& frame){
            return std::holds_alternative<ipc_heap_metadata>(frame.v);
        });
        if(loop.rend() != it){
            heap_frame = &move_heap_next(jump_to_heap(queue, *it));
        }
        else{
            heap_frame = &ipc_heapframe::base_pointer(queue);
        }
        heap_frame->v.size = n;

        ipc_heap_metadata meta;
        meta.heap_size = sizeof(ipc_heapframe) + n;
        if constexpr (enable_canary){
            get_canary(*heap_frame) = canary;
            meta.heap_size += sizeof(canary);
        }
        meta.base_offset = reinterpret_cast<char*>(heap_frame) - reinterpret_cast<char*>(&base_pointer(queue));

        ipc_stackframe* stack_frame = &ipc_stackframe::push(queue);
        stack_frame->v = meta;
        return std::make_tuple(stack_frame, heap_frame);
    }



    // static ipc_heap& malloc(ipc_queue& queue, uint32_t n)
    // {
    //     ipc_heap& heap_end = end(queue);
    //     if constexpr(enable_canary){
    //         if(heap_memory_used(queue) > 0 && canary != reinterpret_cast<uint32_t*>(&heap_end)[-1]){
    //             auto [sp, found] = stack_pointer_in_stack(queue);
    //             ipc_heap_metadata meta = std::get<ipc_heap_metadata>(sp->v);
    //             std::cerr << "The previous heapframe is damaged."
    //               "\n    plugin_id=" << meta.ipc_plugin_id << 
    //               "\n    size     =" << from_heap_index(queue, meta).size <<
    //               "\n    found    =" << found << std::endl;
    //         }
    //     }
    //     uint32_t& bytes = heap_memory_used(queue);
    //     bytes += (n + sizeof(ipc_heap));
    //     if constexpr(enable_canary){
    //         bytes += sizeof(canary);
    //         reinterpret_cast<uint32_t*>(&end(queue))[-1] = canary;
    //     }
    //     heap_end.size = n;
    //     return heap_end;
    // }
    // static ipc_heap_metadata to_heap_index(ipc_queue& queue, const ipc_heap& current)
    // {
    //     ipc_heap_metadata n;
    //     n.index = &current - &begin(queue);
    //     return n;
    // }

    // static ipc_heap& from_heap_index(ipc_queue& queue, ipc_stackframe& n)
    // {
    //     return from_heap_index(queue, std::get<ipc_heap_metadata>(n.v));
    // }
    // static const ipc_heap& from_heap_index(const ipc_queue& queue, const ipc_stackframe& n)
    // {
    //     return from_heap_index(queue, std::get<ipc_heap_metadata>(n.v));
    // }
    // static ipc_heap& base_pointer(ipc_queue& queue)
    // {
    //     return *reinterpret_cast<ipc_heap*>(queue.data() + sizeof(uint32_t));
    // }
    // static const ipc_heap& base_pointer(const ipc_queue& queue)
    // {
    //     return *reinterpret_cast<const ipc_heap*>(queue.data() + sizeof(uint32_t));
    // }
    
    // //== TODO : heapframe iterator
    // static ipc_heap& begin(ipc_queue& queue)
    // {
    //     return base_pointer(queue);
    // }
    // static const ipc_heap& begin(const ipc_queue& queue)
    // {
    //     return base_pointer(queue);
    // }
    // static ipc_heap& end(ipc_queue& queue)
    // {
    //     char* p = reinterpret_cast<char*>(&base_pointer(queue)) + heap_memory_used(queue);
    //     return *reinterpret_cast<ipc_heap*>(p);
    // }
    // static const ipc_heap& end(const ipc_queue& queue)
    // {
    //     const char* p = reinterpret_cast<const char*>(&base_pointer(queue)) + heap_memory_used(queue);
    //     return *reinterpret_cast<const ipc_heap*>(p);
    // }

    // static ipc_heap& move_heap_next(ipc_heap& current)
    // {
    //     char* p = reinterpret_cast<char*>(&current);
    //     p += (current.size + sizeof(ipc_heap));
    //     if constexpr(enable_canary){
    //         p += sizeof(canary);
    //     }
    //     return *reinterpret_cast<ipc_heap*>(p);
    // }
    // static const ipc_heap& move_heap_next(const ipc_heap& current)
    // {
    //     const char* p = reinterpret_cast<const char*>(&current);
    //     p += current.size + sizeof(ipc_heap);
    //     if constexpr(enable_canary){
    //         p += sizeof(canary);
    //     }
    //     return *reinterpret_cast<const ipc_heap*>(p);
    // }
    // static uint32_t memory_avaliable(const ipc_queue& queue)
    // {
    //     const char* last = reinterpret_cast<const char*>(
    //         ipc_stackframe_loop_adapter(const_cast<ipc_queue&>(queue)).end().base()
    //     );
    //     const char* front = reinterpret_cast<const char*>(&base_pointer(queue)) + heap_memory_used(queue);
    //     return last - front;
    // }
};
