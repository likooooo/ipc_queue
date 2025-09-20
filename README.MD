|total shared memory size|
|spin lock(TODO)         |
----------------------------------------------------
|uint32 mem-usage   |
|ipc-heap(bp)       |  <--------|
|ipc-heap(sp)       |  <------| |
|ipc-heap(end)      |         | |
|............       |         | |
|............       |         | |
|ipc-stack(end)     |         | |
|ipc-stack(sp)(heap)|---------| |
|ipc-stack(heap)    |-----------|
|ipc-stack(bp)      |
|ipc-stack count    |

不支持多线程模型