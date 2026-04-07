# libzen API 文档

欢迎使用 libzen API 文档！本文档提供完整的 API 参考。

## 目录

- [内存管理](#内存管理)
- [容器](#容器)
- [算法](#算法)
- [网络编程](#网络编程)
- [文件系统](#文件系统)
- [事件循环](#事件循环)
- [定时器](#定时器)
- [日志系统](#日志系统)
- [配置解析](#配置解析)
- [加密与哈希](#加密与哈希)
- [进程管理](#进程管理)
- [缓冲区](#缓冲区)
- [序列化](#序列化)
- [系统工具](#系统工具)

---

## 内存管理

### 智能指针

#### `unique_ptr<T>`

独占所有权的智能指针。

```cpp
#include <zen/memory.h>

// 创建 unique_ptr
auto ptr = zen::make_unique<int>(42);

// 访问
int value = *ptr;

// 重置
ptr.reset();

// 释放所有权
auto raw = ptr.release();
```

#### `shared_ptr<T>`

共享所有权的智能指针，使用引用计数。

```cpp
#include <zen/memory.h>

auto ptr1 = zen::make_shared<std::string>("Hello");
auto ptr2 = ptr1;  // 引用计数 +1

// 获取引用计数
size_t count = ptr1.use_count();  // 2
```

#### `weak_ptr<T>`

弱引用，不增加引用计数。

```cpp
#include <zen/memory.h>

auto sptr = zen::make_shared<int>(42);
zen::weak_ptr<int> wptr = sptr;

// 尝试升级为 shared_ptr
if (auto ptr = wptr.lock()) {
    *ptr = 100;
}

// 检查对象是否销毁
if (wptr.expired()) {
    // 对象已销毁
}
```

---

## 容器

### `vector<T>`

动态数组，自动扩容。

```cpp
#include <zen/containers/vector.h>

zen::vector<int> vec;
vec.push_back(1);
vec.push_back(2);
vec.push_back(3);

// 访问
int val = vec[0];

// 迭代
for (auto it = vec.begin(); it != vec.end(); ++it) {
    // ...
}

// 大小和容量
size_t size = vec.size();
size_t cap = vec.capacity();
```

### `map<K, V>`

基于红黑树的有序关联容器。

```cpp
#include <zen/containers/map.h>

zen::map<std::string, int> m;
m["a"] = 1;
m["b"] = 2;
m["c"] = 3;

// 查找
auto it = m.find("b");
if (it != m.end()) {
    // 找到
}

// 遍历
for (const auto& kv : m) {
    std::cout << kv.first << ": " << kv.second << std::endl;
}
```

### `unordered_map<K, V>`

基于哈希表的无序关联容器。

```cpp
#include <zen/containers/unordered_map.h>

zen::unordered_map<std::string, int> m;
m["a"] = 1;
m["b"] = 2;

// 查找（平均 O(1)）
int val = m["a"];
```

---

## 算法

### 排序

```cpp
#include <zen/algorithms.h>

zen::vector<int> vec = {3, 1, 4, 1, 5, 9};

// 快速排序
zen::sort(vec.begin(), vec.end());

// 归并排序（稳定）
zen::stable_sort(vec.begin(), vec.end());

// 堆排序
zen::heap_sort(vec.begin(), vec.end());

// 插入排序（小数据量）
zen::insertion_sort(vec.begin(), vec.end());
```

### 查找

```cpp
#include <zen/algorithms.h>

zen::vector<int> vec = {1, 3, 5, 7, 9};

// 二分查找
auto it = zen::lower_bound(vec.begin(), vec.end(), 5);

// 查找
auto found = zen::find(vec.begin(), vec.end(), 7);

// 条件查找
auto pred = [](int x) { return x > 5; };
auto it2 = zen::find_if(vec.begin(), vec.end(), pred);
```

### 数值算法

```cpp
#include <zen/algorithms.h>

zen::vector<int> vec = {1, 2, 3, 4, 5};

// 累加
int sum = zen::accumulate(vec.begin(), vec.end(), 0);

// 内积
int dot = zen::inner_product(vec.begin(), vec.end(), vec.begin(), 0);

// 部分和
zen::partial_sum(vec.begin(), vec.end(), vec.begin());

// 相邻差分
zen::adjacent_difference(vec.begin(), vec.end(), vec.begin());
```

---

## 网络编程

### Socket

```cpp
#include <zen/net.h>

zen::net::socket sock;
sock.create(zen::net::address_family::ipv4, 
            zen::net::socket_type::stream,
            zen::net::protocol_type::tcp);

sock.set_reuse_addr(true);
sock.set_non_blocking(true);

// 连接
zen::net::socket_address addr("127.0.0.1", 8080);
sock.connect(addr);

// 发送
const char* msg = "Hello";
sock.send(msg, strlen(msg));

// 接收
char buf[1024];
ssize_t n = sock.recv(buf, sizeof(buf));
```

### TCP 服务器

```cpp
#include <zen/net.h>

zen::event::event_loop loop;
zen::net::tcp_server server(&loop);

server.set_connection_callback([](const zen::net::tcp_connection_ptr& conn) {
    std::cout << "New connection" << std::endl;
});

server.set_message_callback([](const zen::net::tcp_connection_ptr& conn,
                               zen::net::buffer* buf) {
    std::string data(buf->read_ptr(), buf->readable_bytes());
    conn->send("Echo: " + data);
});

server.start("0.0.0.0", 8080);
loop.run();
```

### TCP 客户端

```cpp
#include <zen/net.h>

zen::event::event_loop loop;
zen::net::tcp_client client(&loop);

client.set_connect_callback([](bool success) {
    if (success) {
        std::cout << "Connected!" << std::endl;
    }
});

client.set_message_callback([](const zen::net::tcp_connection_ptr& conn,
                               zen::net::buffer* buf) {
    // 处理消息
});

client.connect("127.0.0.1", 8080);
loop.run();
```

---

## 文件系统

### 路径操作

```cpp
#include <zen/filesystem.h>

zen::fs::path p("/home/user/document.txt");

// 路径组件
std::string filename = p.filename();      // "document.txt"
std::string stem = p.stem();              // "document"
std::string ext = p.extension();          // ".txt"
zen::fs::path parent = p.parent_path();   // "/home/user"

// 路径拼接
zen::fs::path p2 = p / "subdir" / "file.txt";

// 规范化
zen::fs::path p3 = "/usr/local/../bin";  // -> "/usr/bin"
```

### 目录操作

```cpp
#include <zen/filesystem.h>

// 创建目录
zen::fs::create_directory("/tmp/test");
zen::fs::create_directories("/tmp/a/b/c");

// 遍历
zen::fs::directory_iterator it("/tmp");
zen::fs::directory_iterator end;
for (; it != end; ++it) {
    std::cout << it->path() << std::endl;
}

// 递归遍历
zen::fs::recursive_directory_iterator rit("/tmp");
zen::fs::recursive_directory_iterator rend;
for (; rit != rend; ++rit) {
    std::cout << rit->path() << " depth: " << rit.depth() << std::endl;
}

// 删除
zen::fs::remove_directory("/tmp/test");
zen::fs::remove_all("/tmp/a");
```

---

## 事件循环

### 基本使用

```cpp
#include <zen/event.h>

zen::event::event_loop loop;

// 添加 IO 事件
auto handler = loop.add_io_event(fd, zen::event::io_event::read,
    [](int fd, zen::event::io_event events) {
        // 处理可读事件
    });

// 添加定时器
loop.add_timer(1000, []() {
    std::cout << "Timer fired!" << std::endl;
});

// 运行事件循环
loop.run();

// 停止事件循环
loop.stop();
```

### 重复定时器

```cpp
#include <zen/event.h>

zen::event::event_loop loop;

// 添加重复定时器（每 1000ms 触发）
loop.add_repeat_timer(1000, []() {
    std::cout << "Tick" << std::endl;
});

loop.run();
```

---

## 定时器

### 定时器管理

```cpp
#include <zen/timer.h>

zen::timer_manager mgr;

// 添加一次性定时器
zen::timer_id id1 = mgr.add_once(1000, []() {
    std::cout << "Once timer" << std::endl;
});

// 添加重复定时器
zen::timer_id id2 = mgr.add_repeat(500, []() {
    std::cout << "Repeat timer" << std::endl;
});

// 取消定时器
mgr.cancel(id1);

// 处理定时器
mgr.tick();
```

---

## 日志系统

### 基本使用

```cpp
#include <zen/logging.h>

// 使用全局 logger
ZEN_LOG_INFO("Hello, world!");
ZEN_LOG_WARN("Warning message");
ZEN_LOG_ERROR("Error: code=%d", errno);

// 使用格式化日志
ZEN_LOGF_INFO("Processing %d items", count);

// 设置日志级别
zen::logging::global_logger()->set_level(zen::logging::log_level::debug);

// 添加输出目标
auto file_sink = zen::logging::file_sink::create("/tmp/app.log");
zen::logging::global_logger()->add_sink(file_sink);
```

---

## 配置解析

### INI 配置

```cpp
#include <zen/config.h>

zen::config::ini_config cfg;

// 加载配置
cfg.load("/path/to/config.ini");

// 读取配置
std::string host = cfg.get("server", "host").as_string();
int port = cfg.get("server", "port").as_int();
bool debug = cfg.get("server", "debug").as_bool();

// 写入配置
cfg.set("server", "host", "127.0.0.1");
cfg.set("server", "port", "8080");

// 保存配置
cfg.save("/path/to/config.ini");
```

---

## 加密与哈希

### 哈希

```cpp
#include <zen/crypto.h>

// MD5
std::string md5 = zen::crypto::md5::hash("hello");

// SHA256
std::string sha256 = zen::crypto::sha256::hash("hello");

// SHA512
std::string sha512 = zen::crypto::sha512::hash("hello");

// CRC32
uint32_t crc = zen::crypto::crc32::hash("hello");

// HMAC
std::string hmac = zen::crypto::hmac<zen::crypto::sha256>::hash("key", "message");
```

### 对称加密

```cpp
#include <zen/crypto.h>

// AES-128-CBC
zen::crypto::aes cipher(zen::crypto::aes::key_size_128);
std::string key = "0123456789ABCDEF";
std::string iv = "1234567890ABCDEF";
std::string plaintext = "Hello World!";

std::string encrypted = cipher.encrypt(plaintext, key, 
    zen::crypto::cipher_mode::cbc, iv);

std::string decrypted = cipher.decrypt(encrypted, key,
    zen::crypto::cipher_mode::cbc, iv);
```

### Base64 编码

```cpp
#include <zen/crypto.h>

// 编码
std::string encoded = zen::crypto::base64::encode("Hello");

// 解码
std::string decoded = zen::crypto::base64::decode("SGVsbG8=");
```

---

## 进程管理

### 进程操作

```cpp
#include <zen/process.h>

// 获取进程信息
int pid = zen::process::get_pid();
int ppid = zen::process::get_ppid();
std::string cmd = zen::process::get_process_name(pid);

// 启动子进程
zen::process::process proc;
proc.start("/bin/ls", {"-la", "/tmp"});

// 等待子进程退出
proc.wait();
int exit_code = proc.exit_code();
```

### 守护进程

```cpp
#include <zen/process.h>

zen::process::daemon daemon;

// 启动守护进程
daemon.daemonize();
daemon.write_pid_file("/var/run/myapp.pid");

// 主循环
while (true) {
    // ...
}
```

---

## 缓冲区

### 动态缓冲区

```cpp
#include <zen/buffer.h>

zen::dynamic_buffer buf;

// 追加数据
buf.push_back('H');
buf.append("ello", 4);

// 读取数据
char data[128];
size_t n = buf.read(data, sizeof(data));

// 调整大小
buf.reserve(1024);
buf.resize(512);

// 删除数据
buf.erase_front(5);
buf.erase_back(1);
```

### 环形缓冲区

```cpp
#include <zen/buffer.h>

zen::ring_buffer buf(4096);

// 写入
size_t written = buf.write(data, size);

// 读取
size_t read = buf.read(out, out_size);

// 零拷贝读取
void* ptr1;
size_t size1;
void* ptr2;
size_t size2;
buf.read_ptr(&ptr1, &size1, &ptr2, &size2);

// 提交读取
buf.commit_read(size1 + size2);
```

---

## 序列化

### 二进制序列化

```cpp
#include <zen/serialize.h>

struct my_data {
    int a;
    float b;
    std::string c;
    
    template<typename Serializer>
    void serialize(Serializer& s) const {
        s(a);
        s(b);
        s(c);
    }
};

my_data data{42, 3.14f, "hello"};

zen::serialize::binary_serializer bs;
data.serialize(bs);

// 反序列化
zen::serialize::binary_deserializer bd(bs.data(), bs.size());
my_data loaded;
loaded.deserialize(bd);
```

---

## 系统工具

### 系统信息

```cpp
#include <zen/system.h>

// CPU 信息
int cpus = zen::system::cpu_count();

// 内存信息
uint64_t total_mem = zen::system::total_memory();
uint64_t avail_mem = zen::system::available_memory();

// 主机和用户信息
std::string host = zen::system::hostname();
std::string user = zen::system::username();

// 进程信息
int pid = zen::system::pid();
int tid = zen::system::tid();
```

### 时间和睡眠

```cpp
#include <zen/system.h>

// 获取时间
uint64_t ticks = zen::system::get_tick_count();
double secs = zen::system::get_realtime();
uint64_t monotonic = zen::system::get_monotonic_time();

// 睡眠
zen::system::sleep(1);
zen::system::sleep_ms(100);
zen::system::usleep(1000);
```

---

## 更多文档

每个模块都有更详细的 API 文档，请查看对应的头文件：
- `zen/memory.h` — 内存管理
- `zen/containers/` — 容器
- `zen/algorithms.h` — 算法
- ...
