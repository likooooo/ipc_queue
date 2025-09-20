#pragma once
#include "ipc_heap.h"
#include <array>
#include <cstdint>

struct matrix
{
    std::array<uint32_t, 5> shape{1, 1, 1, 1, 1};
    uint32_t pixel_type{'f'};
    char data[0];
};
using send_matrix_to_ipc = bool (*)(ipc_heap&, const matrix&);
using load_matrix_from_ipc = bool (*)(const ipc_heap&, matrix&);