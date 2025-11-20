# Redis-like Key-Value Store

一个基于 C++ 实现的轻量级键值存储服务器,使用嵌入式哈希表和渐进式扩容技术。

## 项目简介

这是一个类 Redis 的键值存储系统,实现了基本的 GET、SET、DEL 操作。项目采用非阻塞 I/O 和事件驱动架构,支持高并发客户端连接。

### 核心特性

嵌入式哈希表: 使用侵入式设计,HNode 节点嵌入到数据结构中,避免额外内存分配
渐进式扩容: 采用双哈希表策略,在后台逐步迁移数据,避免阻塞操作
拉链法解决冲突: 使用链表处理哈希冲突
非阻塞 I/O: 基于 poll() 的事件循环,支持高并发
管道化请求: 支持批量请求处理

### 架构设计

#### 数据结构层次

```
Entry<Key, Value>
├── Key key          // 键
├── Value val        // 值
└── HNode node       // 哈希节点(嵌入式设计)
    ├── HNode* next  // 链表指针
    └── uint64_t hcode // 哈希值
```

#### 哈希表结构

```
HMap (双表设计)
├── Htab newer       // 当前使用的哈希表
├── Htab older       // 迁移中的旧哈希表
└── size_t migrate_pos // 迁移进度指针
```

```
Htab (单个哈希表)
├── HNode** tab      // 哈希槽位数组
├── size_t mask      // 槽位掩码(容量-1)
└── size_t size      // 当前节点数量
```

#### 内存布局示例

```
哈希表槽位(链表头)

tab[0]: nullptr
tab[1]: Entry1 -> Entry2 -> nullptr
tab[2]: nullptr
tab[3]: Entry3 -> Entry4 -> Entry5 -> nullptr

Entry 对象在堆上分散分配

0x1000: Entry1 {key: "key1", val: "value1", node: {next: 0x3500, hcode: 12345}}
0x3500: Entry2 {key: "key10", val: "value10", node: {next: nullptr, hcode: 54321}}
0x7800: Entry3 {key: "key0", val: "value0", node: {next: 0x9000, hcode: 99999}}
```

### 构建与运行

### 依赖

- C++11 或更高版本
- Linux 操作系统
- CMake 3.10+ (可选)

### 性能特点

操作	平均时间复杂度	最坏时间复杂度		说明
GET	O(1)			    O(n)			n 为冲突链表长度
SET	 O(1) + 迁移成本	O(n)		        包含渐进式扩容开销
DEL	O(1) + 迁移成本	O(n)			包含渐进式扩容开销

- 空间复杂度: O(n)
- 负载因子: 8(可配置)
- 扩容开销: 分摊到多次操作,单次操作影响小于 0.1ms

### 技术亮点

1. 侵入式设计
通过 container_of 宏实现零开销的类型转换
节点嵌入数据结构,避免额外指针存储
减少内存碎片和缓存未命中
2. 渐进式扩容
双哈希表策略,避免长时间阻塞
每次操作最多迁移 128 个节点
适合高并发场景
3. 类型安全
模板化设计支持任意键值类型
编译期类型检查
4. 事件驱动架构
基于 poll() 的非阻塞 I/O
支持数千个并发连接
管道化请求处理