# chapter01

## 编译运行

C语言: ANSI C标准, 用于Unix设计

.c → 预编译 → .i → 编译 → .s → 汇编 → .o → 链接 → a.out

-E  -S  -c   -l


## 虚拟存储

Linux虚拟地址空间: 底部是用户空间, 高地址顶部是内核空间

```
|       内核虚拟空间(用户程序不可见)
|            用户栈(运行时创建)
|                    ↓
|
|                    ↑
| 共享库的内存映射区域(mmap装载动态共享库或者malloc创建大内存)
|                    ↑
|          堆(进程通过malloc分配内存)
|           .bss(未初始化的全局变量)
|      .data(初始化的不为0的数据,静态分配)
|             .text(只读代码段)
|  保留区(未赋予物理地址,任何对它的引用都是非法的)
```
