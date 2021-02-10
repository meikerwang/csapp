
<!-- TOC -->

- [二进制炸弹实验](#%e4%ba%8c%e8%bf%9b%e5%88%b6%e7%82%b8%e5%bc%b9%e5%ae%9e%e9%aa%8c)
  - [phase_1](#phase1)
  - [phase_2](#phase2)
  - [phase_3](#phase3)
  - [phase_4](#phase4)
  - [phase_5](#phase5)
  - [phase_6](#phase6)
  - [secret_phase](#secretphase)

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



## phase_4

调用 __isoc99_sscanf, 格式串参数为 0x4025cf 对应的字符串 "%d %d"

输入两个参数num1和num2, 然后构造参数调用func4, 通过分析可知, 参数num1=7可以成功返回, num2=0则phase_4可以通过

```assembly
000000000040100c <phase_4>:
  40100c:	48 83 ec 18          	sub    $0x18,%rsp
  401010:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx               # 参数b地址(sscanf第4个参数)
  401015:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx               # 参数a地址(sscanf第3个参数)
  40101a:	be cf 25 40 00       	mov    $0x4025cf,%esi
  40101f:	b8 00 00 00 00       	mov    $0x0,%eax
  401024:	e8 c7 fb ff ff       	callq  400bf0 <__isoc99_sscanf@plt>
  401029:	83 f8 02             	cmp    $0x2,%eax                    # 表明sscanf需要读入两个参数, 个数不对, 则explode
  40102c:	75 07                	jne    401035 <phase_4+0x29>
  40102e:	83 7c 24 08 0e       	cmpl   $0xe,0x8(%rsp)               # 比较第一个参数和 14 的大小
  401033:	76 05                	jbe    40103a <phase_4+0x2e>        # 如果是小于或者等于, 则成功跳转, jbe表明a是一个无符号数,4字节
  401035:	e8 00 04 00 00       	callq  40143a <explode_bomb>
  40103a:	ba 0e 00 00 00       	mov    $0xe,%edx                    # edi是参数a的值, esi是0, edx是14
  40103f:	be 00 00 00 00       	mov    $0x0,%esi
  401044:	8b 7c 24 08          	mov    0x8(%rsp),%edi
  401048:	e8 81 ff ff ff       	callq  400fce <func4>               # 调用func4
  40104d:	85 c0                	test   %eax,%eax                    # 判断返回值%eax是否为0
  40104f:	75 07                	jne    401058 <phase_4+0x4c>        # 不为0, 则跳转至explode
  401051:	83 7c 24 0c 00       	cmpl   $0x0,0xc(%rsp)               # 比较参数2和0的大小
  401056:	74 05                	je     40105d <phase_4+0x51>        # 相等则成功跳转, 表明参数2为0
  401058:	e8 dd 03 00 00       	callq  40143a <explode_bomb>
  40105d:	48 83 c4 18          	add    $0x18,%rsp
  401061:	c3                   	retq



0000000000400fce <func4>:
  400fce:	48 83 ec 08          	sub    $0x8,%rsp                  # edi=a, esi=b, edx=c(第一次b=0, c=14)
  400fd2:	89 d0                	mov    %edx,%eax                  # eax=c
  400fd4:	29 f0                	sub    %esi,%eax                  # eax=c-b
  400fd6:	89 c1                	mov    %eax,%ecx                  # ecx=c-b
  400fd8:	c1 e9 1f             	shr    $0x1f,%ecx                 # ecx=0: ecx逻辑右移31位, 而其为正数, 因此为0
  400fdb:	01 c8                	add    %ecx,%eax                  # eax=c-b
  400fdd:	d1 f8                	sar    %eax                       # eax算术右移1为 eax=(c-b)/2
  400fdf:	8d 0c 30             	lea    (%rax,%rsi,1),%ecx         # ecx=(eax+esi)=(c-b)/2+b = (b+c)/2
  400fe2:	39 f9                	cmp    %edi,%ecx                  # 比较ecx和参数a的大小
  400fe4:	7e 0c                	jle    400ff2 <func4+0x24>        # 如果小于或等于, 则跳转
  400fe6:	8d 51 ff             	lea    -0x1(%rcx),%edx            # 如果大于: ecx > a, edx = ecx-1
  400fe9:	e8 e0 ff ff ff       	callq  400fce <func4>             # 调用func4: edi=a, esi=b, edx=ecx-1, 返回eax
  400fee:	01 c0                	add    %eax,%eax                  # 返回值eax=eax*2, 结束
  400ff0:	eb 15                	jmp    401007 <func4+0x39>
  400ff2:	b8 00 00 00 00       	mov    $0x0,%eax                  # eax=0
  400ff7:	39 f9                	cmp    %edi,%ecx                  # 比较ecx=(c+b)/2和参数a的大小
  400ff9:	7d 0c                	jge    401007 <func4+0x39>        # 如果大于或者等于则跳转成功, 返回值eax=0
  400ffb:	8d 71 01             	lea    0x1(%rcx),%esi             # 如果小于, 则 esi = rcx + 1 = (c+b)/2+1
  400ffe:	e8 cb ff ff ff       	callq  400fce <func4>             # 继续调用, edi=a, esi=rcx+1, edx=c, 返回值eax
  401003:	8d 44 00 01          	lea    0x1(%rax,%rax,1),%eax      # eax=eax+eax+1, 返回值eax=eax*2+1
  401007:	48 83 c4 08          	add    $0x8,%rsp
  40100b:	c3                   	retq
```


```c
// func4对应的C语言版本
int func4(int a, int b, int c) {
    int t1 = (c - b) / 2;
    int t2 = t1 + b;
    if (t2 > a) {
        int t5 = t2 - 1;
        int t6 = func4(a, b, t5);
        return t6 * 2;
    } else {
        if (t2 >= a) {
            return 0;
        } else {
            int t3 = t2 + 1;
            int t4 = func4(a, t3, c);
            return 2 * t4 + 1;
        }
    }
}

// 初次调用为func4(a, 0, 14);
// 构造测试函数, 参数a在0-14之间, 最终应该返回0
int main() {
    for (int a = 0; a <= 14; a++) {
        int result = func4(a, 0, 14);
        if (result == 0) {
            printf("a=%d\n", a);
        }
    }
}
// 输出可能的值为0, 1, 3, 7, 多选1

// phase_3 答案是(多个):
0 0
1 0
3 0
7 0
```


## phase_5

```
0000000000401062 <phase_5>:
  401062:	53                   	push   %rbx
  401063:	48 83 ec 20          	sub    $0x20,%rsp
  401067:	48 89 fb             	mov    %rdi,%rbx                    # rbx=rdi, rbx存放输入字符的地址
  40106a:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax
  401071:	00 00
  401073:	48 89 44 24 18       	mov    %rax,0x18(%rsp)
  401078:	31 c0                	xor    %eax,%eax                    # eax = 0
  40107a:	e8 9c 02 00 00       	callq  40131b <string_length>       # 计算输入字符长度函数
  40107f:	83 f8 06             	cmp    $0x6,%eax                    # 返回值大小和6比较
  401082:	74 4e                	je     4010d2 <phase_5+0x70>        # 返回长度应该为6
  401084:	e8 b1 03 00 00       	callq  40143a <explode_bomb>
  401089:	eb 47                	jmp    4010d2 <phase_5+0x70>
  40108b:	0f b6 0c 03          	movzbl (%rbx,%rax,1),%ecx           # ecx = M[rax+rbx]=M[rax+rbx]=输入串第k个字符 , 偏移
  40108f:	88 0c 24             	mov    %cl,(%rsp)                   # cl= x
  401092:	48 8b 14 24          	mov    (%rsp),%rdx                  # rdx=x
  401096:	83 e2 0f             	and    $0xf,%edx                    # edx高4位置0, 说明只取低的四位, 0<=x<=15
  401099:	0f b6 92 b0 24 40 00 	movzbl 0x4024b0(%rdx),%edx          # edx = M[0x4024b0+x]
  4010a0:	88 54 04 10          	mov    %dl,0x10(%rsp,%rax,1)        # 偏移之后将值放入栈中
  4010a4:	48 83 c0 01          	add    $0x1,%rax                    # rax = rax + 1
  4010a8:	48 83 f8 06          	cmp    $0x6,%rax                    # 比较是否结束rax 和 6 的大小
  4010ac:	75 dd                	jne    40108b <phase_5+0x29>        # 如果不等于, 则跳转回去, 循环, 否则进行下一步
  4010ae:	c6 44 24 16 00       	movb   $0x0,0x16(%rsp)              # 0x16(rsp)置0
  4010b3:	be 5e 24 40 00       	mov    $0x40245e,%esi               # esi=0x40245e
  4010b8:	48 8d 7c 24 10       	lea    0x10(%rsp),%rdi              # rdi=rsp+10, 地址值
  4010bd:	e8 76 02 00 00       	callq  401338 <strings_not_equal>   # 比较栈10(rsp) ~ 15(rsp)的位置字符串是否不相等
  4010c2:	85 c0                	test   %eax,%eax                    # 查看eax是否为0
  4010c4:	74 13                	je     4010d9 <phase_5+0x77>        # 等于0, 则表明相等, 进行下一步, 否则爆炸
  4010c6:	e8 6f 03 00 00       	callq  40143a <explode_bomb>
  4010cb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
  4010d0:	eb 07                	jmp    4010d9 <phase_5+0x77>
  4010d2:	b8 00 00 00 00       	mov    $0x0,%eax                    # eax=0
  4010d7:	eb b2                	jmp    40108b <phase_5+0x29>        # 跳转回去




地址0x40245e的长度为6的字符串: 666c 7965 7273 0000: flyers

0x4024b0位置的字符串为:
000024b0: 6d 61 64 75 69 65 72 73 6e 66 6f 74 76 62 79 6c  maduiersnfotvbyl
          m  a  d  u  i  e  r  s  n  f  o  t  v  b  y  l

比较字符串: flyers

y = x&0xF: 0<= y <=15
M[0x4024b0+x&F] == flyers
i=0: M[4024b0+y] = f  ==> y = 9(1001)
i=1: M[4024b1+y] = l  ==> y = 15(1110)
i=2: M[4024b2+y] = y  ==> y = 14(1100)
i=3: M[4024b3+y] = e  ==> y = 5(0010)
i=4: M[4024b4+y] = r  ==> y = 6(0010)
i=5: M[4024b4+y] = s  ==> y = 7(0010)



'A': 0100 0000 + 1
'a': 0110 0000 + 1
因此: chr(0b01000000 + y) = chr(64+y): IONEFG
或者  chr(0b01100000 + y) = chr(96+y): ionefg
```



## phase_6

调用read_six_numbers输入6个数字

输入的数要小于等于6: 因为无符号数, 又-1，所以还要大于等于1, 因此范围是1-6

然后对输入的数进行测试, 当前数是否和之前输入的数相同, 表明这6个数都不应该相同

遍历这6个数时, 会使用 `sub    (%rax),%edx` 相减, 并且由于%edx=%ecx=7, x = 7 - x

对于链表数据分析: data为{332, 168, 942, 691, 477, 443}

降序排列的链表节点为{3, 4, 5, 6, 1, 2}

然后需要考虑用7相减, 因此输入为 4 3 2 1 6 5



## secret_phase

phase_6下面还有个fun7函数未使用, fun7函数被secret_phase函数调用, 又被phase_defused调用

```c
# 查看程序phase_defused
  4015d8:	83 3d 81 21 20 00 06 	cmpl   $0x6,0x202181(%rip)          # 603760 <num_input_strings> 比较过关的数目是否为6
  4015df:	75 5e                	jne    40163f <phase_defused+0x7b>  # 不等于, 则程序结束
  4015e1:	4c 8d 44 24 10       	lea    0x10(%rsp),%r8               # 通过了前六个phase而且没有触发爆炸就能进入接下来的代码

# 查看$0x402619, 其字符串为%d %d %s
  4015f0:	be 19 26 40 00       	mov    $0x402619,%esi               # 25 64 20 25 64 20 25 73 00 "%d %d %s"
  4015f5:	bf 70 38 60 00       	mov    $0x603870,%edi               # phase_4输入
  4015fa:	e8 f1 f5 ff ff       	callq  400bf0 <__isoc99_sscanf@plt> # 输入
  4015ff:	83 f8 03             	cmp    $0x3,%eax                    # 判断输入是否为3
  401602:	75 31                	jne    401635 <phase_defused+0x71>

# 不是, 则程序跳转, 输出下面这一句 $0x402558: Congratulations! You've defused the bomb!, 表明六个Phase都已经通过
# 但是可知跳过了上一条指令 callq secret_phase, 而是没有进入secret_phase
  401630:	e8 0d fc ff ff       	callq  401242 <secret_phase>
  401635:	bf 58 25 40 00       	mov    $0x402558,%edi
  40163a:	e8 d1 f4 ff ff       	callq  400b10 <puts@plt>


# 如果输入参数是3, 则比较$0x402622字符串: 4472 4576 696c 00, 是DrEvil
  401604:	be 22 26 40 00       	mov    $0x402622,%esi               # $0x402622字符串是:DrEvil
  401609:	48 8d 7c 24 10       	lea    0x10(%rsp),%rdi              # 比较两者是否相同
  40160e:	e8 25 fd ff ff       	callq  401338 <strings_not_equal>
  401613:	85 c0                	test   %eax,%eax
  401615:	75 1e                	jne    401635 <phase_defused+0x71>  # 不相同, 则同样跳过secret_phase

# 因此只有phase_4中输入的第三个参数是 DrEvil 并且通过前面六个phase
# 才会通过puts输出两个提示并且进入secret_hase
#         Curses, you've found the secret phase!
#         But finding it and solving it are quite different...
  401617:	bf f8 24 40 00       	mov    $0x4024f8,%edi
  40161c:	e8 ef f4 ff ff       	callq  400b10 <puts@plt>
  401621:	bf 20 25 40 00       	mov    $0x402520,%edi
  401626:	e8 e5 f4 ff ff       	callq  400b10 <puts@plt>
  40162b:	b8 00 00 00 00       	mov    $0x0,%eax
  401630:	e8 0d fc ff ff       	callq  401242 <secret_phase>

# 读入一个字符输入, 转换为long整数, 要不大于1000, 然后调用fun7函数, 并且需要返回2才可以成功
# fun7返回2后输出$0x402438字符串: Wow! You 've defused the secret stage!
0000000000401242 <secret_phase>:
  401242:	53                   	push   %rbx
  401243:	e8 56 02 00 00       	callq  40149e <read_line>         # 读入一行输入
  401248:	ba 0a 00 00 00       	mov    $0xa,%edx
  40124d:	be 00 00 00 00       	mov    $0x0,%esi
  401252:	48 89 c7             	mov    %rax,%rdi
  401255:	e8 76 f9 ff ff       	callq  400bd0 <strtol@plt>        # 将字符串转换为long数字, 十进制数
  40125a:	48 89 c3             	mov    %rax,%rbx                  # ebx = eax
  40125d:	8d 40 ff             	lea    -0x1(%rax),%eax            # eax = eax-1
  401260:	3d e8 03 00 00       	cmp    $0x3e8,%eax                # 比较eax 和 1000
  401265:	76 05                	jbe    40126c <secret_phase+0x2a> # 小于或者等于, 则跳过炸弹
  401267:	e8 ce 01 00 00       	callq  40143a <explode_bomb>
  40126c:	89 de                	mov    %ebx,%esi                  # 参数2: esi = ebx
  40126e:	bf f0 30 60 00       	mov    $0x6030f0,%edi             # 参数1: edi
  401273:	e8 8c ff ff ff       	callq  401204 <fun7>              # 调用fun7函数
  401278:	83 f8 02             	cmp    $0x2,%eax                  # 比较返回值eax和2
  40127b:	74 05                	je     401282 <secret_phase+0x40> # 如果相等, 则跳过, 输出
  40127d:	e8 b8 01 00 00       	callq  40143a <explode_bomb>
  401282:	bf 38 24 40 00       	mov    $0x402438,%edi
  401287:	e8 84 f8 ff ff       	callq  400b10 <puts@plt>
  40128c:	e8 33 03 00 00       	callq  4015c4 <phase_defused>
  401291:	5b                   	pop    %rbx
  401292:	c3                   	retq

# 如何使fun7返回2呢
# 需要先返回eax=0, 在返回2*eax + 1, 最后返回 2 * eax
# 返回顺序是: edi == esi, edi < esi, edi > esi
# 因此调用顺序是 edi > esi, edi  < esi, edi == edi
# 实际上func7类似于构建了一个二叉树
0000000000401204 <fun7>:
  401204:	48 83 ec 08          	sub    $0x8,%rsp
  401208:	48 85 ff             	test   %rdi,%rdi
  40120b:	74 2b                	je     401238 <fun7+0x34>       # 如果rdi为0, 则返回-1, 结束
  40120d:	8b 17                	mov    (%rdi),%edx              # rdi地址的数据移动到edx, edx = *rdi
  40120f:	39 f2                	cmp    %esi,%edx                # 比较 *rdi 和 输入值 esi 值的大小
  401211:	7e 0d                	jle    401220 <fun7+0x1c>       # 如果小于等于
  401213:	48 8b 7f 08          	mov    0x8(%rdi),%rdi           # 如果*rdi > esi: rdi = rdi + 8
  401217:	e8 e8 ff ff ff       	callq  401204 <fun7>            # 递归执行
  40121c:	01 c0                	add    %eax,%eax                # 返回值*2
  40121e:	eb 1d                	jmp    40123d <fun7+0x39>       # 返回, 表明之前返回值应该为1
  401220:	b8 00 00 00 00       	mov    $0x0,%eax                # *rdi <= esi
  401225:	39 f2                	cmp    %esi,%edx                # 比较esi和edx内容
  401227:	74 14                	je     40123d <fun7+0x39>       # edx == esi, 则返回0
  401229:	48 8b 7f 10          	mov    0x10(%rdi),%rdi          # rdi = rid + 16
  40122d:	e8 d2 ff ff ff       	callq  401204 <fun7>            # 递归调用fun7
  401232:	8d 44 00 01          	lea    0x1(%rax,%rax,1),%eax    # eax = 2*eax + 1
  401236:	eb 05                	jmp    40123d <fun7+0x39>       # 返回
  401238:	b8 ff ff ff ff       	mov    $0xffffffff,%eax
  40123d:	48 83 c4 08          	add    $0x8,%rsp
  401241:	c3                   	retq

# 开始时调用: edi > esi(输入)
#  edi  0x6030f0, *(edi) = 0x24, 此时esi 应该小于0x24
#  rdi = rdi + 0x8, rdi = 0x6030f8
# 第二次递归调用: edi值为 0x6030f8
# x/a 0x6030f8: 0x603110
# x/a 0x603110: 0x8
# 说明rsi应该小于0x24并且大于0x8
# 第三次调用: 0x603110 + 0x10: 0x603150
# x/a 0x603150: 0x16
# 因此最终返回0x16 = 22
# 说明最终输入的值应该是
22
```

