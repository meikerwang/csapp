### Data Lab


#### 环境安装

dlc 是 64 位 Linux 程序，使用 ./dlc bits.c 来检查是否符合限制条件。但 Makefile 里却指定了 -m32，可能是考虑到 C 标准只规定 int 至少为 16 位，一些环境可能会生成非 32 位的 int

```shell
# 后两个库用于在 64 位环境下编译 32 位程序
sudo apt install build-essential gcc-multilib g++-multilib

# 编译并且运行测试程序
make clean && make btest && ./btest
```