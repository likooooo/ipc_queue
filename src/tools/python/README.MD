## 参数扫描使用指南

该文档介绍了 `sweep_table` 和 `sweep_row` 的使用方法，他们二者联合起来使用，可以实现非常灵活的参数 sweep。 

### `sweep_table.py` 使用指南

这个脚本用于生成参数组合表，并将每个独特的组合以新行的形式输出到标准输出。这些组合可以由数值范围和离散的标称值共同创建。

#### 主要功能

  * **线性扫描（Linear Sweeps）**：在起始值和结束值之间创建一系列等距的数值。
  * **随机采样（Randomized Sweeps）**：在指定范围内根据不同的概率分布生成随机值。
  * **指定参数（Nominal Parameters）**：在参数扫描中加入一个或多个离散的参数列表，支持非数值形式。

#### 命令行参数

| 参数名 | 类型 |  默认值 | 描述 |
| :--- | :--- |  :--- | :--- |
|`start`|`list of float`|无| 一个浮点数列表，代表一个或多个参数扫描的起始值。|
|`end`|`list of float`|无| 一个浮点数列表，代表一个或多个参数扫描的结束值。|
|`size`|`list of int`|无| 一个整数列表，指定每个扫描的步数。|
|`nominal`|`list`|无| 指定参数列表。|
|`random_sampling`|`string`|均匀网格采样|用于扫描的随机分布名称。支持的分布包括 `uniform`、`gauss`、`normal`、`exponential`、`gamma`、`beta`、`poisson` 和 `binomial`。|
|`seed`|`int`|4396|随机数生成器的种子。|
|`optional-params`|`float`|无|脚本支持特定分布的额外可选参数，例如 `--mu`、`--sigma`、`--rate`、`--shape`、`--scale`、`--a`、`--b`、`--lambda` `--n`和 `--p`。|


#### 示例

1.  **简单的线性扫描**  
    生成一个从 0 到 100，共 11 个步长的线性扫描。

    ```bash
    ./sweep_table.py --start 0 --end 100 --size 11
    ```

2.  **多个线性扫描**  
    生成两个独立的扫描，并组合所有可能性。第一个扫描是从 0 到 1，共 5 个步长；第二个扫描是从 10 到 20，共 3 个步长。

    ```bash
    ./sweep_table.py --start 0 10 --end 1 20 --size 5 3
    ```
    这将生成一个包含 15 行 (5 × 3) 的表格。

3.  **结合线性参数和指定参数**  
    将一个线性扫描与一个指定参数列表结合。

    ```bash
    ./sweep_table.py --start 1 --end 100 --size 10 --nominal '["red", "green", "blue"]'
    ```
    这将生成 30 行 (3 x 10)。

4.  **使用随机采样**  
    使用正态分布在 0 到 100 之间生成 50 个随机数。

    ```bash
    ./sweep_table.py --start 0 --end 100 --size 50 --random_sampling gauss
    ```

-----

### `sweep_row.py` 使用指南

这个脚本从标准输入读取参数组合，处理每一行，然后用这些参数作为自变量执行一个外部命令。它的设计目的是与 `sweep_table.py` 通过管道符 (`|`) 配合使用。

#### 主要功能

  * **管道输入**：从标准输入 (`stdin`) 读取每行参数。
  * **命令执行**：为每一行输入执行一个指定的外部命令。
  * **通信协议**：可以使用 `--queue` 参数来管理如何将数据传递给外部程序。

#### 命令行参数

| 参数名 | 类型 |  默认值 | 描述 |
| :--- | :--- |  :--- | :--- |
|`cmd`|`string`|无|对每一行输入要执行的外部命令。|
|`queue`|`string`|shell| 通信协议。目前支持 `"shell"`，它直接执行命令; `"ipc"` 将数据发送至 ipc 列队再执行命令, 命令结束释放发送的数据。|
|`securty_mode`|`bool`|false|外部命令的解析方式, 默认值是 0 表示由 shell 直接解析, 适合复杂 shell 命令组合。 非 0 会先尝试将字符转换成 python object, 再转发给 shell。|

#### 示例

1.  **为线性扫描运行一个命令**  
    使用 `sweep_table.py` 生成参数组合，并通过管道将其传递给 `sweep_row.py`，每隔一秒钟输出一行表格数据并在第一列插入当前时间。

    ```bash
    # linux
    ./sweep_table.py --start 0 10 --end 1 20 --size 2 3 | ./sweep_row.py --securty_mode 0  --cmd "sleep 0.5  && python3 -c 'import datetime; print(datetime.datetime.now().strftime(\"%H:%M:%S\"), \" \",end=\"\")' && echo "

    # windows
    # TODO
    ./sweep_table.py --start 0 10 --end 1 20 --size 5 3 | ./sweep_row.py --cmd "echo  %time%"
    ```

2.  **为随机扫描运行一个命令**  
    两组 sweep_table 和 sweep_row 的递归调用, 内层循环生成 10 个随机数，外层循环通过 nominal 指定的字符串和复数列表。

    ```bash
    ./sweep_table.py --nominal '[["red", "green", "blue"], ["10+1i", "100-1i"]]' | ./sweep_row.py --cmd "./sweep_table.py --start 0 --end 1 --size 10 --random_sampling uniform | ./sweep_row.py --cmd echo | ./sweep_row.py --securty_mode 1 --cmd echo"
    ```
    命令分解:
    1. 生成一个由 nominal 指定的参数列表, 这两列分别是 `string` 和 `complex` 类型。 
    ``` bash
            ./sweep_table.py --nominal '[["red", "green", "blue"], ["10+1i", "100-1i"]]' | ./sweep_row.py --cmd echo
    ```
    2. 成一组均匀随机的参数列表。
    ``` bash
            ./sweep_table.py --start 0 --end 1 --size 10 --random_sampling uniform
    ```
    3. 打印前面两组参数列表组合后的结果, 要将前面参数列表里面复数字符串转成 `complex` 类型, 需要两层 `sweep_row` 的嵌套, 且第二层需要用启用 `securty_mode` .
    ``` bash
            ./sweep_row.py --cmd echo | ./sweep_row.py --securty_mode 1 --cmd echo
    ```
    将上面三条简单命令嵌套起来组合成本例, 将 `norminal` 指定参数列表和均匀随机分布的列表进行组合, 得到新的参数列表。
3.  **一个 IPC 的例子**  
    IPC 通常需要启用 `securty_mode`
    ``` bash
    # TODO
    ```
    
