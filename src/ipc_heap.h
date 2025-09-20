#pragma once
#include <cstdint>

struct ipc_heap
{
    // size of p
    uint32_t size;
    char p[0];
};
struct ipc_heap_metadata
{
    uint32_t base_offset{0};
    uint32_t heap_size{0};
    uint32_t ipc_plugin_id{0};
    char reserved[4]{0};
};
static_assert(16 == sizeof(ipc_heap_metadata));

struct ipc_heap_object
{
    uint32_t user_type;
};