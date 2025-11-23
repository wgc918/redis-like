# Redis-like 键值存储系统项目文档

## 项目概述

### 项目简介

一个基于C++实现的轻量级Redis-like键值存储系统，支持网络通信、多种数据结构和RESP协议。

### 核心特性

- ✅ 支持字符串(String)和有序集合(ZSet)数据结构
- ✅ 自定义RESP协议解析和序列化
- ✅ 多客户端连接支持(poll模型)
- ✅ 渐进式哈希重哈希和AVL树索引
- ✅ 完整的日志系统
- ✅ 命令分发器模式

## 系统架构

### 整体架构图

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Client        │    │   Network Layer  │    │   Protocol      │
│                 │───▶│   Server         │───▶│   Parser        │
└─────────────────┘    │   Connection     │    │   Serializer    │
                       └──────────────────┘    └─────────────────┘
                               │                        │
                               ▼                        ▼
                       ┌──────────────────┐    ┌─────────────────┐
                       │   Command        │    │   Data          │
                       │   Dispatcher     │───▶│   Structures    │
                       │   Registry       │    │   HashTable     │
                       └──────────────────┘    │   AVL Tree      │
                                               │   String/ZSet   │
                                               └─────────────────┘
```

### 核心模块说明

#### 1. 网络层 (Network)

- **Server**: 主服务器，处理连接和事件循环
- **Connection**: 客户端连接管理，读写缓冲区
- **技术**: poll多路复用，非阻塞IO

#### 2. 协议层 (Protocol)  

- **Parser**: RESP协议解析器
- **Serializer**: 响应序列化器
- **支持类型**: 简单字符串、错误、整数、批量字符串、数组

#### 3. 命令层 (Command)

- **CommandDispatcher**: 命令分发和执行
- **CommandRegistry**: 命令注册表
- **支持命令**: GET/SET/DEL/ZADD/ZREM/ZSCORE/ZRANK/ZCARD/ZRANGE/ZALL

#### 4. 数据结构层 (Data Structures)

- **HashTable**: 渐进式重哈希哈希表
- **AVLTree**: 自平衡二叉搜索树
- **String**: 字符串键值对
- **ZSet**: 有序集合实现

#### 5. 工具层 (Utils)

- **Logger**: 分级日志系统
- **BufferPool**: 缓冲区管理

## 详细目录结构

### 核心模块

```
src/
├── command/           # 命令系统
│   ├── command_dispatcher.cpp/h  # 命令分发器
│   ├── commands.h                # 命令定义和注册表
│   └── [命令处理器实现]
├── data_structures/   # 数据结构
│   ├── hashTable.cpp/h          # 哈希表(渐进式重哈希)
│   ├── avl.cpp/h                # AVL平衡树
│   ├── string.cpp/h             # 字符串类型
│   ├── zset.cpp/h               # 有序集合类型
│   └── global/globals.h         # 全局数据
├── network/           # 网络层
│   ├── server.cpp/h             # 服务器主循环
│   └── connection.cpp/h         # 连接管理
├── protocol/          # 协议处理
│   ├── parser.cpp/h             # RESP解析
│   └── serializer.cpp/h         # 响应序列化
└── utils/             # 工具类
    ├── logger/                   # 日志系统
    └── buffer/                   # 缓冲区池
```

##  核心实现细节

### 1. 渐进式哈希表 (HMap)

```cpp
class HMap {
private:
    HTab newTab;        // 新哈希表
    HTab oldTab;        // 旧哈希表  
    uint32_t migrate_pos; // 迁移位置
    // 每次迁移128个节点，避免阻塞
};
```

### 2. 有序集合 (ZSet)

```cpp
struct Value {
    HMap hmap;      // 哈希表: O(1)查找
    AVLTree tree;   // AVL树: O(logN)范围查询
};
// 支持: ZADD, ZREM, ZSCORE, ZRANK, ZRANGE等命令
```

### 3. 命令分发器

```cpp
class CommandDispatcher {
    Response execute_command(const vector<string>& args);
    // 支持: 参数验证、错误处理、命令路由
};
```

### 4. RESP协议支持

```
# 请求: *3\r\n$3\r\nSET\r\n$5\r\nmykey\r\n$7\r\nmyvalue\r\n
# 响应: +OK\r\n 或 -Error message\r\n 或 $5\r\nhello\r\n
```

## 编译和运行

### 环境要求

- C++17 兼容编译器
- Linux/Unix 环境
- CMake 3.10+

### 编译命令

```bash
mkdir build && cd build
cmake ..
make -j4
```

### 运行服务

```bash
./test
```

### 客户端测试

```bash
# 使用redis-cli或telnet测试
telnet localhost 6379

# 示例命令
SET mykey "Hello"
GET mykey
ZADD myset 1 "member1"
ZRANGE myset 0 -1
```

## 性能特点

### 时间复杂度

| 操作     | 字符串 | 有序集合    |
| -------- | ------ | ----------- |
| 插入     | O(1)   | O(logN)     |
| 查找     | O(1)   | O(logN)     |
| 删除     | O(1)   | O(logN)     |
| 范围查询 | -      | O(logN + M) |

### 内存管理

- 渐进式重哈希避免服务停顿
- 连接缓冲区池化

## 顶级哈希表内存示意图

### 整体内存布局

```
┌─────────────────────────────────────────────────────────────────┐
│                      顶级哈希表 (HMap_string)                     │
├─────────────────────────────────────────────────────────────────┤
│ 槽位数组 (HNode** tab)                                            │
│ 索引: 0      1      2      3      4      5      6      7         │
│     ┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐           │
│     │  ●  │  ●  │  ●  │  ●  │  ●  │  ●  │  ●  │  ●  │            │
│     └─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘           │
│       │      │      │      │      │      │      │               │
│       ▼      ▼      ▼      ▼      ▼      ▼      ▼               │
│      NULL  链表头  链表头  NULL  链表头  NULL  NULL  链表头         │
│               │         │         │                  │          │
│               ▼         ▼         ▼                  ▼          │
│           Entry_str  ZsetNode  Entry_str         ZsetNode       │
│           │         │         │                  │              │
│           ▼         ▼         ▼                  ▼              │
│           ...       ...       ...                ...            │
└─────────────────────────────────────────────────────────────────┘
```

## 协议流程总结

**客户端发送**：

text

```
[4字节总长度][4字节命令数量][4字节cmd1长度][cmd1数据][4字节cmd2长度][cmd2数据]...
```

**服务器处理**：

1. 读取4字节长度头
2. 读取指定长度的数据体
3. 解析命令，执行后生成 RESP 格式响应
4. 发送：`[4字节RESP数据长度][RESP数据]`

**客户端接收**：

1. 读取4字节长度头
2. 读取指定长度的 RESP 数据
3. 用 RESP 反序列化器解析数据

##   钩子(HNode)与实际数据结构的关系

### 1. 字符串条目 (Entry_str) 内存布局

```
                          container_of 宏反向定位
                               ▲
                               │
┌─────────────────────────────────────────────────────────────┐
│                    Entry_str 完整对象                        │
├──────────────┬──────────────┬───────────────────────────────┤
│ std::string  │ std::string  │          HNode                │
│    key       │   value      │                               │
│              │              │ ┌───────────────────────────┐ │
│   "name"     │  "John"      │ │ HNode* next = nullptr     │ │
│              │              │ │ uint64_t hcode = 0x3A7F   │ │
│ (32字节)     │  (32字节)     │ │ (对齐填充)                  │ │
│              │              │ └───────────────────────────┘ │
│              │              │         (24字节)              │
└──────────────┴──────────────┴───────────────────────────────┘
       ↑               ↑                      ↑
   key数据区       value数据区            哈希表钩子
  (堆分配)         (堆分配)              (嵌入在结构中)
```

### 2. 有序集合节点 (ZsetNode) 内存布局

```
                               container_of 宏反向定位
                                    ▲
                                    │
┌─────────────────────────────────────────────────────────────────┐
│                      ZsetNode 完整对象                          │
├──────────────┬──────────────┬───────────────────────────────────┤
│ std::string  │  Value*      │             HNode                 │
│     key      │   value      │                                   │
│              │              │ ┌───────────────────────────────┐ │
│   "scores"   │   0x7F8A4B   │ │ HNode* next = nullptr         │ │
│              │              │ │ uint64_t hcode = 0x5B2E       │ │
│ (32字节)      │   (8字节)     │ │ (对齐填充)                     │ │
│              │              │ └───────────────────────────────┘ │
│              │              │           (24字节)                 │
└──────────────┴──────────────┴───────────────────────────────────┘
       ↑                  ↑                         ↑
   key数据区       指向Value对象的指针          哈希表钩子
  (堆分配)        (管理有序集合内部数据)
```

### 3. 完整的哈希链表示例

```
槽位2: ● → [HNode] → [HNode] → NULL
          │         │
          ▼         ▼
      Entry_str  ZsetNode
        │         │  
        ▼         ▼
      "user:1"  "leaderboard"
        │         │
        ▼         ▼
      "Alice"    Value*(0x7F8A4B)
                  │
                  ▼
             ┌───────────────────┐
             │      Value        │
             ├───────────────────┤
             │ HMap  hmap        │ → 内部哈希表 (管理集合元素)
             │ AVLTree tree      │ → AVL树根节点
             └───────────────────┘
```

##  container_of 宏的工作原理

### 宏定义解析

```cpp
#define container_of(ptr, T, member) \
    reinterpret_cast<T*>(reinterpret_cast<char*>(ptr) - offsetof(T, member))

// 使用示例:
HNode *node = // 从哈希表中获取的节点指针
Entry_str *entry = container_of(node, Entry_str, node);
```

### 内存计算过程

```
假设:
Entry_str对象地址: 0x1000
node成员偏移: offsetof(Entry_str, node) = 64字节
HNode节点地址: 0x1040

container_of计算:
1. char* ptr = reinterpret_cast<char*>(0x1040) 
2. 减去偏移: 0x1040 - 64 = 0x1000
3. 转回Entry_str*: reinterpret_cast<Entry_str*>(0x1000)

得到完整的Entry_str对象地址!
```

##  具体内存示例

### 场景：存储3个数据后的内存状态

```
顶级哈希表 HMap_string (初始大小8槽位)

槽位0: NULL
槽位1: ● → [HNode|hcode=0x3A7F|next=0x0] → Entry_str{key="name", value="John"}
槽位2: NULL  
槽位3: ● → [HNode|hcode=0x5B2E|next=0x0] → ZsetNode{key="scores", value=0x7F8A4B}
槽位4: ● → [HNode|hcode=0x8C91|next=0x0] → Entry_str{key="age", value="25"}
槽位5: NULL
槽位6: NULL
槽位7: NULL
```

### 有序集合内部结构展开

```
ZsetNode "scores" (0x7F8A4B) 指向的Value对象:

Value @0x7F8A4B:
  HMap hmap:    // 内部哈希表，管理集合元素
    newTab: [槽位数组] → Entry_zset链表
    oldTab: NULL
    
  AVLTree tree: // AVL树，按分数排序
    root: → AVLNode{score=95.0, name="Alice"}
           ├─ left: → AVLNode{score=87.5, name="Bob"} 
           └─ right: → AVLNode{score=98.0, name="Carol"}
```

## 哈希表操作的内存影响

### 1. 插入操作内存分配

```cpp
// SET newkey "value" 的内存变化:
1. 堆分配: Entry_str对象 (~88字节)
2. 计算哈希: hash("newkey") → 槽位索引
3. 链表插入: 在对应槽位头部插入HNode
4. 更新计数: HMap.size++

内存增长: ~88字节 + key/value数据长度
```

### 2. 查找操作内存访问模式

```cpp
// GET name 的访问路径:
1. 计算哈希: hash("name") → 槽位索引(比如1)
2. 遍历链表: 比较每个HNode的hcode和key
3. container_of: 从HNode获取完整Entry_str
4. 返回值: entry->value

内存访问: 1次随机访问(槽位) + N次顺序访问(链表)
```

### 3. 渐进式重哈希内存变化

```
重哈希前:
旧表: 8槽位, 80字节基础 + 64字节槽位数组 = 144字节

开始重哈希:
新表: 16槽位, 16字节基础 + 128字节槽位数组 = 144字节  
旧表: 保持原样
总内存: 288字节 (2倍增长)

重哈希完成:
释放旧表: 144字节
总内存: 144字节 (回到正常)
```

##  设计模式应用

### 1. 命令模式

```cpp
// CommandDispatcher + CommandHandler
using CommandHandler = function<Response(const vector<string>&)>;
```

### 2. 工厂模式  

```cpp
// CommandRegistry自动注册命令
registry_["GET"] = Command("GET", CommandType::GET, 2, 2, handle_get);
```

### 3. 组合模式

```cpp
// ZSet组合HashTable和AVLTree
struct Value {
    HMap hmap;    // 快速查找
    AVLTree tree; // 有序遍历
};
```

## 扩展指南

### 添加新命令

1. 在`commands.h`中定义命令类型
2. 在`command_dispatcher.cpp`中实现处理器
3. 在构造函数中注册命令

### 添加新数据结构

1. 继承`HNode`嵌入哈希节点
2. 实现对应的比较函数
3. 在全局哈希表中管理

##  待优化项目

- [ ] 持久化支持 (RDB/AOF)
- [ ] 集群模式
- [ ] 事务支持
- [ ] 发布订阅
- [ ] LRU缓存淘汰
- [ ] 性能基准测试
- [ ] 顶级哈希表存储对象的封装
- [ ] 应用程序接口（c++连接器）

## 相关资源

### 参考协议

- [RESP协议规范](https://redis.io/docs/reference/protocol-spec/)

---

*文档版本: 1.0 | 最后更新: 2024年11月*

