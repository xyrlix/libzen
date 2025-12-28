# Doxygen 指南

## 1. Doxygen 简介

Doxygen 是一个用于生成软件文档的工具，它可以从源代码中的特殊注释中提取信息，生成各种格式的文档（如 HTML、LaTeX、RTF 等）。Doxygen 主要用于 C、C++、Java、Python 等语言，是开源社区中最常用的文档生成工具之一。

### 1.1 Doxygen 的主要功能

- 生成类和函数的 API 文档
- 生成类之间的继承关系图
- 生成协作关系图
- 支持 Markdown 格式的注释
- 支持自定义宏和别名
- 支持多种输出格式

## 2. Doxygen 注释规范

libzen 项目使用 Doxygen 风格的注释来生成 API 文档。以下是 Doxygen 注释的基本规范：

### 2.1 注释的位置

Doxygen 注释应该放在要注释的代码元素（如类、函数、变量等）的前面，使用 `/**` 开始，使用 `*/` 结束。

### 2.2 常用的 Doxygen 标签

| 标签          | 描述     | 示例                                       |
| ------------- | -------- | ------------------------------------------ |
| `@brief`      | 简短描述 | `@brief 这是一个简短描述`                  |
| `@param`      | 函数参数 | `@param[in] name 用户名`                   |
| `@return`     | 返回值   | `@return 成功返回 0，失败返回错误码`       |
| `@tparam`     | 模板参数 | `@tparam T 元素类型`                       |
| `@exception`  | 异常说明 | `@exception std::runtime_error 运行时错误` |
| `@see`        | 相关链接 | `@see other_function()`                    |
| `@note`       | 注意事项 | `@note 这个函数是线程安全的`               |
| `@warning`    | 警告信息 | `@warning 这个函数已过时`                  |
| `@deprecated` | 废弃说明 | `@deprecated 请使用 new_function() 代替`   |
| `@example`    | 示例代码 | `@example example.cpp`                     |

### 2.3 类的注释示例

```cpp
/**
 * @brief 向量容器类
 *
 * 一个动态数组容器，支持快速随机访问和末尾插入/删除操作。
 * 类似于标准库的 std::vector。
 *
 * @tparam T 元素类型
 * @tparam Allocator 分配器类型
 */
template <typename T, typename Allocator = default_allocator<T>>
class vector {
public:
    /**
     * @brief 默认构造函数
     *
     * 创建一个空向量。
     */
    vector();

    /**
     * @brief 带初始大小的构造函数
     *
     * 创建一个包含 count 个默认构造元素的向量。
     *
     * @param count 初始元素数量
     */
    explicit vector(size_t count);

    /**
     * @brief 复制构造函数
     *
     * @param other 要复制的向量
     */
    vector(const vector& other);

    /**
     * @brief 移动构造函数
     *
     * @param other 要移动的向量
     */
    vector(vector&& other) noexcept;

    /**
     * @brief 析构函数
     */
    ~vector();

    /**
     * @brief 复制赋值运算符
     *
     * @param other 要复制的向量
     * @return 引用当前对象
     */
    vector& operator=(const vector& other);

    /**
     * @brief 移动赋值运算符
     *
     * @param other 要移动的向量
     * @return 引用当前对象
     */
    vector& operator=(vector&& other) noexcept;

    /**
     * @brief 获取元素数量
     *
     * @return 当前向量中的元素数量
     */
    size_t size() const noexcept;

    /**
     * @brief 获取容量
     *
     * @return 当前向量的容量
     */
    size_t capacity() const noexcept;

    /**
     * @brief 检查向量是否为空
     *
     * @return 如果向量为空返回 true，否则返回 false
     */
    bool empty() const noexcept;

    /**
     * @brief 下标访问运算符
     *
     * @param index 元素索引
     * @return 引用指定位置的元素
     * @note 不进行边界检查
     */
    T& operator[](size_t index) noexcept;

    /**
     * @brief 常量下标访问运算符
     *
     * @param index 元素索引
     * @return 常量引用指定位置的元素
     * @note 不进行边界检查
     */
    const T& operator[](size_t index) const noexcept;

    /**
     * @brief 获取首元素
     *
     * @return 引用首元素
     * @warning 如果向量为空，行为未定义
     */
    T& front() noexcept;

    /**
     * @brief 获取尾元素
     *
     * @return 引用尾元素
     * @warning 如果向量为空，行为未定义
     */
    T& back() noexcept;

    /**
     * @brief 获取数据指针
     *
     * @return 指向向量数据的指针
     */
    T* data() noexcept;

    /**
     * @brief 末尾插入元素
     *
     * @param value 要插入的元素
     */
    void push_back(const T& value);

    /**
     * @brief 末尾移动插入元素
     *
     * @param value 要插入的元素
     */
    void push_back(T&& value);

    /**
     * @brief 末尾插入多个元素
     *
     * @param count 插入元素的数量
     * @param value 要插入的元素值
     */
    void insert_back(size_t count, const T& value);

    /**
     * @brief 删除末尾元素
     *
     * @warning 如果向量为空，行为未定义
     */
    void pop_back();

    /**
     * @brief 清空向量
     *
     * 删除所有元素，但不释放内存。
     */
    void clear() noexcept;

    /**
     * @brief 预留内存
     *
     * @param new_capacity 新的容量
     */
    void reserve(size_t new_capacity);

    /**
     * @brief 收缩容量
     *
     * 将容量调整为与大小相同。
     */
    void shrink_to_fit();
};
```

### 2.4 函数的注释示例

```cpp
/**
 * @brief 计算两个数的和
 *
 * @param a 第一个数
 * @param b 第二个数
 * @return 两个数的和
 *
 * @example
 * @code
 * int sum = add(1, 2); // sum = 3
 * @endcode
 */
int add(int a, int b) {
    return a + b;
}
```

### 2.5 变量的注释示例

```cpp
/**
 * @brief 全局常量：π
 */
const double PI = 3.141592653589793;

/**
 * @brief 用户计数器
 */
static int user_count = 0;
```

## 3. Doxygen 配置

libzen 项目使用 `Doxyfile` 配置文件来控制 Doxygen 的行为。以下是 `Doxyfile` 中的主要配置项：

### 3.1 项目信息

```
PROJECT_NAME           = "libzen"
PROJECT_NUMBER         = "1.0.0"
PROJECT_BRIEF          = "从零开始搭建的C++基础库"
```

### 3.2 文档输出设置

```
OUTPUT_DIRECTORY       = ./docs/api
OUTPUT_LANGUAGE        = Chinese
```

### 3.3 源文件设置

```
INPUT                  = ./include ./src
FILE_PATTERNS          = *.h *.cpp *.cc
RECURSIVE              = YES
```

### 3.4 文档注释设置

```
JAVADOC_AUTOBRIEF      = YES
ALIASES                = ""\
                         "rst=@verbatim embed:rst:leading-asterisk" \
                         "endrst=@endverbatim"
```

### 3.5 类和模块设置

```
CLASS_DIAGRAMS         = YES
HAVE_DOT               = YES
CLASS_GRAPH            = YES
COLLABORATION_GRAPH    = YES
UML_LOOK               = YES
```

### 3.6 输出格式设置

```
GENERATE_HTML          = YES
HTML_OUTPUT            = html
GENERATE_LATEX         = NO
GENERATE_MAN           = NO
```

## 4. 生成 Doxygen 文档

要生成 libzen 项目的 Doxygen 文档，请按照以下步骤操作：

### 4.1 安装 Doxygen

首先，确保已经安装了 Doxygen 和 Graphviz（用于生成图形）：

#### Linux

```bash
sudo apt-get install doxygen graphviz
```

#### macOS

```bash
brew install doxygen graphviz
```

#### Windows

从 Doxygen 官方网站下载安装包：https://www.doxygen.nl/download.html

### 4.2 生成文档

在项目根目录下运行以下命令：

```bash
doxygen Doxyfile
```

### 4.3 查看文档

生成的文档将存储在 `docs/api/html` 目录中。打开 `index.html` 文件即可查看文档：

```bash
# Linux/macOS
open docs/api/html/index.html

# Windows
start docs/api/html/index.html
```

## 5. 最佳实践

### 5.1 注释的内容

- 描述代码的"为什么"而不仅仅是"是什么"
- 解释复杂的算法或设计决策
- 说明函数的前置条件和后置条件
- 描述函数的副作用
- 说明线程安全性

### 5.2 注释的风格

- 使用清晰、简洁的语言
- 避免使用缩写和行话
- 保持注释的格式一致
- 定期更新注释，确保与代码同步

### 5.3 常见的错误

- 注释与代码不一致
- 注释过于冗长或过于简短
- 使用过时的标签或语法
- 忽略了重要的细节

## 6. 参考资料

- [Doxygen 官方网站](https://www.doxygen.nl/)
- [Doxygen 文档](https://www.doxygen.nl/manual/)
- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)

---

**libzen** - 从零开始搭建的 C++ 基础库，让每一行代码都知其然，知其所以然。
