# System-Level I/O


## Unix I/O

```c
int open(char *filename, int flags, mode_t mode);

int close(int fd);

// size_t is defined as unsigned int
// ssize_t is defined as int, as it returns -1 when occurred an error, occurred EOF return 0
ssize_t read(int fd, void *buf, size_t n);

ssize_t write(int fd, const void *buf, size_t n);
```

## 读取元数据

```c
#include <unistd.h>
#include <sys/stat.h>

// 传入文件名或者fd, 填入对应的stat结构体
int stat(const char *filename, struct stat *buf);
int fstat(int fd, struct stat *buf);
```

## 共享文件

- descriptor table: 每个进程都有自己的描述符表, 表项由进程打开的文件描述符来索引, 指向一个文件表的表项
- file table: 打开的文件集合由一个文件表, 所有进程共享, 表项包括文件位置, 引用次数, 以及指向v-node表表项的指针
- vnode table: 所有进程共享, 表项包含stat结构体中的大部分信息

- 无共享情况: 描述符表中的两个项指向两个不同的file talbe表项, 然后又分别指向不同的v-node表项
- 多次打开情况: 例如通过两次open打开某个文件进行读, 则存在两个文件描述符, 指向不同的file table表项(分别记录一个文件的不同读位置), 然后这两个表项指针指向同一个v-node表项
  - 例如通过两个Open打开同一个文件, 然后两个fd都读一个字符, 它们读的数据是一样的, 不会相互影响
- 父子进程fork: 子进程会有父进程的描述符表的一个副本, 共享打开文件表集合
  - 然而通过Fork()分别在子进程和父进程中读字符, 然后子进程先读, 父进程读的字符会是第2个字符



## I/O 重定向

```c
// dump2函数拷贝描述符表项oldfd到描述符表项newfd
int dup2(int oldfp, int newfd);
```
调用dup2(4,1), 则两个文件描述符都指向文件B, 标准输出已经关闭(文件表项refcnt=0, v-node也删除了), 而文件B的文件表项refcnt=2, 因此任何写到标准输出的数据都被重定向到文件B

重定向将标准输入定位到描述符5: `dup2(5, 0)`

例如可以打开两次同一个文件, 如果不重定位, 则描述符表项不同, 指向不同的文件表项, v-node也不一样, 因此读的数据不同, 但是如果 `dup(fd2, fd1)`, 则读完fd2, 在读fd1, 则是读的是下一个字符


## 标准I/O

ANSI C定义了标准I/O库, 替代了Unix I/O, libc中提供的打开关闭文件函数(fopen, fclose), 读写字符函数(fread, fwrite), 读写字符串函数(fgets, fputs), 已经复杂格式的I/O函数(scanf, printf)

ANSIC将打开的文件模型化为一个流, 每个程序开始时都有三个打开的流: stdin, stdout, stderr:
```c
#include <stdio.h>
extern FILE *stdin;     // 描述符0
extern FILE *stdout;    // 描述符1
extern FILE *stderr;    // 描述符2
```

Unix I/O在内核中实现, 应用程序通过open,close,lseek,read,write,stat函数来访问Unix I/O

标准I/O函数提供了Unix I/O函数的一个更加完整的带有缓冲的替代品

Unix对网络的抽象是一种套接字的文件类型, 也用文件描述符来引用, 成为套接字描述符

在网络套接字上不要使用标准I/O函数, 而是使用Unix I/O


