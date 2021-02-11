//
// Created by wangzhi on 2021/2/5.
//

#include <assert.h>

int int_shifts_are_logical() {
    // x 是全1的二进制bit
    int x = -1;
    // 如果是算术右移, 则x右移之后值不变, 和x异或之后结果为0
    int t = (x >> 1) ^x;
    // 算术右移t==0, 应该返回0, 逻辑右移应该返回1
    return t != 0;
}

int main() {
    int result = int_shifts_are_logical();
    assert(result == 0);
}