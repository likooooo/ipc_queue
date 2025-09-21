#pragma once
#include "ipc_heapframe.h"
using stack_pack = ipc_stackframe;
struct heap_pack
{
    stack_pack stack_frame;
    ipc_heapframe  heap_frame;
};
struct vector_object_pack
{
    uint32_t size;
    uint32_t bytes;
    char data[0];
};