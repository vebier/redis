# Redis-like Key-Value Store

一个基于 C++ 实现的轻量级键值存储服务器，使用嵌入式哈希表和渐进式扩容技术。

## 项目简介

这是一个类 Redis 的键值存储系统，实现了 GET、SET、DEL、KEYS 等基本操作以及有序集合（ZSet）相关操作。项目采用非阻塞 I/O 和事件驱动架构，支持高并发客户端连接，适合作为分布式系统中的内存数据库或缓存组件。

### 核心特性

- **嵌入式哈希表**：使用侵入式设计，HNode 节点嵌入到数据结构中，避免额外内存分配
- **渐进式扩容**：采用双哈希表策略，在后台逐步迁移数据，避免阻塞操作
- **拉链法解决冲突**：使用链表处理哈希冲突
- **非阻塞 I/O**：基于 poll() 的事件循环，支持高并发
- **管道化请求**：支持批量请求处理
- **单例模式**：使用线程安全的单例模式管理数据库实例
- **多种数据结构**：支持字符串和有序集合（ZSet）

## 支持的命令

### 字符串命令

- `GET key`：获取指定键的值
- `SET key value`：设置键值对
- `DEL key`：删除指定的键
- `KEYS`：列出所有键

### 有序集合（ZSet）命令

- `ZADD key score member [score member ...]`：向有序集合添加一个或多个成员，或者更新已存在成员的分数
- `ZREM key member [member ...]`：移除有序集合中的一个或多个成员
- `ZSCORE key member`：返回有序集合中指定成员的分数
- `ZCARD key`：获取有序集合的成员数
- `ZRANGE key start stop [WITHSCORES]`：通过索引区间返回有序集合指定区间内的成员

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



## 构建与运行

### 依赖

- C++20 或更高版本
- Linux 操作系统
- CMake 3.31 或更高版本

### 构建步骤

```bash
# 创建构建目录
mkdir -p cmake-build-debug
cd cmake-build-debug

# 配置并构建
cmake ..
make

# 或者直接在根目录构建
cmake -B build
cmake --build build
```

### 运行服务器

```bash
# 在构建目录中运行
./main
```

服务器将在端口 1234 上启动监听。

### 运行客户端

```bash
# 在构建目录中运行
./client
```

## 性能特点

| 操作 | 平均时间复杂度 | 最坏时间复杂度 | 说明 |
|------|--------------|--------------|------|
| GET  | O(1)         | O(n)         | n 为冲突链表长度 |
| SET  | O(1) + 迁移成本 | O(n)      | 包含渐进式扩容开销 |
| DEL  | O(1) + 迁移成本 | O(n)      | 包含渐进式扩容开销 |
| ZADD | O(log n)     | O(log n)     | AVL树插入 |
| ZREM | O(log n)     | O(log n)     | AVL树删除 |
| ZSCORE | O(1)       | O(1)         | 哈希表查找 |
| ZRANGE | O(log n + m) | O(log n + m) | m为返回元素数量 |

- **空间复杂度**: O(n)
- **负载因子**: 8（可配置）
- **扩容开销**: 分摊到多次操作，单次操作影响小于 0.1ms
- **最大消息大小**: 32MB
- **最大参数数量**: 200,000

## 技术亮点

1. **侵入式设计**
    - 通过 container_of 宏实现零开销的类型转换
    - 节点嵌入数据结构，避免额外指针存储
    - 减少内存碎片和缓存未命中

2. **渐进式扩容**
    - 双哈希表策略，避免长时间阻塞
    - 每次操作最多迁移 128 个节点
    - 适合高并发场景

3. **类型安全**
    - 模板化设计支持任意键值类型
    - 编译期类型检查

4. **事件驱动架构**
    - 基于 poll() 的非阻塞 I/O
    - 支持数千个并发连接
    - 管道化请求处理

5. **多种数据结构**
    - 哈希表：提供O(1)的键值操作
    - AVL树：提供有序集合功能和范围查询
    - 组合设计：结合两种数据结构的优势

## 项目结构

- **include/**: 头文件
    - **core/**: 核心功能模块
    - **network/**: 网络相关功能
    - **protocol/**: 协议解析和请求处理
    - **server/**: 服务器实现
    - **storage/**: 存储引擎实现
- **src/**: 源代码实现
- **main.cpp**: 服务器入口
- **client.cpp**: 客户端实现
