[toc]

## homework


#### 2.55 显示虚存中的字节

show_bytes.c

``` shell
# 对于 int test_num = 0b000100100011010001010110;
# show_int/show_long/show_longlong/show_float/show_pointer的结果如下:
$ gcc -m32 show_bytes.c -o show_bytes && ./show_bytes
56 34 12 00
56 34 12 00
56 34 12 00 00 00 00 00
b0 a2 91 49
d0 7e f2 ff
$ gcc -m64 show_bytes.c -o show_bytes && ./show_bytes
56 34 12 00
56 34 12 00 00 00 00 00
56 34 12 00 00 00 00 00
b0 a2 91 49
e8 f9 4b 77 fe 7f 00 00
```
可以知道`5.4.0-52-generic 18.04.1-Ubuntu SMP` 是小端系统, 低字节在最低位, 以低字节开始输出
- 32位时一个word是4个字节, long和地址长度都是一个字长也就是4个字节
- 32位时一个word是8个字节, long和地址长度都是一个字长也就是8个字节
- long long是8个字节, 和系统位数无关


#### 2.58 判断是否是小端系统
is_little_endian.c

```c
typedef unsigned char *byte_pointer;

int is_little_endian() {
    int test_num = 0xff;
    byte_pointer byte_start = (byte_pointer) &test_num;

    if (byte_start[0] == 0xff) {
        return 1;
    }
    return 0;
}
```

对于数0xff, 在小端系统上最低位应该确定是0xff, 而大端系统最低位是0x00

> char vs unsigned char:
> char的最高位为符号位，因此char能表示的数据范围是-128~127，unsigned char没有符号位，因此能表示的数据范围是0~255
> 在赋值操作给int或者其他更大类型时, uc进行0扩展, 而char进行符号扩展


#### 2.59 合并两个字

union_two_word.c

```c
int res = (x & 0xFF) | (y & (~0xFF));
```
通过位操作实现
获取x的最低位, x & 0xFF, 获取y的其他位, y & 0xFFFFFF00, 然后两者按位或



#### 2.60 字节替换

replace_byte.c

```c
unsigned mask1 = 0xFF << (i * (1 << 3));
unsigned mask2 = b << (i * (1 << 3));
unsigned result = x & (~mask1) | mask2;
```
将0xFF左移 i * 8 位, 将x的该字节置位0, 然后将 b 左移 i * 8位, 置成b


#### 2. 61 生成满足条件的1

generate_abcd.c

```c
// 任何位都为1
int A(int x) {
    return !(~x);
}

// 任何位都为0
int B(int x) {
    return !x;
}

// 最高有效字节的所有位都等于1
int C(int x) {
    return A(x | 0x00FFFFFF);
}

// 最低有效字节的所谓位都等于0
int D(int x) {
    return B(x & 0xFF);
}
```