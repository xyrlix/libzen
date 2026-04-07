# libzen 开发任务清单

## 📋 总览

本文档提供详细的开发任务清单，按照依赖关系和优先级组织。每个任务都包含具体的要求、实现要点和验证标准。

**核心原则：** 手撕算法，不使用系统库，代码简洁有说明，模块化。

---

## 🔥 第一阶段：基础模块 (P0 - 1-2周)

### Task 1.1: 完善 base 模块

#### 📁 文件结构
```
src/base/
├── macros.h           ✅ 已完成
├── type_traits.h      ⚠️ 待完善
├── compile_tools.h    ⚠️ 待完善
└── CMakeLists.txt     ✅ 已有
```

#### ✅ 实现要求

**1. type_traits.h**
- 实现基础类型判断:
  ```cpp
  template<typename T> struct is_integral;
  template<typename T> struct is_floating_point;
  template<typename T> struct is_pointer;
  template<typename T> struct is_reference;
  template<typename T> struct is_const;
  template<typename T> struct is_volatile;
  template<typename T> struct is_void;
  ```

- 实现类型转换判断:
  ```cpp
  template<typename From, typename To> struct is_convertible;
  template<typename T> struct remove_reference;
  template<typename T> struct remove_cv;
  template<typename T> struct remove_pointer;
  ```

- 实现类型关系判断:
  ```cpp
  template<typename Base, typename Derived> struct is_base_of;
  template<typename T> struct is_same;
  ```

- 实现类型属性:
  ```cpp
  template<typename T> struct is_array;
  template<typename T> struct is_function;
  template<typename T> struct is_class;
  template<typename T> struct is_union;
  ```

**2. compile_tools.h**
- 编译期常量工具:
  ```cpp
  template<bool B, typename T, typename F> struct conditional;
  template<bool B, typename T = void> struct enable_if;
  ```

- 编译期整数序列:
  ```cpp
  template<typename T, T... I> struct integer_sequence;
  template<size_t... I> using index_sequence = integer_sequence<size_t, I...>;
  ```

#### 📝 实现要点
- 使用模板特化实现类型判断
- 使用 SFINAE (Substitution Failure Is Not An Error) 技巧
- 提供编译期常量和类型别名
- 每个trait都要有value成员或type成员

#### ✅ 验证标准
- 所有类型判断trait都能正确编译
- 能通过编译期类型检查
- 有对应的单元测试

---

### Task 1.2: 实现 utility 模块

#### 📁 文件结构
```
src/utility/
├── pair.h             📝 需创建
├── tuple.h            📝 需创建
├── optional.h         📝 需创建
├── variant.h          📝 需创建
├── move.h             📝 需创建
└── CMakeLists.txt     📝 需创建
```

#### ✅ 实现要求

**1. pair.h**
```cpp
template<typename T1, typename T2>
struct pair {
    T1 first;
    T2 second;

    // 构造函数
    constexpr pair();
    constexpr pair(const T1& x, const T2& y);
    constexpr pair(T1&& x, T2&& y);

    // 比较运算符
    bool operator==(const pair& other) const;
    bool operator<(const pair& other) const;
};

// 工厂函数
template<typename T1, typename T2>
pair<T1, T2> make_pair(T1&& x, T2&& y);
```

**2. tuple.h**
- 实现变参模板tuple
- 使用递归继承或递归组合
- 实现get<N>()访问元素
- 实现make_tuple工厂函数

**3. optional.h**
```cpp
template<typename T>
class optional {
public:
    optional() noexcept;
    optional(nullopt_t) noexcept;
    optional(const T& value);
    optional(T&& value);

    explicit operator bool() const noexcept;
    T& operator*();
    T* operator->();

    // 或运算符
    template<typename U>
    optional<T> operator||(optional<U>&& other) const;
};
```

**4. variant.h**
- 实现类型安全的联合体
- 使用递归组合
- 实现visit模式匹配

**5. move.h**
```cpp
// 移动语义和完美转发
template<typename T>
typename remove_reference<T>::type&& move(T&& t) noexcept;

template<typename T>
T&& forward(typename remove_reference<T>::type& t) noexcept;

template<typename T>
T&& forward(typename remove_reference<T>::type&& t) noexcept;

// swap
template<typename T>
void swap(T& a, T& b) noexcept(noexcept(a.swap(b)));
```

#### 📝 实现要点
- 使用模板元编程
- 实现移动语义
- 使用完美转发
- 提供异常安全保证

#### ✅ 验证标准
- 能正确存储和访问元素
- 支持移动语义
- 有对应的单元测试

---

### Task 1.3: 实现 memory 模块

#### 📁 文件结构
```
src/memory/
├── allocator.h        ⚠️ 待完善
├── memory_pool.h      ⚠️ 待完善
├── unique_ptr.h       ⚠️ 待完善
├── shared_ptr.h       ⚠️ 待完善
├── weak_ptr.h         ⚠️ 待完善
└── CMakeLists.txt     ✅ 已有
```

#### ✅ 实现要求

**1. allocator.h**
```cpp
template<typename T>
class allocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    allocator() noexcept = default;
    template<typename U>
    allocator(const allocator<U>&) noexcept;

    T* allocate(size_t n);
    void deallocate(T* p, size_t n);

    template<typename U, typename... Args>
    void construct(U* p, Args&&... args);

    template<typename U>
    void destroy(U* p);
};
```

**2. memory_pool.h**
```cpp
class memory_pool {
private:
    struct Block {
        Block* next;
        // 数据区域紧随其后
    };

    Block* free_list_;
    size_t block_size_;
    size_t pool_size_;

public:
    memory_pool(size_t block_size, size_t initial_pool_size);
    ~memory_pool();

    void* allocate();
    void deallocate(void* p);
};
```

**3. unique_ptr.h**
```cpp
template<typename T, typename Deleter = default_delete<T>>
class unique_ptr {
private:
    T* ptr_;
    Deleter deleter_;

public:
    constexpr unique_ptr() noexcept;
    explicit unique_ptr(T* p) noexcept;
    ~unique_ptr();

    // 禁止拷贝
    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    // 允许移动
    unique_ptr(unique_ptr&& other) noexcept;
    unique_ptr& operator=(unique_ptr&& other) noexcept;

    // 重置
    void reset(T* p = nullptr) noexcept;
    T* release() noexcept;

    // 访问
    T* get() const noexcept;
    T& operator*() const;
    T* operator->() const noexcept;
    explicit operator bool() const noexcept;
};

// 数组特化
template<typename T, typename Deleter>
class unique_ptr<T[], Deleter> { /* ... */ };

// 工厂函数
template<typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args);
```

**4. shared_ptr.h**
```cpp
template<typename T>
class shared_ptr {
private:
    T* ptr_;
    control_block* ctrl_;

    struct control_block {
        atomic<long> ref_count;
        atomic<long> weak_count;
        // 自定义删除器和分配器
    };

public:
    constexpr shared_ptr() noexcept;
    explicit shared_ptr(T* p);
    ~shared_ptr();

    shared_ptr(const shared_ptr& other) noexcept;
    shared_ptr& operator=(const shared_ptr& other) noexcept;

    shared_ptr(shared_ptr&& other) noexcept;
    shared_ptr& operator=(shared_ptr&& other) noexcept;

    long use_count() const noexcept;
    void reset() noexcept;
    void reset(T* p);

    T* get() const noexcept;
    T& operator*() const;
    T* operator->() const noexcept;
    explicit operator bool() const noexcept;
};

template<typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args);
```

**5. weak_ptr.h**
```cpp
template<typename T>
class weak_ptr {
private:
    control_block* ctrl_;

public:
    constexpr weak_ptr() noexcept;
    weak_ptr(const shared_ptr<T>& other) noexcept;
    ~weak_ptr();

    long use_count() const noexcept;
    bool expired() const noexcept;
    shared_ptr<T> lock() const noexcept;
};
```

#### 📝 实现要点
- unique_ptr: 独占所有权，禁止拷贝，允许移动
- shared_ptr: 共享所有权，使用原子操作实现引用计数
- weak_ptr: 解决循环引用问题
- 内存池: 使用链表管理空闲块
- 分配器: 手动管理内存分配

#### ✅ 验证标准
- 内存不会泄漏
- 引用计数正确
- 能正确处理循环引用
- 有内存泄漏检测测试

---

## 🎯 第二阶段：核心容器与算法 (P0 - 3-4周)

### Task 2.1: 实现 iterators 模块

#### 📁 文件结构
```
src/iterators/
├── iterator_traits.h  📝 需创建
├── iterator_tags.h    📝 需创建
├── reverse_iterator.h 📝 需创建
└── CMakeLists.txt     📝 需创建
```

#### ✅ 实现要求

**1. iterator_tags.h**
```cpp
namespace zen {

// 迭代器类别标签
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

} // namespace zen
```

**2. iterator_traits.h**
```cpp
template<typename Iterator>
struct iterator_traits {
    using difference_type = typename Iterator::difference_type;
    using value_type = typename Iterator::value_type;
    using pointer = typename Iterator::pointer;
    using reference = typename Iterator::reference;
    using iterator_category = typename Iterator::iterator_category;
};

// 指针特化
template<typename T>
struct iterator_traits<T*> {
    using difference_type = ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using iterator_category = random_access_iterator_tag;
};
```

**3. reverse_iterator.h**
```cpp
template<typename Iterator>
class reverse_iterator {
protected:
    Iterator current_;

public:
    using iterator_type = Iterator;
    using iterator_category = typename iterator_traits<Iterator>::iterator_category;
    using value_type = typename iterator_traits<Iterator>::value_type;
    using difference_type = typename iterator_traits<Iterator>::difference_type;
    using pointer = typename iterator_traits<Iterator>::pointer;
    using reference = typename iterator_traits<Iterator>::reference;

    reverse_iterator();
    explicit reverse_iterator(Iterator x);
    reverse_iterator(const reverse_iterator& other);

    reference operator*() const;
    pointer operator->() const;
    reverse_iterator& operator++();
    reverse_iterator operator++(int);
    reverse_iterator& operator--();
    reverse_iterator operator--(int);
};
```

---

### Task 2.2: 实现 containers 模块 - vector

#### 📁 文件结构
```
src/containers/sequential/
├── vector.h           ⚠️ 待完善
└── vector.cpp         ⚠️ 待完善
```

#### ✅ 实现要求

**vector.h 核心接口**
```cpp
template<typename T, typename Allocator = allocator<T>>
class vector {
private:
    T* data_;
    size_t size_;
    size_t capacity_;
    Allocator allocator_;

    void reallocate(size_t new_capacity);
    void move_elements(T* dest, T* src, size_t count);

public:
    // 类型定义
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;

    // 迭代器
    using iterator = T*;
    using const_iterator = const T*;

    // 构造函数
    vector() noexcept(noexcept(Allocator()));
    explicit vector(const Allocator& alloc) noexcept;
    explicit vector(size_type count, const T& value = T(), const Allocator& alloc = Allocator());
    explicit vector(size_type count, const Allocator& alloc = Allocator());
    vector(const vector& other);
    vector(vector&& other) noexcept;
    ~vector();

    // 赋值运算符
    vector& operator=(const vector& other);
    vector& operator=(vector&& other) noexcept;

    // 元素访问
    reference operator[](size_type pos);
    const_reference operator[](size_type pos) const;
    reference at(size_type pos);
    const_reference at(size_type pos) const;
    reference front();
    const_reference front() const;
    reference back();
    const_reference back() const;
    T* data() noexcept;
    const T* data() const noexcept;

    // 迭代器
    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    iterator end() noexcept;
    const_iterator end() const noexcept;

    // 容量
    bool empty() const noexcept;
    size_type size() const noexcept;
    size_type capacity() const noexcept;
    void reserve(size_type new_cap);
    void shrink_to_fit();

    // 修改器
    void clear() noexcept;
    void push_back(const T& value);
    void push_back(T&& value);
    void pop_back();
    iterator insert(const_iterator pos, const T& value);
    iterator insert(const_iterator pos, T&& value);
    iterator erase(const_iterator pos);
    iterator erase(const_iterator first, const_iterator last);
    void resize(size_type count);
    void resize(size_type count, const T& value);
};
```

#### 📝 实现要点

**关键算法：**
1. **动态扩容策略**
   - 容量不足时按 2 倍扩容
   - 手动分配新内存
   - 移动构造/移动语义
   - 析构旧对象并释放内存

2. **插入元素**
   - 检查容量，必要时扩容
   - 移动后续元素
   - 构造新元素
   - 使用 placement new

3. **删除元素**
   - 调用析构函数
   - 移动后续元素填补空缺

4. **内存管理**
   - 使用自定义分配器
   - 正确处理异常安全
   - RAII 管理资源

**实现示例：**
```cpp
// 动态扩容
template<typename T, typename Allocator>
void vector<T, Allocator>::reallocate(size_t new_capacity) {
    if (new_capacity <= capacity_) return;

    // 分配新内存
    T* new_data = allocator_.allocate(new_capacity);

    // 移动元素
    for (size_t i = 0; i < size_; ++i) {
        allocator_.construct(new_data + i, std::move(data_[i]));
        allocator_.destroy(data_ + i);
    }

    // 释放旧内存
    if (data_) {
        allocator_.deallocate(data_, capacity_);
    }

    data_ = new_data;
    capacity_ = new_capacity;
}

// 插入元素
template<typename T, typename Allocator>
void vector<T, Allocator>::push_back(const T& value) {
    if (size_ >= capacity_) {
        reallocate(capacity_ == 0 ? 1 : capacity_ * 2);
    }
    allocator_.construct(data_ + size_, value);
    ++size_;
}
```

#### ✅ 验证标准
- 内存管理正确，无泄漏
- 支持移动语义
- 异常安全
- 时间复杂度符合要求（push_back均摊O(1)）

---

### Task 2.3: 实现 containers 模块 - list

#### ✅ 实现要求

```cpp
template<typename T, typename Allocator = allocator<T>>
class list {
private:
    struct Node {
        T data;
        Node* prev;
        Node* next;
    };

    Node* head_;  // 头哨兵
    Node* tail_;  // 尾哨兵
    size_t size_;
    Allocator allocator_;

public:
    // 双向链表实现
    // 支持任意位置插入/删除 O(1)
    // 但不支持随机访问
};
```

#### 📝 实现要点
- 使用哨兵节点简化边界处理
- 每个节点包含前驱和后继指针
- 插入和删除只需修改指针
- 迭代器支持双向遍历

---

### Task 2.4: 实现 containers 模块 - map (红黑树)

#### ✅ 实现要求

```cpp
template<typename Key, typename Value, typename Compare = less<Key>>
class map {
private:
    enum Color { RED, BLACK };

    struct Node {
        Key key;
        Value value;
        Color color;
        Node* left;
        Node* right;
        Node* parent;
    };

    Node* root_;
    size_t size_;
    Compare compare_;

    // 红黑树操作
    void left_rotate(Node* x);
    void right_rotate(Node* y);
    void insert_fixup(Node* z);
    void erase_fixup(Node* x);

public:
    // 红黑树实现
    // 插入、删除、查找 O(log n)
    // 保证平衡
};
```

#### 📝 实现要点
- 实现红黑树的5条性质
- 旋转操作（左旋、右旋）
- 插入修复（维持红黑性质）
- 删除修复（维持红黑性质）
- 中序遍历得到有序序列

---

### Task 2.5: 实现 containers 模块 - unordered_map (哈希表)

#### ✅ 实现要求

```cpp
template<typename Key, typename Value,
         typename Hash = hash<Key>,
         typename KeyEqual = equal_to<Key>>
class unordered_map {
private:
    struct Bucket {
        list<pair<const Key, Value>> chain;
    };

    vector<Bucket> buckets_;
    size_t size_;
    Hash hash_;
    KeyEqual key_equal_;

    float max_load_factor_;

    void rehash(size_t count);

public:
    // 哈希表实现
    // 平均 O(1) 查找、插入、删除
    // 使用链地址法解决冲突
};
```

#### 📝 实现要点
- 手撕哈希函数（不能使用std::hash）
- 链地址法解决冲突
- 动态扩容（当负载因子超过阈值）
- 迭代器遍历所有桶

---

### Task 2.6: 实现 algorithms 模块

#### ✅ 实现要求

**1. 排序算法**
```cpp
// 快速排序
template<typename RandomIt, typename Compare>
void quick_sort(RandomIt first, RandomIt last, Compare comp);

// 归并排序
template<typename RandomIt, typename Compare>
void merge_sort(RandomIt first, RandomIt last, Compare comp);

// 堆排序
template<typename RandomIt, typename Compare>
void heap_sort(RandomIt first, RandomIt last, Compare comp);
```

**2. 查找算法**
```cpp
// 二分查找
template<typename RandomIt, typename T, typename Compare>
RandomIt binary_search(RandomIt first, RandomIt last, const T& value, Compare comp);

// 线性查找
template<typename InputIt, typename T, typename Compare>
InputIt find(InputIt first, InputIt last, const T& value, Compare comp);
```

**3. 转换算法**
```cpp
// 转换
template<typename InputIt, typename OutputIt, typename UnaryOp>
OutputIt transform(InputIt first, InputIt last, OutputIt d_first, UnaryOp unary_op);

// 遍历
template<typename InputIt, typename UnaryOp>
UnaryOp for_each(InputIt first, InputIt last, UnaryOp f);
```

**4. 数值算法**
```cpp
// 累加
template<typename InputIt, typename T>
T accumulate(InputIt first, InputIt last, T init);
```

#### 📝 实现要点

**快速排序示例：**
```cpp
template<typename RandomIt, typename Compare>
void quick_sort(RandomIt first, RandomIt last, Compare comp) {
    if (first >= last - 1) return;

    // 选择基准值
    auto pivot = *(first + (last - first) / 2);

    // 分区
    auto i = first, j = last - 1;
    while (i <= j) {
        while (comp(*i, pivot)) ++i;
        while (comp(pivot, *j)) --j;
        if (i <= j) {
            std::swap(*i, *j);
            ++i;
            --j;
        }
    }

    // 递归排序
    quick_sort(first, j + 1, comp);
    quick_sort(i, last, comp);
}
```

#### ✅ 验证标准
- 算法正确性
- 时间复杂度符合理论
- 稳定性（如果需要）
- 边界情况处理

---

## 🚀 第三阶段：基础工具模块 (P1 - 5-6周)

### Task 3.1: 实现 math 模块

#### ✅ 实现要求
- 基础数学运算（gcd, lcm, abs）
- 随机数生成器（手撕随机数算法）
- 数值统计（mean, variance, std_dev）
- 定点数运算（可选）

### Task 3.2: 实现 type 模块

#### ✅ 实现要求
- 泛型约束（concepts）
- 类型擦除
- 安全类型转换

### Task 3.3: 实现 error 模块

#### ✅ 实现要求
- 错误码封装
- 异常链
- 错误上下文

### Task 3.4: 实现 fmt 模块

#### ✅ 实现要求
- 类型安全的格式化
- 自定义类型格式化
- 便捷打印接口（print, println）

---

## 📋 开发检查清单

每个模块完成后，检查以下项目：

- [ ] **代码实现**
  - [ ] 所有接口都已实现
  - [ ] 代码风格符合规范
  - [ ] 注释清晰完整
  - [ ] 异常安全

- [ ] **单元测试**
  - [ ] 基本功能测试
  - [ ] 边界条件测试
  - [ ] 异常情况测试
  - [ ] 性能测试（如果需要）

- [ ] **文档**
  - [ ] 头文件注释
  - [ ] 实现注释
  - [ ] 使用示例
  - [ ] 设计文档

- [ ] **验证**
  - [ ] 编译通过
  - [ ] 测试通过
  - [ ] 无内存泄漏
  - [ ] 性能达标

---

## 💡 开发建议

1. **先小步快跑**：每个函数都要先实现最简单的版本，测试通过后再优化
2. **测试驱动**：先写测试用例，再实现功能
3. **注释先行**：写代码前先写注释，明确算法思路
4. **逐步优化**：先保证正确性，再优化性能
5. **参考标准**：接口设计参考 STL，但要保持简洁

---

**开始编码吧！建议从 base 模块的 type_traits.h 开始！** 🚀
