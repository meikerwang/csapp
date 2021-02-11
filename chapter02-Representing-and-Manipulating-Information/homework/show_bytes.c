//
// Created by wangzhi on 2021/1/31.
//

/*
 * show-bytes.c
 */

#include <stdio.h>

typedef unsigned char *byte_pointer;

void show_bytes(byte_pointer start, size_t len) {
    size_t i;
    for (i = 0; i < len; i++) {
        printf("%.2x ", start[i]);
    }
    printf("\n");
}

void show_int(int x) {
    show_bytes((byte_pointer) &x, sizeof(int));
}

void show_float(float x) {
    show_bytes((byte_pointer) &x, sizeof(float));
}

void show_long(long x) {
    show_bytes((byte_pointer) &x, sizeof(long));
}

void show_longlong(long long x) {
    show_bytes((byte_pointer) &x, sizeof(long long));
}

void show_pointer(void *x) {
    show_bytes((byte_pointer) &x, sizeof(void *));
}

void test_show_bytes(int val) {
    int ival = val;
    float fval = (float) ival;
    int *pval = &ival;
    long lval = (long) val;
    long long llval = (long long) val;

    show_int(ival);
    show_long(lval);
    show_longlong(llval);
    show_float(fval);
    show_pointer(pval);
}

int main(int argc, char *argv[]) {
    int test_num = 0b000100100011010001010110;

    test_show_bytes(test_num);
    return 0;
}
