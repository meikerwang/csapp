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
//int union_two_word(int x, int y) {
//    return (x & 0xff) | (y & (~0xff));
//}
//
//int main() {
//    int x = 0x89ABCDEF;
//    int y = 0x76543210;
//    int z = union_two_word(x, y);
//    assert(z == 0x765432EF);
//    show_int(z);
//}
