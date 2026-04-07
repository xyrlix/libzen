# libzen 项目结构分析与开发计划

## 一、项目概述

libzen 是一个从零开始搭建的 C++ 基础库，旨在提供高效、可靠的核心功能组件。**关键要求：不要直接使用系统库，要手撕算法手动实现，算法简洁有说明，模块化。**

## 二、当前项目结构分析

### 2.1 目录结构

```
libzen/
├── include/zen/          # 对外暴露的头文件(仅声明)
├── src/                 # 所有实现代码
│   ├── base/           # 基础宏、类型 traits
│   ├── memory/         # 内存分配与智能指针
│   ├── containers/     # 核心容器实现
│   ├── iterators/      # 迭代器体系
│   ├── algorithms/     # 泛型算法
│   ├── utility/        # 通用工具类
│   ├── fmt/            # 格式化与打印
│   ├── threading/      # 并发编程
│   ├── io/             # 异步IO和文件IO
│   ├── logging/        # 日志系统
│   ├── config/         # 配置解析
│   ├── filesystem/     # 文件系统操作
│   ├── net/            # 网络编程
│   ├── timer/          # 定时器框架
│   └── ... (其他20+个模块)
├── tests/              # 单元测试
├── examples/           # 使用示例
└── docs/               # 文档
```

### 2.2 设计原则

1. **接口与实现分离**: `include/zen/` 仅存放对外接口，`src/` 存放实现
2. **模块化设计**: 按功能域拆分子目录
3. **低耦合依赖**: 严格控制模块间依赖，避免循环依赖
4. **工程化配套**: 完善的 CMake、测试、示例、文档

### 2.3 当前实现状态

根据代码分析，大部分模块**只有框架代码，实际实现为空**：

```cpp
// 大部分头文件都只有空的命名空间声明
#ifndef ZEN_MEMORY_UNIQUE_PTR_H
#define ZEN_MEMORY_UNIQUE_PTR_H

namespace zen {
// Unique pointer implementation - 实际上是空的
} // namespace zen

#endif
```

**只有极少数模块有实际实现**（如 base/macros.h）。

## 三、开发优先级与模块依赖链

### 3.1 依赖关系图

```
base (最底层)
  ↓
utility → memory → iterators
  ↓                    ↓
math, type, error  containers
  ↓                    ↓
fmt, threading    algorithms
  ↓                    ↓
timer, event, io, filesystem
  ↓
buffer, system, config, logging
  ↓
net, crypto, serialize, proto
  ↓
process, pool, database, storage
  ↓
http, rpc, image, audio, video, blockchain
```

### 3.2 开发阶段划分

#### 第一阶段：基础模块 (核心基础，必须优先实现)
- ✅ base (基础宏、类型 traits)
- ✅ utility (通用工具类：pair, tuple, optional, variant)
- ✅ memory (智能指针、内存池、分配器)

#### 第二阶段：核心容器与算法 (C++基础库的核心)
- ✅ iterators (迭代器基础和适配器)
- ✅ containers (vector, list, string, map, unordered_map)
- ✅ algorithms (排序、查找、转换等算法)

#### 第三阶段：基础工具模块 (支撑上层功能)
- ✅ math (数学运算、随机数)
- ✅ type (类型系统增强)
- ✅ error (错误处理)
- ✅ fmt (格式化和打印)

#### 第四阶段：并发与系统模块 (工业级开发必备)
- ✅ threading (线程、同步原语、线程池)
- ✅ timer (定时器)
- ✅ event (事件驱动框架)
- ✅ io (异步IO)
- ✅ filesystem (文件系统)

#### 第五阶段：高级工具模块 (完善功能)
- ✅ buffer (高性能缓冲区)
- ✅ system (系统工具)
- ✅ config (配置解析)
- ✅ logging (日志系统)

#### 第六阶段：网络与安全模块 (网络编程)
- ✅ net (网络编程)
- ✅ crypto (加密哈希)
- ✅ serialize (序列化)
- ✅ proto (结构化序列化)

#### 第七阶段：应用与扩展模块 (高级功能)
- process, pool, database, storage, http, rpc
- image, audio, video
- blockchain, monitoring, graphics

## 四、开发策略与方法

### 4.1 代码实现原则

#### 核心要求：手撕算法，不使用系统库
1. **算法自主实现**: 所有算法都要从零实现，不能调用 std::sort、std::vector 等
2. **内存管理**: 自己实现内存分配器、智能指针
3. **容器实现**: 手动实现 vector、map、list 等数据结构
4. **网络编程**: 直接使用系统调用封装，不使用第三方库

#### 代码质量要求
1. **简洁清晰**: 每个算法都要有清晰的注释和说明
2. **模块化**: 功能解耦，职责单一
3. **可读性**: 变量命名清晰，逻辑分层
4. **可测试性**: 每个模块都要有对应的单元测试

### 4.2 具体实现方法

#### 示例：vector 的实现

```cpp
// 手撕 vector，不使用 std::vector
template<typename T>
class vector {
private:
    T* data_;           // 数据指针
    size_t size_;        // 元素个数
    size_t capacity_;   // 容量

    // 手动实现扩容策略
    void reserve(size_t new_capacity) {
        if (new_capacity <= capacity_) return;

        // 手动分配新内存
        T* new_data = static_cast<T*>(::operator new(new_capacity * sizeof(T)));

        // 手动移动元素
        for (size_t i = 0; i < size_; ++i) {
            new (new_data + i) T(std::move(data_[i]));
            (data_ + i)->~T();  // 调用析构函数
        }

        // 释放旧内存
        ::operator delete(data_);

        data_ = new_data;
        capacity_ = new_capacity;
    }

public:
    // 实现标准接口
    vector() : data_(nullptr), size_(0), capacity_(0) {}

    void push_back(const T& value) {
        if (size_ >= capacity_) {
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);  // 2倍扩容
        }
        new (data_ + size_) T(value);
        ++size_;
    }

    // ... 其他方法
};
```

#### 示例：排序算法的实现

```cpp
// 手撕快速排序，不使用 std::sort
template<typename T, typename Compare>
void quick_sort(T* arr, int left, int right, Compare comp) {
    if (left >= right) return;

    // 选择基准值
    T pivot = arr[(left + right) / 2];

    int i = left, j = right;
    while (i <= j) {
        while (comp(arr[i], pivot)) ++i;
        while (comp(pivot, arr[j])) --j;
        if (i <= j) {
            std::swap(arr[i], arr[j]);
            ++i;
            --j;
        }
    }

    // 递归排序
    quick_sort(arr, left, j, comp);
    quick_sort(arr, i, right, comp);
}
```

### 4.3 文件组织规范

#### 头文件组织 (include/zen/)
```cpp
// 只包含声明和文档注释
#ifndef ZEN_MODULE_CLASS_H
#define ZEN_MODULE_CLASS_H

namespace zen {

template<typename T>
class ClassName {
public:
    // 公开接口

private:
    // 私有成员
};

} // namespace zen

#endif
```

#### 实现文件组织 (src/module/)
```cpp
#include "internal_header.h"

namespace zen {

// 实现细节
// 包含详细的算法注释

} // namespace zen
```

### 4.4 测试驱动开发 (TDD)

每个模块都要有对应的测试：
```
tests/
├── test_base.cpp
├── test_memory.cpp
├── test_containers.cpp
└── ...
```

## 五、具体模块开发指南

### 5.1 Base 模块 (已完成框架)

**需要实现：**
- ✅ 基础宏 (DONE: macros.h)
- ✅ 类型 traits 扩展
- ✅ 编译期工具

**关键点：**
- 提供跨平台宏定义
- 实现类型判断工具
- 提供编译期常量计算

### 5.2 Memory 模块

**需要实现：**
- ✅ unique_ptr (独占所有权)
- ✅ shared_ptr (共享所有权)
- ✅ weak_ptr (弱引用)
- ✅ 内存池 (高效内存管理)
- ✅ 自定义分配器

**关键点：**
- 实现引用计数机制
- 处理循环引用问题
- 优化内存分配性能

### 5.3 Containers 模块

**需要实现：**
- ✅ vector (动态数组)
- ✅ list (双向链表)
- ✅ string (字符串)
- ✅ map (红黑树)
- ✅ unordered_map (哈希表)

**关键点：**
- 手写数据结构
- 实现迭代器
- 支持动态扩容
- 时间复杂度优化

### 5.4 Algorithms 模块

**需要实现：**
- ✅ 排序算法 (quick_sort, merge_sort, heap_sort)
- ✅ 查找算法 (binary_search)
- ✅ 转换算法 (transform, for_each)
- ✅ 数值算法 (accumulate)

**关键点：**
- 手撕经典算法
- 注释算法思路
- 时间复杂度分析
- 泛型编程

### 5.5 Threading 模块

**需要实现：**
- ✅ thread 封装
- ✅ mutex (互斥锁)
- ✅ condition_variable (条件变量)
- ✅ 线程池

**关键点：**
- 封装系统 API (pthread)
- 实现线程安全
- 优化线程调度

## 六、下一步行动计划

### 6.1 立即开始 (第1-2周)

**优先级 P0：核心基础模块**
1. 完善 base 模块 (类型 traits、编译期工具)
2. 实现 utility 模块 (pair, tuple, optional)
3. 实现 memory 模块 (unique_ptr, shared_ptr, weak_ptr)
4. 编写对应测试用例

### 6.2 第二阶段 (第3-4周)

**优先级 P0：核心容器与算法**
1. 实现 iterators 模块 (迭代器基础)
2. 实现 containers 模块 (vector, list, string)
3. 实现 algorithms 模块 (排序、查找算法)
4. 编写详细测试和示例

### 6.3 第三阶段 (第5-6周)

**优先级 P1：基础工具模块**
1. 实现 math 模块
2. 实现 type 模块
3. 实现 error 模块
4. 实现 fmt 模块

### 6.4 第四阶段 (第7-10周)

**优先级 P2：并发与系统模块**
1. 实现 threading 模块
2. 实现 timer 模块
3. 实现 event 模块
4. 实现 io 和 filesystem 模块

### 6.5 第五阶段 (第11-12周)

**优先级 P2：高级工具模块**
1. 实现 buffer 模块
2. 实现 system 模块
3. 实现 config 模块
4. 实现 logging 模块

### 6.6 第六阶段 (第13-16周)

**优先级 P3：网络与安全模块**
1. 实现 net 模块
2. 实现 crypto 模块
3. 实现 serialize 模块
4. 实现 proto 模块

## 七、质量保证措施

### 7.1 代码规范
- 遵循 Google C++ Style Guide
- 使用 C++17 特性
- 每个函数都要有 Doxygen 注释
- 变量命名清晰有意义

### 7.2 测试覆盖
- 每个模块都要有单元测试
- 测试覆盖率不低于 80%
- 包含边界测试和异常测试

### 7.3 文档要求
- 每个类和函数都要有详细注释
- 提供使用示例
- 说明算法的时间复杂度
- 说明设计思路

### 7.4 性能优化
- 关注内存分配效率
- 减少不必要的拷贝
- 使用移动语义
- 优化热点路径

## 八、总结

**libzen 项目的核心价值：**
- 从零实现 C++ 基础库，深入理解底层原理
- 手撕经典算法，掌握数据结构和算法
- 模块化设计，学习工程化开发
- 完善的测试和文档，建立良好习惯

**关键成功因素：**
1. 严格按照依赖顺序开发
2. 每个算法都要手撕实现，不依赖系统库
3. 代码简洁清晰，注释详细
4. 模块化设计，低耦合高内聚
5. 测试驱动，保证质量

**预期成果：**
- 一个完整可用的 C++ 基础库
- 深入理解 C++ 语言特性和底层原理
- 掌握数据结构、算法和系统编程
- 具备工业级代码开发能力

---

**开始行动吧！建议从 base 模块开始，逐步实现各个功能。**
