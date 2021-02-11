# chapter03 程序的机器级表示

### 3.1 历史观点
8086 -> 80286 -> i386 -> i486 -> Pentium系列 -> Core2 -> Corei7

IA32架构与x86_64架构

### 3.2 程序编码

```shell
# -O1 表示一级优化, 一般O2是较好的选择
gcc -O1 -o p p1.c p2.c

# 生成code.s汇编文件
gcc -O1 -S code.c

# 生成code.o目标代码文件
gcc -O1 -c code.c

# 生成可执行文件prog
gcc -O1 -o prog code.o main.c

# 区别是连接之后的文件中的地址为实际偏移地址了
```


### 3.3 数据格式

整数类型: 单字节b(char), 字w(short), 双字l(int, long, char*)

浮点类型: 单精度s(float), 双精度l(double), 扩展双精度t(long double, 10/12字节)

沿用16位术语, Intel使用word表示16位数据类型, 因此32位是长字long word, 使用后缀l, 64位表示4个字(quad words), 使用后缀q


### 3.4 访问信息

通用寄存器: %eax, %ecx, %edx, $ebx, %edi, %esi, %ebp, %esp

寻址模式
- 立即数寻址: $+标准C用法的整数
- 寄存器寻址: R[E_a]表示寄存器E_a的值
- 存储器寻址: 立即数前面不加$表示内存地址, Mb[Addr]表示从内存地址Addr开始偏移b个字节的引用
  - Imm(Eb,Ei, s)表示基址变址寻址, 一个立即数偏移Imm, 基址寄存器Eb, 变址寄存器Ei, 比例因子(1,2,4,8), 则内存地址计算为Imm + R[Eb] + R[Esi] * s, 当s省略时, 比例因子默认为1


数据传送指令mov, movzbw, movsbw, pushl, popl
- IA32限制: 传送指令的而两个操作数不能都指向存储器位置
- movs和movz是符号扩展和零扩展
- IA32中高地址为栈低, 低地址为栈顶, 因此栈顶低地址为压入弹出

### 3.5 算术和逻辑运算

leal指令加载有效地址, 但是常用于整数的算术操作

一元操作: INC DEC NEG NOT

二元操作: ADD SUB IMUL XOR OR AND

移位操作: shl, sal, shr, sar: 移位操作的寄存器只能使用立即数或者%cl

乘法指令: imull/mull R[%edx]:R[%eax] <-- S * R[%eax]

除法指令: idivl/divl
- R[%edx] = R[%edx]:R[%eax] mod S
- R[%eax] = R[%edx]:R[%eax] / S
- idivl需要对%eax进行符号扩展, cltd结果存放在%edx中
- divl需要对%eax进行零扩展, 结果存放在%edx中



### 3.6 控制指令

条件标志位: CF, ZF, SF, OF

比较指令 cmp S2, S1: s1 - S2  test S2, S1

访问条件: set系列 e/ne/s/ns/g/ge/l/le/a/ae/b/be, 将一个8位字节或者寄存器设置为0或者1

跳转指令: jmp, je, js, ge 等直接跳转和简介跳转

条件分支: if-true翻译和if-false翻译

循环: do-while, while, for

条件传送指令: cmove/cmovne cmovs/cmovns等

switch语句: 使用跳转表(数组)实现, 更加高效, 不需要那么多test/cmp指令, 表项i是一段代码的地址, c语言中存在&&可以引用一个指向代码位置的指针



### 3.7 过程调用

栈帧结构: %ebp帧指针, %esp栈指针, call 指令, leave, ret

调用者保存寄存器: %eax, %ecx, %edx, 被调用者保存寄存器:%ebx, %ebi, %esi, 这些被调用者使用前需要将其保存在栈中, 返回时需要恢复

gcc栈分配为16字节的整数倍, 进行数据对齐, 包括%ebp的4个字节, 返回地址4个字节, 然后其他空间 16*n + 8, 因此可能存在不使用的栈空间

