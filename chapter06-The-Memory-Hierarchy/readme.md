# The Memory Hierarchy

## 1. 主存

**存储器分类**
- RAM: 每个单元读写时间一样, 且与各单元所在位置无关, 地址译码时间相同, 例如内存
- DAM: 直接存取存储器, 直接定位到读写数据块, 在读写数据库时按照顺序进行, 例如磁盘
- AM: 相联存储器, 按内容检索到 存储位置进行读写, 例如快表

**主存结构**
- CPU执行指令时: 取指令, 取数据, 存数据
- 主存和CPU的连接线有三种类型: 数据线, 地址线, 控制线
- 地址译码器输入的是地址, 输出的是地址驱动信号


**SRAM vs DRAM**
- SRAM 每个存储单元由六个晶体管组成, 对电气干扰不敏感, 比DRAM快, 但是价格更贵
- 6管静态NMOS记忆单元, 存在一对正负反馈门电路, 因此不需要一直保持刷新
- DRAM 每个存储单元由一个电容和一个晶体管组成, 每隔一段时间必须刷新一次
- 动态单管记忆单元: 电路原件减少, 功耗小, 集成度高, 但是速度慢, 是破坏性读出(需要读后再生), 需要定时刷新



## 2. 磁盘存储器

**信息存储原理**
- 写数据: 写1: 磁头线圈通正向电流, 写0: 线圈通反向电流. 不同的磁化状态被记录在磁盘表面
- 读数据: 通过感应电压的极性不同, 可以读出确定的0或者1

扇区:
- 每个磁盘有两个面, 由多个磁道组成
- 每个磁面有许多同心圆, 成为磁道, 每个磁道都有编号, 最外面的为0
- 每个磁道划分为若干个扇区 每个扇区的大小是512字节, 近几年扇区变大, 4096字节, 称为4K扇区, 效率更高

磁盘操作流程:
- 所有磁盘同步寻道: 由柱面号控制
- 选择磁头: 有磁头号控制
- 被选中磁头等待扇区到达磁头下方: 有扇区号控制
- 读写该扇区的数据
- T = 平均寻道时间(5ms) + 平均旋转时间(4-6ms) + 数据传输时间(大约0.01ms/扇区)


CPU读磁盘一个扇区的流程
- CPU对磁盘控制器或DMA控制器初始化：读命令、磁盘逻辑块号、主存起始地址、数据块大小, 然后启动磁盘驱动器工作
- 磁盘控制器读相应的扇区, 并按DMA方式把数据送主存对应位置
- 当DMA传送结束, 磁盘控制器向CPU发出“DMA结束中断请求”, 要求CPU进行相应的后处理


**固态硬盘**
- 它并不是一种磁表面存储器, 而是一种使用NAND闪存组成的外部存储系统, 与U盘并没有本质差别, 只是容量更大, 存取性能更好
- 电信号的控制使得固态硬盘的内部传输速率远远高于常规硬盘, SSD是通过标准磁盘接口与I/O总线互连的
- 在SSD中有一个闪存翻译层, 它将来自CPU的逻辑磁盘块读写请求翻译成对底层SSD物理设备的读写控制信号. 因此, 这个闪存翻译层相当于磁盘控制器
- 它用闪存颗粒代替了磁盘作为存储介质, 利用闪存的特点, 以区块写入和抹除的方式进行数据的写入
- 写操作比读操作慢得多. 顺序读比顺序写大致快一倍, 而随机读比随机写大致快10倍.  随机读写延时比硬盘低两个数量级(随机读约为几十微秒, 随机写约为几百微秒)




## 3. 高速缓存

**局部性原理:**
- Temporal Locality: 刚被访问过的单元很可能不久又被访问
- Spatial Locality: 刚被访问过的单元的邻近单元很可能不久被访问

**Cache:**
- Cache是一种小容量高速缓冲存储器, 它由SRAM组成


**写策略**
- 当多个设备都允许访问主存, 并且每个CPU都带有各自的Cache而共享主存
- 处理Cache读比Cache写更容易, 故指令Cache比数据Cache容易设计
- 对于写命中(Write Hit): 要写的单元已经在Cache中
  - Write Through 直写
    - 同时写Cache和主存单元, 使用写缓冲
  - Write Back 回写
    - 只写cache不写主存, 缺失时一次写回, 每行有个修改位(“dirtybit-脏位”), 大大降低主存带宽需求, 控制可能很复杂
- 对于写不命中(Write Miss): 要写的单元不在Cache中
  - Write Allocate写分配
    - 将主存块装入Cache, 然后更新相应单元
    - 试图利用空间局部性, 但每次都要从主存读一个块
  - Not Write Allocate 非写分配
    - 直接写主存单元, 不把主存块装入到Cache

**Intel Core i7处理器**
- 一个CPU有4个核心Core0 - Core3
- 每个Core有 自己的Regs, L1 d-cache, L1 i-cache, L2 cache
- 所有的4个核心共享L3 Cache
- i-cache和d-cache都是32KB、8路、4个时钟周期; L2 cache：256KB、8路、11个时钟周期. 所有核共享的L3 cache：8MB、16路、30~40个时钟周期. Core i7中所有cache的块大小都是64B

