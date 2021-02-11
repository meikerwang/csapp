//
// Created by wangzhi on 2021/1/31.
//
#include <assert.h>

int A(int x) {
    // 只有x全部都为1, ~x 等于 0
    return !(~x);
}

int B(int x) {
    // 只有x所有位都是0, 则!x等于1
    return !x;
}

int C(int x) {
    // 只有x的最高有效字节等于1, 此时 x | 0x00FFFFFF才为全1
    return A(x | 0x00FFFFFF);
}

int D(int x) {
    return B(x & 0xFF);
}

int main() {
    int all_bit_one = ~0;

    assert(A(all_bit_one) == 1);
    assert(B(all_bit_one) == 0);
    assert(C(all_bit_one) == 1);
    assert(D(all_bit_one) == 0);

    int all_bit_zero = 0;
    assert(A(all_bit_zero) == 0);
    assert(B(all_bit_zero) == 1);
    assert(C(all_bit_zero) == 0);
    assert(D(all_bit_zero) == 1);

    assert(A(0xFFAA8800) == 0);
    assert(B(0xFFAA8800) == 0);
    assert(C(0xFFAA8800) == 1);
    assert(D(0xFFAA8800) == 1);

    assert(A(0xFFAA8800) == 0);
    assert(B(0xFFAA8800) == 0);
    assert(C(0xFFAA8800) == 1);
    assert(D(0xFFAA8800) == 1);

}