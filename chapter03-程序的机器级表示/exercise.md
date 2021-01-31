# 练习题

### 3.1 存储器寻址

01x00(0xFF) 0x104(0xAB) 0x108(0x13) 0x10C(0x11) %eax(0x100) %ecx(0x1) %edx(0x3)

- %eax: 0x100
- 0x104: 0xAB
- $0x108: 0x108
- (%eax): M[0x100] = 0xFF
- 4(%eax): M[4+0x100] = M[0x104] = 0xAB
- 9(%eax,%edx): M[9+R[eax]+R[edx]] = M[9+0x100+0x3] = M[0x10C] = 0x11
- 260(%ecx,%edx): M[260+R[ecx]+R[edx]] = M[260+1+3] = M[0x108] = 0x13
- 0xFC(,%ecx,4): M[0xFC+R[ecx]\*4] = M[0xFC+0x4] = M[0x100] = 0xFF
- (%eax,%edx,4): M[R[eax]+R[edx]*4] = M[0x100+0x3\*4]=M[0x10C] = 0x11


### 3.2 确定指令后缀

```assembly
movl $eax, (%esp)
movw (%eax), %dx
movb $0xFF, %bl
movb (%esp,%edx,4), %dh
pushl $0xFF
movw %dx, (%eax)
popl %edi
```

### 3.3 找出错误

```shell
movb $0xF, (%bl) # %bl不能作为地址寄存器
movl %ax, (%esp) # 指令后缀和寄存器ID不匹配, 应该为movw
movw (%eax), 4(%esp) # 源地址和目的地址不能都是内存引用
movb %ah, %sh # 没有%sh这个寄存器
movl $eax, $0x123 #立即数不能作为目的地址
movl $eax, %dx # 目标寄存器错误, 应该为%edx
movb $si, 8(%ebp) # 指令后缀和寄存器不匹配, 应该为movw
```

### 3.4 根据类型移动数据

```
int int => movl %eax, (%edx)
char int => movsbl %al, (%edx)
char unsigned => movsbl %al, (%edx)
uchar int => movzbl %al, (%edx)
int char => movb %al, (%edx)
unsigned uchar => movb %al, (%edx)
unsigned int => movl %eax, (%edx)
```

### 3.5 写出等效的c代码

```c
void decode1(int *xp, int *yp, int *zp){
    int x = *xp;
    int y = *yp;
    int z = *zp;
    *yp = x;
    *zp = y;
    *xp = z;
}
```

### 3.6  leal指令
```
6 + x
x + y
x + 4y
7 + 9x
10 + 4y
9 + x + 2y
```


### 3.7 给出指令目的位置已经对应的值

```shell
addl $ecx, (%eax)
# 目的地址%eax对应的为0x100地址, 更新值为0xFF+0x1 = 0x100
subl %edx, 4(%eax)
# 目的地之对应的0x100+4 = 0x104地址, 更新为0xAB-0x3 = 0xA8
imull $16, (%eax, %edx, 4)
# 目的地址0x100+0x3*4 = 0x10C地址, 更新为0x11*16 = 0x110
incl 8(%eax)
# 目的地址0x100+8=0x108地址, 更新为0x13+1 = 0x14
decl %ecx
# 目的为%ecx寄存器, 更新为0x1-1 = 0x0
subl %edx, %eax
# 目的地址为%eax, 更新0x100-0x3 = 0xFD
```



### 3.8 移位操作

x和n两个参数的位置是%ebp中地址偏移8和12的地方, 在移位中第一个操作数是立即数或者$cl

```assembly
movl 8(%ebp), %eax  # get x
sall $2, %eax       # x <<= 2
movl 12(%ebp),%ecx  # get n
sarl %cl, %eax      # x >>= n
```



### 3.9 一个寄存器可以使用存放多个值

```c
int t1 = y ^ x;
int t2 = t1 >> 3;
int t3 = ~t2;
int t4 = t3 - z;

movl 12(%ebp), %eax
xorl 8(%ebp), %eax
sarl $3, %eax
notl %eax
subl 16(%ebp), %eax
```



### 3.10 异或操作实现置零

xorl %eax, %eax

1. 这段代码通过异或操作实现 x^x 的功能, 即 x = 0
2. 将寄存器%edx置0的更直接的汇编置零是 movl $0, %eax
3. 通过反汇编可以查看到,xorl版本只需要两个字节, 而movl需要5个字节
   - 31 c0              xor %eax, %eax
   - b8 00 00 00 00     mov $0x0, %eax



### 3.11 计算无符号除法

无符号除法需要设置%edx为0, 使用divl
```assembly
movl 8(%ebp), %eax      # load x into %eax
movl $0, %edx           # set high-order bits %edx to 0
divl 12(%ebp)           # unsigned divide by y
movl %eax, 4(%esp)      # store x / y
movl %edx, (%esp)       # store x % y
```

有符号除法idvil, 需要对%eax进行符号扩展, 放在%edx中

可以简单的将 `movl $0, %edx` 替换成 `cltd` 指令, 或者替换为
```assembly
movl %eax, %edx
sarl $31, %edx
```
根据x的符号将寄存器%edx设置为全零或者全1, 因此%edx和%eax联合起来存放x的64位扩展版本



### 3.16 条件语句&&

```assembly
testl %eax, %eax
je .L3              # 测试%eax不等于0则向下执行
testl %edx, %edx    # 测试%edx大于0则向下执行
jle .L3
addl %edx, %(eax)   # 实现*p += a
.L3:
```

### 3.17 if-true表达式

```c
int gododiff(int x, int y){
    int result;
    if(x < y)
        goto if_true
    result = x - y;
    goto done;
if_true:
    result = y - x;
done:
    return result;
}
```
if-false表达式和if-true翻译方案都可以, 但是对于if-false翻译对于没有else分支的情况下更简单, 可以省略if_false标签, goto if_false改为goto done即可
```c
    t = test-expr
    if(!t)
        goto if_false;
    then-statement
    goto done;
if_false:
    else-statement
done:
```


### 3.25

Tavg(p) = (1-p) * Tok * p (Tok + Tmp) = Tok + p*Tmp

Trandom = Tavg(0.5) = Tok + 0.5Tmp

分支行为容易预测时, 16个时钟, 随机模式大约需要31个时钟周期

预测错误的惩罚Tmp = (Trandom - Tok) / 0.5 = (31 - 16) / 2 = 30

分支预测错误时执行的时钟周期为 16 + 30 = 46 个时钟周期



### 3.30 获取当前指令地址

```assembly
    call next
next:
    popl %eax
```
A: call的时候会将下一条指令的地址压入栈中, 因此栈顶压入元素为程序计数器的值, 此时%eax存储的是popl指令的地址
B: 这个不是一个真正的过程调用, 因为call调用顺序和指令顺序相同, 并且没有ret返回, 而恢复栈指针是直接从栈中弹出的
C: 这个是IA32将程序计数器的值放到整数寄存器中的唯一方法


### 3.32 确定函数原型

函数原型:
```c
int fun(short c, char d, int *p, int x);
```

注意到将d存放到p处是通过 `movl 16(%ebp), %eax` 完成的, 因此p是第三个参数, 长度为4个字节, 因此p是int * 或者unsigned *

d是第一个参数, 长度一个字节, 并且是符号扩展, d是char类型

同样的减法是 `subl %eax, %edx` 完成的, 因此c是两个字节的, 并且是符号扩展, 因此c的类型是short

x的长度是4个字节, 因此x是int 或者 unsigned 类型

返回值类型是可能是int或者unsigned

以上多选择是相互独立的


### 3.34 递归调用

1. %eax中存放的是返回值, 因此 `movl $0, %eax` 表明默认返回值是0
2. 当条件 `testl %ebx, %ebx | je .L3` 表明条件 x == 0, 直接返回, 因此此时是 return 0, 而%ebx存放的是参数值x
3. 参数nx等于 x>>1, 然后将参数保存在栈顶 `movl %eax, (%esp)`, 然后调用rfun, 最后调用返回时利用返回值%eax计算新的结果
4. %ebx, %ebi, %esi是被调用者保存寄存器