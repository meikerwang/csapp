# chapter07 程序的链接

<!-- TOC -->

- [chapter07 程序的链接](#chapter07-程序的链接)
    - [1. 编译器](#1-编译器)
    - [2. 目标文件](#2-目标文件)
        - [可重定位目标文件](#可重定位目标文件)
        - [可执行目标文件格式](#可执行目标文件格式)
        - [存储器映像](#存储器映像)
    - [3. 符号解析](#3-符号解析)
        - [符号表](#符号表)
        - [符号解析](#符号解析)
        - [静态链接](#静态链接)
    - [4. 重定位](#4-重定位)
        - [重定位信息](#重定位信息)
        - [加载](#加载)
    - [5. 动态链接](#5-动态链接)
        - [加载时动态链接](#加载时动态链接)
        - [运行时动态链接](#运行时动态链接)
        - [位置无关代码PIC](#位置无关代码pic)

<!-- /TOC -->

## 1. 编译器

预处理 -> 编译 -> 汇编 -> 链接 -> 加载执行

静态链接器将多个可重定位目标文件合成一个可执行目标文件:
- 符号解析
  -  程序中有定义和引用的符号 (包括变量和函数等)
  -  编译器将定义的符号存放在一个符号表.symtab中
  -  链接器将每个符号的引用都与一个确定的符号定义建立关联
- 重定位
  - 将多个代码段与数据段分别合并为一个单独的代码段和数据段
  - 计算每个定义的符号在虚拟地址空间中的绝对地址
  - 将可执行文件中符号引用处的地址修改为重定位后的地址信息

## 2. 目标文件

### 可重定位目标文件

ELF文件组成: ELF头, .text, .rodata, .data, .bss. symtab, .rel.text, .rel.data, .debug, .line, .strtab, 节头表

ELF头: 指定节头表和程序头表的偏移位置和大小, 查看test.o 和test的elf头: readelf -h test.o
```
ELF Header:
  Magic:   7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00
  Class:                             ELF32
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              REL (Relocatable file) # test文件类型: EXEC (Executable file)
  Machine:                           Intel 80386
  Version:                           0x1
  Entry point address:               0x0                    # test 文件的入口点非0: 0x80482e0
  Start of program headers:          0 (bytes into file)    # test是EXEC, 一定有程序头表: 52 (bytes into file)
  Start of section headers:          432 (bytes into file)  # 两者节头表的起始地址不一样
  Flags:                             0x0
  Size of this header:               52 (bytes)
  Size of program headers:           0 (bytes)              # test有程序头表, 因此大小非0: 32 (bytes)
  Number of program headers:         0                      # test有程序头表, 程序头的数量9
  Size of section headers:           40 (bytes)
  Number of section headers:         11                     # 两者的section headers数量不一样, test更多
  Section header string table index: 8
```

节头表: 指定各个节的偏移位置和大小, 由一个结构体数组组成, 每一个元素表示一个节的信息, 包括偏移位置, 大小, 名字等, 每一个数组元素大小为 40bytes, readelf -S 查看节头表信息

test.o共有11个节, 从0x1b0开始时节头表, 可以看出在节头表中记录了.bss的大小, 但是.bss在目标文件中不占实际存储空间, 但是程序加载运行时需要分配空间


### 可执行目标文件格式

连接器将多个可重定位目标文件(Relocatable file)合并成一个可执行目标文件(Executable file)

ELF文件组成: ELF头, 程序头表, .init, .text, .rodata(前面都是只读段), .data, .bss(中间是读写段), .symtab, .debug, .line, .strtab, 节头表(后面是不需要加载到存储器)

EXEC和REL文件区别:
- ELF头中的e_entry给出了系统将控制权转移到起始虚拟地址(入口), 即第一条地址
- 包含.init节, 执行主函数之前会先执行.init函数
- 少了.rel.text和.rel.data, 因为数据和函数已经进行了符号解析和重定位
- 多了一个程序头表/段头表, 这是一个结构数组

ELF有两个可装入段
- 只读代码段: ELF头-.rodata这几个节构成了一个read-only code segment
- 读写数据段: .data和.bss两个节构成了read/write data segment


程序头表: 描述了某一个文件段或者特殊节和虚拟地址的对应关系, 可以通过readelf -l查看某一个段是否可装入, 在文件中的偏移地址和字节大小, 已经映射到虚拟地址的位置,权限, 对齐方式(一个段要4KB对齐)
```
例如readelf -l test查看
发现有8个段, 其中有两个可装入段LOAD
  LOAD           0x000000 0x08048000 0x08048000 0x005c0 0x005c0 R E 0x1000
  LOAD           0x000f08 0x08049f08 0x08049f08 0x00110 0x00114 RW  0x1000

   02     .interp .note.ABI-tag .note.gnu.build-id .gnu.hash .dynsym .dynstr .gnu.version .gnu.version_r .rel.dyn .rel.plt .init .plt .plt.got .text .fini .rodata .eh_frame_hdr .eh_frame
   03     .init_array .fini_array .jcr .dynamic .got .got.plt .data .bss
```

### 存储器映像

ABI描述了应用程序与操作系统之间的底层接口, 而存储器映像由ABI规定
- 只读代码段映射到0x8048000开始
- 可读数据段映射到只读代码段后面的4KB对齐的位置, 其中.bss初始化为0
- 运行时堆则在读写数据段后面的4KB对齐的高地址上, 通过malloc分配, 向高地址空间增长
- 用户栈则从用户空间最大地址向下增长
- 堆和栈中间的区域时共享库目标代码保留的, 而栈上面的高地址是内核区域



## 3. 符号解析

### 符号表

对一个模块m而言, 符号表.symtab中存在三种符号:
- 全局符号: m定义, 被其他外部模块引用, 非static全局变量和函数
- 外部符号: 在m中引用的由其他符号定义的函数和变量
- 本地符号: static属性的函数和变量, 不能被其他模块引用, 他们会分配在静态数据去.data或者.bss, 而不是.rel.data区域
- 局部变量分配在栈中, 不会存在于符号表中

符号表也是一个结构数组:
- 每个表项包含符号名, 相对于该节的偏移量, 字节数等
- 符号类型可以使NOTYPE(未指定), OBJECT(变量), FUNC(函数), SECTION等
- 绑定属性可以使LOCAL, GLOBAL, WEAK(无初始值), 其中LOCAL只对该目标文件可见
- 其他情况: ABS表示不该被重定位; UND表示未定义; COM表示未初
始化数据 (.bss) , 此时, value表示对齐要求, size给出最小大小
- readelf -s 查看符号表


### 符号解析

Symbol resolution: 将每个模块中引用的符号与某个目标模块中的定义符号建立关联. 本地符号在本模块内定义并引用, 因此其解析较简单, 只要与本模块内唯一的定义符号关联即可

函数名和已初始化的全局变量名是强符号, 未初始化的全局变量名是弱符号

多重定义符号解析三条规则:
- 强符号不能多次定义: 强符号只能被定义一次, 否则链接错误
- 若一个符号被定义为一次强符号和多次弱符号, 则按强定义为准: 对弱符号的引用被解析为其强定义符号
- 若有多个弱符号定义, 则任选其中一个:  使用命令 gcc –fno-common链接时, 会输出一条警告信息

使用建议
- 尽量避免使用全局变量, 而是使用static本地变量
- 弱使用, 全局变量要赋初值
- 引用外部变量时, 外部全局变量要使用extern

符号解析算法:
- E表示将被合并以组成可执行文件的所有目标文件集合, U表示当前所有未解析的引用符号的集合, D表示当前所有定义符号的集合
- 开始E、U、D为空, 首先扫描main.o, 把它加入E, 同时把myfun1加入U, main加入D
- 处理完libc.a时, U一定是空的, 如果U非空, 则报错并终止, 否则会合并目标文件并进行重定位处理


### 静态链接

```shell
gcc -c addvec.c multvec.c # 生成两个.o文件
ar rcs libvector.a addvec.o mulvec.o # 将目标文件打包静态库文件 , 也称存档文件(archive)
# main.c 中引用vector.h文件, 同时vector.h头文件中要先声明libvector.a中函数的原型(必须)
gcc -static -o main main.o ./libvector.a
# 在gcc命令行中无需明显指定C标准库libc.a(默认库)
```
链接顺序问题:
- 按照命令行给出的顺序扫描.o 和.a 文件
– 扫描期间将当前未解析的引用记录到一个列表U中
– 每遇到一个新的.o 或 .a 中的模块, 都试图用其来解析U中的符号
– 如果扫描到最后, U中还有未被解析的符号, 则发生错误
- 好的做法：将静态库放在命令行的最后


## 4. 重定位

### 重定位信息

重定位做的三件事
- 合并相同的节: 将集合E的所有目标模块中相同的节合并成新节
- 确定新节中所有定义符号在虚拟地址空间中的地址
- 对引用符号进行重定位, 修改.text节和.data节中对每个符号的引用(需要用到在.rel_data和.rel_text节中保存的重定位信息)

.rel.data和.rel.text节是一个结构数组, 每个表项的信息包含需要重定向的相对于该节的偏移位置, 重定位类型(PC相对/绝对), 在符号表中的索引

用命令readelf -r main.o可显示main.o中的重定位条目

通过计算可以算出相对寻址(.rel.text)和绝对寻址(.rel.data)的位置


### 加载

程序头表描述了可执行文件中代码和数据的存储映像

可执行文件的加载
- 在shell启动命令, 执行可执行文件 ./main
- 由shell命令行解释器构造argv和envp参数
- 调用fork()函数, 创建一个子进程, 与父进程shell完全相同 (只读/共享) , 包括只读代码段、可读写数据段、堆以及用户栈等
- 调用execve()函数,在当前进程 (新创建的子进程) 的上下文中加载并运行hello程序. 将hello中的.text节、.data节、.bss节等内容加载到当前进程的虚拟地址空间 (仅修改当前进程上下文中关于存储映像的一些数据结构, 不从磁盘拷贝代码、数据等内容)
- 加载后, 将PC (EIP) 设定指向Entry point (即符号_start处), 最终调用hello程序的main()函数, hello程序开始在一个进程的上下文中运行. main函数原型: int main(int argc, char *argv[], char *envp[]);
- start -> __libc_init_first  -> _init -> atexit -> main -> _exit 返回操作系统


## 5. 动态链接

### 加载时动态链接

静态库存在的问题: 主存资源浪费, 磁盘资源浪费, 不便于更新

共享库是第三种目标文件, 可以动态地在装入时或运行时被加载并链接

```shell
gcc –shared –fPIC –o mylib.so myproc1.o myproc2.o
gcc –o myproc main.o ./mylib.so
```

加载 myproc 时, 加载器发现在其程序头表中有 .interp 段, 其中包含了动态链接器路径名 ld-linux.so, 因而加载器根据指定路径加载并启动动态链接器运行. 动态链接器完成相应的重定位工作后, 再把控制权交给myproc, 启动其第一条指令执行

### 运行时动态链接

类UNIX系统中的动态链接器接口定义了相应的函数, 如dlopen, dlsym,dlerror, dlclose等, 其头文件为dlfcn.

```c
#include <stdio.h>
#include <dlfcn.h>
int main() {
    void *handle;
    void (*myfunc1)();
    char *error;
    /* 动态装入包含函数myfunc1()的共享库文件 */
    handle = dlopen("./mylib.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }
    /* 获得一个指向函数myfunc1()的指针myfunc1*/
    myfunc1 = dlsym(handle, "myfunc1");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "%s\n", error);
        exit(1);
    }
    /* 现在可以像调用其他函数一样调用函数myfunc1() */
    myfunc1();
    /* 关闭 (卸载) 共享库文件 */
    if (dlclose(handle) < 0) {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }
    return 0;
}
```


### 位置无关代码PIC

共享库代码是一种PIC
- 共享库代码的位置可以是不确定的, 可将共享库加载到任意地址运行
- 即使共享库代码的长度发生变化, 也不影响调用它的程序

1. 模块内的过程调用、跳转, 采用PC相对偏移寻址
2. 模块内数据访问, 如模块内的全局变量和静态变量
   1. 调用__get_pc后, call指令的返回地址被置ECX, 获取返回地址, 然后加上当前位置到.data的偏移0x118c, 以及加上变量在.data的偏移0x28, 得到最终全局变量的地址
   2. 需要多使用指令, 存在一定的性能损失
3. 模块外数据的引用
   1. 在.data节开始处设置一个指针数组, 全局偏移表GOT, 指向全局变量, 因此GOT与引用数据变量的位置的相对位置固定
   2. 编译器为GOT每一项生成一个重定位项 (如.rel节)
   3. 加载时, 动态链接器对GOT中各项进行重定位, 填入所引用的地址 (如&b)
   4. 缺陷: 多用4条指令; 并且多了Global Offset Table, 故需多用一个寄存器 (如EBX) , 易造成寄存器溢出
4. 模块间调用、跳转
   1. 类似于(3), 在GOT中加一个项(指针), 用于指向目标函数的首地址 (如&ext)
   2. 动态加载时, 填入目标函数的首地址
   3. 多用三条指令并额外多用一个寄存器 (如EBX)
   4. 可用“延迟绑定 (lazy binding) ”技术减少指令条数：不在加载时重定位, 而延迟到第一次函数调用时, 需要用GOT和PLT (Procedure linkage Table, 过程链接表)