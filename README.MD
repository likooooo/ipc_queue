## Simulation Toykit 使用指南
```
    Simulation Toykit 是 IPC 框架, 用于多进程之间通信。 它将任意进程抽象成一个函数调用(function)。 
任意 function 在调用其他 function 时, 参数通过栈传递(IPC Queue)。function 的栈清理按照调用约定分为
stdcall 和 cdecl, 前者是执行者释放，后者是调用者释放。
```
### IPC Queue 内存模型
``` text
|total shared memory size|
|spin lock               |
----------------------------------------------------
|ipc-heap(bp)            |  <--------|
|ipc-heap(sp)            |  <------| |
|ipc-heap(end)           |         | |
|............            |         | |
|............            |         | |
|ipc-stack(end)          |         | |
|ipc-stack(sp)(heap)     |---------| |
|ipc-stack(heap)         |-----------|
|ipc-stack(bp)           |
|ipc-stack count         |

小于等于 16 字节的数据应该直接放在 stack 上面, 否则放在 heap 上。
```
### 调用约定
TODO
