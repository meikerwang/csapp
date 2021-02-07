### DataLab


#### 环境安装

dlc 是 64 位 Linux 程序，使用 ./dlc bits.c 来检查是否符合限制条件。但 Makefile 里却指定了 -m32，可能是考虑到 C 标准只规定 int 至少为 16 位，一些环境可能会生成非 32 位的 int

```shell
# 后两个库用于在 64 位环境下编译 32 位程序
sudo apt install build-essential gcc-multilib g++-multilib

# 编译并且运行测试程序
make clean && make btest && ./btest
```


#### 位运算

1. bitXor(x, y): 只用非运算 ~ 和 与运算& 实现异或, 可以使用~和&或者~和|实现所有运算
```c
int bitXor(int x, int y) {
  return ~(~x & ~y) & ~(x & y);
}
```


2. tmin(): 返回最小的数的补码形式, 补码最小值就是符号位为1，其余全为0,对数值 0x1 进行移位运算，得到结果
```c
int tmin(void) {
  return 0x01 << 31;
}
```


3. isMax(x): 取反加一和自己相同的有TMin, 0, 而TMax取反之后就是TMin, 需要排除特殊值0
```c
int isTmax(int x) {
  x = ~x;
  return !((~x+1) ^ x) & !!(x ^ 0);
}
```


4. allOddBits(x): 移位构造0xAAAAAAAA的mask, 然后获取输入 x 值的奇数位，其他位清零（mask&x），然后与 mask 进行异或操作，若相同则最终结果为0，然后返回其值的逻辑非
```c
int allOddBits(int x) {
  int mask = (0xAA << 24) + (0xAA << 16) + (0xAA << 8) + 0xAA;
  return !((mask & x) ^ mask);
}
```


5. negate(x): 取反加一就行
```c
int negate(int x) {
  return ~x + 1;
}
```


6. isAsciiDigit(x): 判断一个x的范围, x - 0x30应该是非负, 0x39 - x 也应该是非负的
```c
int isAsciiDigit(int x) {
  return !((x + ~0x30 + 1) >> 31) & !((~x + 0x39 + 1) >> 31);
}
```


7. conditional(x,y,z): 根据x是否为0构造出0x00或者0xFFFFFFFF, 然后使用mask进行&
```c
int conditional(int x, int y, int z) {
  int b = !x; // x为0时, b为1, x不为0时, b为0
  b = (b << 31) >> 31; // 此时b为x0FFFF(x为0)或者0x00(x不为0)
  return (y & ~b) | (z & b); // 只有一边为0
}
```


8. isLessOrEqual(x,y): 需要先判断符号, 因为减法会出现溢出操作, 两者同号之后再相减判断符号位: y - x ==> y + ~x + 1
```c
int isLessOrEqual(int x, int y) {
  // 先判断两者符号, x为负数则sign为1, y为正数则sign为0
  int x_sign = x >> 31;
  int y_sign = y >> 31;
  // x负y正, sign 1,0 直接返回1
  // x正y负, sign 0,1 则返回0
  // 两者同号且y比x大 则该差值非负, 移位后为0
  return (x_sign & !y_sign)
    | (!(~x_sign & y_sign) & !((y + ~x +1) >> 31));
}
```


9. logicalNeg(x): 先判断符号位,再和TMax相加比较最高位是否溢出
```c
int logicalNeg(int x) {
  // x为负数是sign为1, 否则为0
  int sign = (x >> 31) & 0x01;
  // 符号位0时, 只有0+TMax才为正数不会溢出, 符号位为0
  int TMax = ~(0x01 << 31);
  return (sign | (((x + TMax) >> 31) & 0x01)) ^ 1;
}
```


10.  howManyBits(x): 利用二分法找到除了最高位以外的最高位. 由于补码的特性, 负数是要找最高位的0, 正数找最高位1
```c
int howManyBits(int x) {
  int b16,b8,b4,b2,b1,b0;
  int sign=x>>31;
  x = (sign&~x) | (~sign&x);//如果x为正则不变，否则按位取反
  // 不断缩小范围
  b16 = !!(x>>16)<<4;//高十六位是否有1
  x = x>>b16;//如果有（至少需要16位），则将原数右移16位
  b8 = !!(x>>8)<<3;//剩余位高8位是否有1
  x = x>>b8;//如果有（至少需要16+8=24位），则右移8位
  b4 = !!(x>>4)<<2;//同理
  x = x>>b4;
  b2 = !!(x>>2)<<1;
  x = x>>b2;
  b1 = !!(x>>1);
  x = x>>b1;
  b0 = x;
  return b16+b8+b4+b2+b1+b0+1;//+1表示加上符号位
}
```



#### floating-point operation

1. floatScale2(uf): 需要判断是否为规格化数据, 非规格化和特殊值需要特别处理
```c
unsigned floatScale2(unsigned uf) {
  int exp = (0x7F800000 & uf) >> 23;
  int sign = uf & (0x1 << 31);
  // 阶码全0, 则直接左移*2, 同时要保证符号位不会变
  if(exp == 0){
    return uf << 1 | sign;
  }
  // 阶码全1, 则表示无穷值, 直接返回
  if(exp == 0xFF){
    return uf;
  }
  // 否则exp加一
  exp = exp + 1;
  // 如果乘2之后变成了无穷大, 则直接返回无穷大的值
  if(exp == 0xFF){
    return 0x7F800000 | sign;
  }
  return (exp << 23) | (uf & 0x807FFFFF);
}
```


2. floatFloat2Int(uf): 将浮点数转换为整数
```c
int floatFloat2Int(unsigned uf) {
  int sign    = uf >> 31;
  int exp  = ((uf & 0x7f800000) >> 23) - 127;
  int frac = (uf & 0x007fffff) | 0x00800000;
  // 当uf为非规格化, 此时uf的阶码和尾数都为0, 符号位1表示-0, 符号位0表示+0, 都返回0
  if(!(uf&0x7fffffff)) return 0;

  // 当真实指数大于31, 即1<<31会覆盖符号位,溢出,返回规定值
  if(exp > 31) return 0x80000000;
  if(exp < 0) return 0;

  // 把小数部分(23位)转换为整数判断是否溢出
  if(exp > 23) frac <<= (exp-23);
  else frac >>= (23-exp);

  if(!((frac >> 31) ^ sign)) return frac;
  else if(frac >> 31) return 0x80000000;
  else return ~frac + 1;
}
```


3. floatPower2: 求2^x次方, 看是否溢出, 在正常范围内则直接返回exp << 23
```c
unsigned floatPower2(int x) {
    int INF = 0xFF << 23;
    int exp = x + 127;
    // 偏移后的指数exp小于0, 则表示结果为0
    if(exp <= 0){
      return 0;
    }
    // 指数大于255, 表示为无穷大或者溢出
    if(exp >= 255){
      return INF;
    }
    return exp << 23;
}
```