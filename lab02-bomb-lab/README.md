<!-- TOC -->

- [二进制炸弹实验](#%e4%ba%8c%e8%bf%9b%e5%88%b6%e7%82%b8%e5%bc%b9%e5%ae%9e%e9%aa%8c)
  - [phase_1](#phase1)
  - [phase_2](#phase2)
  - [phase_3](#phase3)

<!-- /TOC -->

# 二进制炸弹实验


http://csapp.cs.cmu.edu/3e/labs.html


```shell
file bomb
bomb: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, for GNU/Linux 2.6.24, BuildID[sha1]=11c83ac9c51d3036cf2669235060f17e2cd0400b, not stripped
```

main代码
```
  400e32:	e8 67 06 00 00       	callq  40149e <read_line>   # 调用read_line读入字符串
  400e37:	48 89 c7             	mov    %rax,%rdi            # 输入的字符串地址用%rax返回, 传入%rdi作为第一个参数
  400e3a:	e8 a1 00 00 00       	callq  400ee0 <phase_1>     # 调用phase_1函数
  400e3f:	e8 80 07 00 00       	callq  4015c4 <phase_defused>
```

## phase_1

```c
0000000000400ee0 <phase_1>:
  400ee0:	48 83 ec 08          	sub    $0x8,%rsp                  # 栈下移
  400ee4:	be 00 24 40 00       	mov    $0x402400,%esi             # %esi内容为0x402400, 参数1是%edi, 参数2是%esi, 比较内容即可
  400ee9:	e8 4a 04 00 00       	callq  401338 <strings_not_equal> # 调用strings_not_equal函数
  400eee:	85 c0                	test   %eax,%eax                  # 测试eax是否为0, 即返回值是否为0,
  400ef0:	74 05                	je     400ef7 <phase_1+0x17>      # 返回值为0, 则成功了, not_equal等于0, 则表示equals相等则成功
  400ef2:	e8 43 05 00 00       	callq  40143a <explode_bomb>      # 不是0, 则失败, 引爆炸弹
  400ef7:	48 83 c4 08          	add    $0x8,%rsp
  400efb:	c3                   	retq



objdump -D bomb # 查看数据段位置
  4023ff:	00 42 6f             	add    %al,0x6f(%rdx)
  402402:	72 64                	jb     402468 <_IO_stdin_used+0x1b8>
  402404:	65 72 20             	gs jb  402427 <_IO_stdin_used+0x177>
  402407:	72 65                	jb     40246e <_IO_stdin_used+0x1be>
  402409:	6c                   	insb   (%dx),%es:(%rdi)



# 通过gdb工具查看内存0x402400位置的字符串内容(以\0结尾)
x/1s 0x402400
0x402400:	"Border relations with Canada have never been better."
# 或者使用ddx工具转换为字符串, 然后搜索 426f 就可以找到
xxd bomb > /tmp/hex

# phase_1 答案是:
Border relations with Canada have never been better.
```



## phase_2

rdi, rsi, rdx, rcx, r8, r9

1. `read_six_numbers`

会调用__isoc99_sscanf输入参数, 8个参数, 更多的参数通过栈来传递
```
0x14 = 0x14(%rsp)
0x10 = 0x10(%rsp)
r9  = rsi + 0xc
r8  = rsi + 0x8
rcx = rsi + 4
rdx = rsi
arg2(rsi) -> &x
arg1(rdi) -> input
```

第一个参数是rdi, 输入的字符串, 第二个参数是$0x4025c3格式串, 查看二进制为"%d %d %d %d %d %d"

__isoc99_sscanf (const char *s, const char *format, ...)

因此该函数的大致实现是:
```c
void read_six_numbers(char *s, int *nums){
    const char * format = "%d %d %d %d %d %d";
    int cnt = sscanf(s, format, nums);
    if(cnt > 5){
        return;
    }else{
        explode_bomb();
    }
}

# gdb调试结果:
(gdb) x/1xw ($rsp)
0x7fffffffdca0:	0x00000001
(gdb) x/1xw ($rsp)+4
0x7fffffffdca4:	0x00000002
(gdb) x/1xw ($rsp)+8
0x7fffffffdca8:	0x00000004
(gdb) x/1xw ($rsp)+0x10
0x7fffffffdcb0:	0x00000010
(gdb) x/1xw ($rsp)+0x14
0x7fffffffdcb4:	0x00000020
(gdb) x/1xw ($rsp)+0x18
0x7fffffffdcb8:	0x00401431


# phase_2 汇编代码解释
0000000000400efc <phase_2>:
  400efc:	55                   	push   %rbp
  400efd:	53                   	push   %rbx
  400efe:	48 83 ec 28          	sub    $0x28,%rsp
  400f02:	48 89 e6             	mov    %rsp,%rsi
  400f05:	e8 52 05 00 00       	callq  40145c <read_six_numbers>  # 读取六个数, 存放在栈中位置
  400f0a:	83 3c 24 01          	cmpl   $0x1,(%rsp)                # 第一个输入元素在栈顶, 比较num[0]和1
  400f0e:	74 20                	je     400f30 <phase_2+0x34>      # 相等则跳转进入下一步
  400f10:	e8 25 05 00 00       	callq  40143a <explode_bomb>      # num[0] != 1 则爆炸
  400f15:	eb 19                	jmp    400f30 <phase_2+0x34>
  400f17:	8b 43 fc             	mov    -0x4(%rbx),%eax            # eax = M[rbx - 4]也就是栈顶, 前一个参数
  400f1a:	01 c0                	add    %eax,%eax                  # eax = eax * 2
  400f1c:	39 03                	cmp    %eax,(%rbx)                # 比较前一个数的两倍的大小和当前位置元素是否相等
  400f1e:	74 05                	je     400f25 <phase_2+0x29>      # 相等则跳转, 否则就爆炸
  400f20:	e8 15 05 00 00       	callq  40143a <explode_bomb>      # 爆炸
  400f25:	48 83 c3 04          	add    $0x4,%rbx                  # M[R[rbx]]为当前参数, M[R[rbx]+4]表示下一个位置, 表明进入下一次循环
  400f29:	48 39 eb             	cmp    %rbp,%rbx                  # 比较rbp和rbx, 也就是是否到达栈参数顶部
  400f2c:	75 e9                	jne    400f17 <phase_2+0x1b>      # 不相等则循环
  400f2e:	eb 0c                	jmp    400f3c <phase_2+0x40>      # 结束循环
  400f30:	48 8d 5c 24 04       	lea    0x4(%rsp),%rbx             # rbx = (rsp + 0x4), M[rbx] = num[1] 第二个参数, 为2
  400f35:	48 8d 6c 24 18       	lea    0x18(%rsp),%rbp            # rbp = (rsp + 0x18), 最后一个参数的栈上面4个单元的地址
  400f3a:	eb db                	jmp    400f17 <phase_2+0x1b>      # 跳转
  400f3c:	48 83 c4 28          	add    $0x28,%rsp
  400f40:	5b                   	pop    %rbx
  400f41:	5d                   	pop    %rbp
  400f42:	c3                   	retq

# phase_2 答案是:
1 2 4 8 16 32
```



## phase_3

调用__isoc99_sscanf
```
  400f51:	be cf 25 40 00       	mov    $0x4025cf,%esi
  400f56:	b8 00 00 00 00       	mov    $0x0,%eax
  400f5b:	e8 90 fc ff ff       	callq  400bf0 <__isoc99_sscanf@plt>
```

查看数据段0x4025cf位置对应的是 "25 64 20 25 64 00" => "%d %d", 因此输入为两个参数

gdb运行到phase_3 查看栈顶元素
```c
(gdb) i r $rsp
rsp            0x7fffffffdcc0	0x7fffffffdcc0
(gdb) x/8xw ($rsp)
0x7fffffffdcc0:	0x00000000	0x00000000	0x00000001	0x00000007
0x7fffffffdcd0:	0x00400c90	0x00000000	0x00400e77	0x00000000
# 可以知道参数1位置在0x8(%esp), 参数2在0xC(%esp)

# 第一个参数应该不大于7, 且应该是无符号数, 则0<=a<=7
cmpl   $0x7,0x8(%rsp)               # 比较 a 和 7 的大小
ja     400fad <phase_3+0x6a>

# dest = M[0x402470 + 8 * a] 注意是小端方式
jmpq   *0x402470(,%rax,8)

# 此时存在跳转表

  402470:	7c 0f                	jl     402481 <_IO_stdin_used+0x1d1>
  402472:	40 00 00             	add    %al,(%rax)
  402475:	00 00                	add    %al,(%rax)
  402477:	00 b9 0f 40 00 00    	add    %bh,0x400f(%rcx)
  40247d:	00 00                	add    %al,(%rax)
  40247f:	00 83 0f 40 00 00    	add    %al,0x400f(%rbx)
  402485:	00 00                	add    %al,(%rax)
  402487:	00 8a 0f 40 00 00    	add    %cl,0x400f(%rdx)
  40248d:	00 00                	add    %al,(%rax)
  40248f:	00 91 0f 40 00 00    	add    %dl,0x400f(%rcx)
  402495:	00 00                	add    %al,(%rax)
  402497:	00 98 0f 40 00 00    	add    %bl,0x400f(%rax)
  40249d:	00 00                	add    %al,(%rax)
  40249f:	00 9f 0f 40 00 00    	add    %bl,0x400f(%rdi)
  4024a5:	00 00                	add    %al,(%rax)
  4024a7:	00 a6 0f 40 00 00    	add    %ah,0x400f(%rsi)
  4024ad:	00 00                	add    %al,(%rax)

a = 0: M[0x402470] = 0x00 00 00 00 00 40 0f 7c
a = 0: 0x400f7c
a = 1: 0x400fb9
a = 2: 0x400f83
a = 3: 0x400f8a
a = 4: 0x400f91
a = 5: 0x400f98
a = 6: 0x400f9f
a = 7: 0x400fa6

0000000000400f43 <phase_3>:
  400f43:	48 83 ec 18          	sub    $0x18,%rsp
  400f47:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx
  400f4c:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx
  400f51:	be cf 25 40 00       	mov    $0x4025cf,%esi
  400f56:	b8 00 00 00 00       	mov    $0x0,%eax
  400f5b:	e8 90 fc ff ff       	callq  400bf0 <__isoc99_sscanf@plt>
  400f60:	83 f8 01             	cmp    $0x1,%eax                    # 读入两个参数, %eax为2, 比较%eax和1
  400f63:	7f 05                	jg     400f6a <phase_3+0x27>        # %eax大于1则跳转到下两个指令
  400f65:	e8 d0 04 00 00       	callq  40143a <explode_bomb>
  400f6a:	83 7c 24 08 07       	cmpl   $0x7,0x8(%rsp)               # 比较 a 和 7 的大小
  400f6f:	77 3c                	ja     400fad <phase_3+0x6a>        # 大于则爆炸, 表明参数 a 应该不大于7, 并且a是无符号数, 则0<=a<=7
  400f71:	8b 44 24 08          	mov    0x8(%rsp),%eax               # R[eax] = a
  400f75:	ff 24 c5 70 24 40 00 	jmpq   *0x402470(,%rax,8)           # 跳转地址是M[0x402470 + 8 * a] (尾号是0或者8才可以)
  400f7c:	b8 cf 00 00 00       	mov    $0xcf,%eax                   # case 0: eax = 0xcf    207
  400f81:	eb 3b                	jmp    400fbe <phase_3+0x7b>
  400f83:	b8 c3 02 00 00       	mov    $0x2c3,%eax                  # case 2: eax = 0x2c3   707
  400f88:	eb 34                	jmp    400fbe <phase_3+0x7b>
  400f8a:	b8 00 01 00 00       	mov    $0x100,%eax                  # case 3: eax = 0x100   256
  400f8f:	eb 2d                	jmp    400fbe <phase_3+0x7b>
  400f91:	b8 85 01 00 00       	mov    $0x185,%eax                  # case 4: eax = 0x185   389
  400f96:	eb 26                	jmp    400fbe <phase_3+0x7b>
  400f98:	b8 ce 00 00 00       	mov    $0xce,%eax                   # case 5: eax = 0xce    206
  400f9d:	eb 1f                	jmp    400fbe <phase_3+0x7b>
  400f9f:	b8 aa 02 00 00       	mov    $0x2aa,%eax                  # case 6: eax = 0x2aa   682
  400fa4:	eb 18                	jmp    400fbe <phase_3+0x7b>
  400fa6:	b8 47 01 00 00       	mov    $0x147,%eax                  # case 7: eax = 0x147   327
  400fab:	eb 11                	jmp    400fbe <phase_3+0x7b>
  400fad:	e8 88 04 00 00       	callq  40143a <explode_bomb>
  400fb2:	b8 00 00 00 00       	mov    $0x0,%eax                    # default: 不存在此项
  400fb7:	eb 05                	jmp    400fbe <phase_3+0x7b>
  400fb9:	b8 37 01 00 00       	mov    $0x137,%eax                  # case 1: eax = 0x137   311
  400fbe:	3b 44 24 0c          	cmp    0xc(%rsp),%eax               # eax中的值和第二个参数num2比较
  400fc2:	74 05                	je     400fc9 <phase_3+0x86>        # 相等则跳转成功, 否则就爆炸
  400fc4:	e8 71 04 00 00       	callq  40143a <explode_bomb>
  400fc9:	48 83 c4 18          	add    $0x18,%rsp
  400fcd:	c3                   	retq
```

因此该函数的大致实现是:
```c
unsigned int a;
int b;

eax = sscanf(input, "%d %d", &a, &b)
if(eax > 1){
    if(a > 7){
        explode_bomb();
    }
    else{
        eax = a;
        dest = (0x402470 + 8 * eax)
        eax = b
        if(b == ?){
            success();
        }
    }
}

# phase_3 答案是(多个):
0 207
1 311
2 707
3 256
4 389
5 206
6 682
7 327
```