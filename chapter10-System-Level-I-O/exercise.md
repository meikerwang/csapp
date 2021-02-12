# Exercise

## 10.1 打印文件描述符fd

Unix进程生命周期开始时, 已经都有了三个打开的文件, 标准输入(描述符fd=0), 标准输出(描述符fd=1), 标准错误输出(描述符fd=2)

`int open(char *filename, int flags, mode_t mode)` 总是返回一个当前没有打开的最小描述符

第一次调用返回3, 然后 `int close(int fd)` 函数会释放这个文件描述符, 最后调用open会返回描述符3, 因此程序输出是fd=3