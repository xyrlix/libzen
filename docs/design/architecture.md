# libzen 架构设计文档

## 概述

libzen 是一个从零开始搭建的 C++ 基础库，旨在提供高效、可靠的核心功能组件。本文档描述 libzen 的架构设计原则和核心模块的设计思路。

## 设计原则

### 1. 零外部依赖

libzen 不依赖任何第三方库，所有功能从零实现。这确保了：
- 完全掌控代码质量和性能
- 避免版本冲突和兼容性问题
- 便于学习和理解底层原理

### 2. 高性能优先

所有算法和数据结构都经过精心优化：
- 时间复杂度最优
- 空间效率高
- 避免不必要的内存分配
- 利用现代 CPU 特性

### 3. 模块化设计

清晰的模块划分，低耦合高内聚：
- 每个模块职责单一
- 模块间依赖关系清晰
- 便于独立开发和测试
- 易于扩展和维护

### 4. 现代化 C++17

充分利用 C++17 特性：
- 模板和元编程
- RAII 资源管理
- 移动语义
- 完美转发

### 5. 跨平台兼容

虽然主要针对 Linux，但代码结构考虑了跨平台兼容性：
- 平台相关代码隔离
- 统一的接口抽象
- 便于扩展到其他平台

---

## 模块架构

### 依赖层次

```
┌─────────────────────────────────────────────────────┐
│                  应用层                               │
├─────────────────────────────────────────────────────┤
│  http  │  rpc  │  database  │  storage  │  crypto  │
├─────────────────────────────────────────────────────┤
│     net  │  event  │  pool  │  serialize  │  proto  │
├─────────────────────────────────────────────────────┤
│  filesystem  │  buffer  │  timer  │  process  │  system│
├─────────────────────────────────────────────────────┤
│    logging  │  io  │  fmt  │  error  │  type  │  math│
├─────────────────────────────────────────────────────┤
│  threading  │  containers  │  algorithms  │  iterators│
├─────────────────────────────────────────────────────┤
│     memory  │  utility  │  base (type_traits)       │
└─────────────────────────────────────────────────────┘
```

### 核心模块说明

#### 1. 基础层

**base/** - 基础类型工具
- `type_traits.h` - 类型萃取
- `compile_tools.h` - 编译期工具

**memory/** - 内存管理
- `unique_ptr.h` - 独占指针
- `shared_ptr.h` - 共享指针
- `weak_ptr.h` - 弱引用指针
- `allocator.h` - 内存分配器

**utility/** - 通用工具
- `pair.h` - 键值对
- `tuple.h` - 元组
- `optional.h` - 可选值
- `function.h` - 函数包装器

**iterators/** - 迭代器体系
- `iterator_base.h` - 迭代器基类
- `reverse_iterator.h` - 反向迭代器

---

#### 2. 容器和算法层

**containers/** - 容器
- `vector.h` - 动态数组
- `list.h` - 双向链表
- `map.h` - 红黑树映射
- `unordered_map.h` - 哈希表映射

**algorithms/** - 算法
- `sort.h` - 排序算法
- `find.h` - 查找算法
- `numeric.h` - 数值算法
- `transform.h` - 转换算法

---

#### 3. 并发和事件层

**threading/** - 并发编程
- `thread.h` - 线程封装
- `pool/thread_pool.h` - 线程池
- `future/future.h` - 异步结果
- `sync/mutex.h` - 互斥锁

**event/** - 事件驱动
- `event_loop.h` - 统一事件循环（epoll/timer/signal）

---

#### 4. 系统层

**filesystem/** - 文件系统
- `path.h` - 路径操作
- `directory.h` - 目录遍历

**buffer/** - 缓冲区
- `dynamic_buffer.h` - 动态缓冲区
- `ring_buffer.h` - 环形缓冲区

**system/** - 系统工具
- `system_info.h` - 系统信息
- `system_utils.h` - 系统工具函数

**process/** - 进程管理
- `process.h` - 进程封装
- `daemon.h` - 守护进程

**timer/** - 定时器
- `timer_queue.h` - 定时器队列
- `timer_manager.h` - 定时器管理器

---

#### 5. 网络层

**net/** - 网络编程
- `core/socket.h` - Socket 封装
- `core/epoll.h` - Epoll 封装
- `reactor/reactor.h` - Reactor 模式
- `tcp/tcp_server.h` - TCP 服务器
- `tcp/tcp_client.h` - TCP 客户端

---

#### 6. 应用层模块

**config/** - 配置解析
- `ini_config.h` - INI 配置
- `json_config.h` - JSON 配置

**crypto/** - 加密与哈希
- `hash.h` - 哈希算法
- `symmetric.h` - 对称加密
- `asymmetric.h` - 非对称加密

**pool/** - 池化技术
- `connection_pool.h` - 连接池
- `object_pool.h` - 对象池

**serialize/** - 序列化
- `binary_serialize.h` - 二进制序列化
- `json_serialize.h` - JSON 序列化

**proto/** - Protobuf 兼容
- `codec.h` - 编解码
- `message.h` - 消息定义

**http/** - HTTP 支持
- `http_client.h` - HTTP 客户端
- `http_server.h` - HTTP 服务器

**rpc/** - 远程过程调用
- `rpc_server.h` - RPC 服务器
- `rpc_client.h` - RPC 客户端

**database/** - 数据库接口
- `connection.h` - 数据库连接
- `connection_pool.h` - 连接池

**storage/** - 存储系统
- `storage.h` - 文件/键值/对象存储

**image/** - 图像处理
- `image.h` - 图像加载/处理

**audio/** - 音频处理
- `audio.h` - 音频编解码

**video/** - 视频处理
- `video.h` - 视频编解码

**blockchain/** - 区块链
- `blockchain.h` - 区块链基础

**monitoring/** - 系统监控
- `monitoring.h` - 指标/性能分析

**graphics/** - 图形处理
- `graphics.h` - 2D 图形

---

## 核心设计决策

### 1. 智能指针设计

**决策**：从零实现 `unique_ptr`/`shared_ptr`/`weak_ptr`

**原因**：
- 深入理解 RAII 和资源管理
- 避免依赖 `<memory>` 头文件
- 定制化内存管理策略

**关键点**：
- `unique_ptr` 独占所有权，可移动
- `shared_ptr` 使用引用计数，线程安全
- `weak_ptr` 避免循环引用

---

### 2. 容器实现

**决策**：从零实现 `vector`/`list`/`map`/`unordered_map`

**原因**：
- 掌握数据结构的底层实现
- 优化内存布局和缓存友好性
- 定制化容器行为

**关键点**：
- `vector` - 动态扩容，指数增长策略
- `list` - 哨兵节点，双向循环链表
- `map` - 红黑树，O(log n) 查找
- `unordered_map` - 开放寻址法，FNV-1a 哈希

---

### 3. Reactor 模式

**决策**：使用 Reactor 模式处理网络事件

**原因**：
- 高并发场景下性能优异
- 单线程处理多连接，避免锁竞争
- 适合 Linux epoll 机制

**关键点**：
- 统一事件循环（epoll + timer + signal）
- 边缘触发（EPOLLET）
- 回调机制，用户友好

---

### 4. 定时器设计

**决策**：使用最小堆管理定时器

**原因**：
- 添加定时器：O(log n)
- 获取最近定时器：O(1)
- 删除定时器：O(log n)

**关键点**：
- 固定大小数组，避免内存分配
- 惰性删除（标记 cancelled）
- 支持一次性定时器和重复定时器

---

### 5. 缓冲区设计

**决策**：实现动态缓冲区和环形缓冲区

**原因**：
- 动态缓冲区：自动扩容，适合未知数据量
- 环形缓冲区：固定容量，适合流式数据

**关键点**：
- 动态缓冲区：指数增长策略（2x）
- 环形缓冲区：容量为 2 的幂次方，使用位运算
- 零拷贝接口：`read_ptr()`/`write_ptr()`

---

### 6. 序列化设计

**决策**：支持二进制和 JSON 序列化

**原因**：
- 二进制：高效，适合网络传输
- JSON：可读，适合配置和调试

**关键点**：
- 模板化接口，支持自定义类型
- 二进制序列化：小端字节序
- 可变长度整数：节省空间

---

## 性能优化策略

### 1. 内存分配优化

- 预分配策略（容器扩容）
- 小对象优化（function）
- 对象池（减少频繁分配）

### 2. 算法优化

- 快速排序：三数取中 + 小数组插入排序
- 哈希表：开放寻址法，线性探测
- 红黑树：自平衡二叉搜索树

### 3. 缓存友好性

- 连续内存布局
- 数组优于链表
- 避免缓存失效

### 4. 并发优化

- 无锁数据结构（atomic）
- 读写分离（shared_mutex）
- 细粒度锁

---

## 测试策略

### 单元测试

每个模块都有对应的单元测试：
- 使用 Google Test 框架
- 覆盖正常和异常情况
- 性能基准测试

### 测试分类

1. **单元测试**：测试单个函数/类
2. **集成测试**：测试模块间交互
3. **压力测试**：测试高并发场景
4. **回归测试**：确保修改不引入 bug

---

## 未来扩展

### 计划中的功能

1. **更多平台支持**
   - Windows（IOCP）
   - macOS（kqueue）

2. **更多算法**
   - 图算法
   - 字符串算法
   - 数值优化算法

3. **更多容器**
   - `deque` - 双端队列
   - `set` - 有序集合
   - `unordered_set` - 无序集合

4. **性能优化**
   - SIMD 优化
   - 多线程算法
   - 内存池优化

---

## 总结

libzen 的架构设计遵循以下原则：
- 简单胜于复杂
- 性能优先
- 模块化
- 可测试性
- 可维护性

通过清晰的层次划分和模块化设计，libzen 提供了一个高效、可靠的 C++ 基础库，适合学习和实际项目使用。
