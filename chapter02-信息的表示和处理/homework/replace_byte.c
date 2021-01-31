////
//// Created by wangzhi on 2021/1/31.
////
//#include <stdio.h>
//#include <assert.h>
//
//typedef unsigned char *byte_pointer;
//
//void show_bytes(byte_pointer start, size_t len) {
//    size_t i;
//    for (i = 0; i < len; i++) {
//        printf("%.2x ", start[i]);
//    }
//    printf("\n");
//}
//
//void show_int(int x) {
//    show_bytes((byte_pointer) &x, sizeof(int));
//}
//
//unsigned replace_byte(unsigned x, unsigned char b, int i) {
//    unsigned mask1 = 0xFF << (i * (1 << 3));
//    unsigned mask2 = b << (i * (1 << 3));
//    unsigned result = x & (~mask1) | mask2;
//    return result;
//}
//
//int main() {
//    unsigned r;
//    r = replace_byte(0x12345678, 0xAB, 2);
//    assert(r == 0x12AB5678);
//    show_int(r);
//    r = replace_byte(0x12345678, 0xAB, 0);
//    assert(r == 0x123456AB);
//    show_int(r);
//}
