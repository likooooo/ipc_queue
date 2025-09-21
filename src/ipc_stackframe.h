#pragma once
#include "ipc_queue.h"
#include <ipc_heap.h>
#include <type_traits>
#include <assert.h>
#include <complex>
#include <variant>
#include <atomic>
#include <array>

template<class ...T>
struct ipc_built_in_type
{
    using type = std::variant<T...>;
    template<class T1>
    constexpr static bool is_built_in_type()
    {
        bool ret = false;
        ((ret |= std::is_same_v<T1, T>), ...);
        return ret;
    }
};

using any_object_smaller_than_16_byes = std::array<char, 16>;
using string_24_bytes = std::array<char, 24>;
using built_in_helper = ipc_built_in_type<
    int8_t, int16_t, int32_t, int64_t, 
    uint8_t, uint16_t, uint32_t, uint64_t, 
    float, double, std::complex<float>, std::complex<double>,
    any_object_smaller_than_16_byes, ipc_heap_metadata
>;
using built_in_t = typename built_in_helper::type;
static_assert(24 == sizeof(built_in_t));

struct ipc_stackframe
{
    union stack_inner_meta
    {
        std::atomic<uint32_t> stack_count;
        std::array<char, 16> mem{0};
    };
    built_in_t v{0};
    string_24_bytes name{0};

    template<class T> T& get()
    {
        static_assert(built_in_helper::is_built_in_type<T>());
        return std::get<T>(v);
    }
    template<class T> const T& get()const
    {
        static_assert(built_in_helper::is_built_in_type<T>());
        return std::get<T>(v);
    }
    template<class T> T try_get()
    {
        return std::holds_alternative<T>(v) ? get<T>() : T();
    }
    template<class T> static T move_stack_next(T p, uint32_t n = 1){return p - n;}
    template<class T> static T move_stack_prev(T p, uint32_t n = 1){return p + n;}

    static stack_inner_meta* tail_pointer(ipc_queue& ipc)
    {
        return move_stack_next(
            reinterpret_cast<stack_inner_meta*>(ipc.data() + ipc.size())
        );
    }
    static const stack_inner_meta* tail_pointer(const ipc_queue& ipc)
    {
        return move_stack_next(
            reinterpret_cast<const stack_inner_meta*>(ipc.data() + ipc.size())
        );
    }

    static const ipc_stackframe& stack_pointer(const ipc_queue& ipc)
    {
        return *move_stack_next(
            reinterpret_cast<const ipc_stackframe*>(
                tail_pointer(ipc)
            ), stackframe_count(ipc)
        );
    }
    static const ipc_stackframe& base_pointer(const ipc_queue& ipc)
    {
        return *move_stack_next(
            reinterpret_cast<const ipc_stackframe*>(tail_pointer(ipc))
        );
    }
    static ipc_stackframe& stack_pointer(ipc_queue& ipc)
    {
        return *move_stack_next(
            reinterpret_cast<ipc_stackframe*>(
                tail_pointer(ipc)
            ), stackframe_count(ipc)
        );
    }
    static ipc_stackframe& base_pointer(ipc_queue& ipc)
    {
        return *move_stack_next(
            reinterpret_cast<ipc_stackframe*>(tail_pointer(ipc))
        );
    }
    static ipc_stackframe& latest(ipc_queue& ipc, uint32_t offset = 0)
    {
        assert(offset < stackframe_count(ipc));
        return stack_pointer(ipc);
    }
    static ipc_stackframe& push(ipc_queue& ipc)
    {
        //== TODO : add spin lock here
        stackframe_count(ipc)++;
        return latest(ipc) = ipc_stackframe();
    }
    static ipc_stackframe& pop(ipc_queue& ipc)
    {
        //== TODO : add spin lock here
        ipc_stackframe& ref = stack_pointer(ipc);
        stackframe_count(ipc)--;
        return ref;
    }
    static const std::atomic<uint32_t>& stackframe_count(const ipc_queue& ipc)
    {
        return ipc_stackframe::tail_pointer(ipc)->stack_count;
    }
    static std::atomic<uint32_t>& stackframe_count(ipc_queue& ipc)
    {
        return ipc_stackframe::tail_pointer(ipc)->stack_count;
    }
    static ipc_queue& clear_all(ipc_queue& ipc)
    {
        stackframe_count(ipc) = 0;
        return ipc;
    }
};
static_assert(48 == sizeof(ipc_stackframe));