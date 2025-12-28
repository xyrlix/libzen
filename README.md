# libzen

<div align="center">
  <img src="https://via.placeholder.com/150" alt="libzen Logo" width="150" height="150">
  <h3>从零开始搭建的C++基础库</h3>
  <p>从语法细节→内存管理→工程化→性能优化，每一行代码知其然，知其所以然。</p>
  
  <p>
    <a href="https://github.com/yourname/libzen/actions"><img src="https://img.shields.io/github/actions/workflow/status/yourname/libzen/build.yml?branch=main" alt="Build Status"></a>
    <a href="https://github.com/yourname/libzen/blob/main/LICENSE"><img src="https://img.shields.io/github/license/yourname/libzen" alt="License"></a>
    <a href="https://github.com/yourname/libzen"><img src="https://img.shields.io/github/stars/yourname/libzen?style=social" alt="Stars"></a>
  </p>
</div>

## 项目简介

libzen是一个从零开始搭建的C++基础库，旨在提供高效、可靠的核心功能组件，涵盖内存管理、容器、算法、并发编程、网络编程等多个领域。本项目注重代码质量和性能优化，同时保持良好的可扩展性和易用性。

## 项目特点

- 🎯 **从零实现**：所有核心组件从零编写，深入理解底层原理
- 🚀 **高性能**：精心优化的算法和数据结构，提供卓越的性能
- 📦 **模块化设计**：清晰的模块划分，便于维护和扩展
- 🔧 **现代化C++**：基于C++17标准，充分利用现代C++特性
- 🔗 **易于集成**：完善的CMake配置，方便在其他项目中使用
- 🧪 **完善测试**：基于Google Test的全面单元测试
- 📚 **详细文档**：丰富的文档和示例，便于学习和使用
- 🌐 **跨平台支持**：支持Linux、macOS和Windows等主流操作系统

## 核心功能

| 模块 | 主要功能 |
|------|----------|
| **内存管理** | 自定义分配器、智能指针（unique_ptr/shared_ptr/weak_ptr）、内存池 |
| **容器库** | 顺序容器（vector/list/string）、关联容器（map/unordered_map）、容器适配器 |
| **泛型算法** | 排序、查找、转换、数值算法 |
| **并发编程** | 线程封装、同步原语、异步任务、线程池 |
| **网络编程** | Socket封装、TCP通信、Reactor模型 |
| **文件系统** | 文件路径操作、目录遍历、文件属性 |
| **日志系统** | 高性能日志、多输出端支持、日志级别控制 |
| **配置解析** | 支持INI和JSON格式 |
| **定时器框架** | 高精度定时器、定时器队列、定时任务管理 |
| **序列化** | 二进制和JSON序列化支持 |
| **数值与数学工具** | 基础数学运算、随机数生成、数值统计、定点数运算 |
| **类型系统增强** | 泛型约束、范围适配器、类型擦除、安全类型转换 |
| **错误处理增强** | 系统错误码封装、异常链、错误上下文、全局错误处理器 |
| **加密与哈希工具** | 哈希算法（MD5/SHA256/CRC32）、对称加密（AES）、非对称加密（RSA）、Base64编码 |
| **进程与信号管理** | 进程封装、守护进程、信号处理、进程工具 |
| **事件驱动框架** | 统一事件循环（epoll+定时器+信号）、事件处理器、安全回调 |
| **轻量级结构化序列化** | 结构化消息定义、高效二进制编解码、简易反射 |
| **系统级开发** | 文件系统操作、系统信息获取、系统工具函数 |
| **高性能缓冲区** | 动态缓冲区、环形缓冲区、零拷贝缓冲区 |
| **异步IO** | 异步文件IO、异步网络IO、IO多路复用 |
| **池化技术** | 线程池、连接池、对象池 |
| **数据库接口** | 数据库连接管理、结果集处理、事务支持 |
| **存储系统** | 文件存储、键值存储、对象存储 |
| **区块链技术** | 区块结构、交易处理、共识算法 |
| **图像处理** | 图像加载、图像处理、图像转换 |
| **音频处理** | 音频编解码、音频处理、音频播放 |
| **视频处理** | 视频编解码、视频处理、视频分析 |

## 环境要求

- C++17兼容编译器
  - GCC 7.1+ 或 Clang 5.0+ 或 MSVC 2019+
- CMake 3.16+
- Git

## 项目结构

```
libzen/
├── include/                # 对外统一暴露的头文件目录
│   └── zen/                # 对外命名空间头文件目录
├── src/                    # 所有模块的源文件目录
│   ├── base/               # 基础宏、类型 traits、编译期工具
│   ├── memory/             # 内存分配与智能指针
│   ├── containers/         # 核心容器实现
│   ├── iterators/          # 迭代器体系
│   ├── algorithms/         # 泛型算法
│   ├── utility/            # 通用工具类
│   ├── fmt/                # 格式化与打印
│   ├── threading/          # 并发编程
│   ├── io/                 # 异步IO和文件IO
│   ├── logging/            # 日志系统
│   ├── config/             # 配置解析
│   ├── filesystem/         # Linux文件系统操作
│   ├── net/                # 网络编程
│   ├── timer/              # 定时器框架
│   ├── serialize/          # 序列化/反序列化
│   ├── math/               # 数值与数学工具
│   ├── type/               # 类型系统增强
│   ├── error/              # 错误处理增强
│   ├── crypto/             # 加密与哈希工具
│   ├── process/            # 进程与信号管理
│   ├── event/              # 事件驱动框架
│   ├── proto/              # 轻量级结构化序列化
│   ├── system/             # 系统级开发工具
│   ├── buffer/             # 高性能缓冲区
│   ├── database/           # 数据库接口
│   ├── storage/            # 存储系统
│   ├── blockchain/         # 区块链技术
│   ├── pool/               # 池化技术
│   ├── image/              # 图像处理
│   ├── audio/              # 音频处理
│   └── video/              # 视频处理
├── tests/                  # 单元测试
├── cmake/                  # cmake配置文件目录
├── examples/               # 使用示例目录
├── docs/                   # 文档目录
├── scripts/                # 辅助脚本目录
├── CMakeLists.txt          # 核心cmake配置
├── LICENSE                 # 开源协议
└── README.md               # 项目说明
```

## 目录设计核心原则与补充说明

### 1. 核心分层：「对外接口」与「内部实现」分离
- **include/zen/**：仅存放对外暴露的头文件，无实现代码，外部项目只需配置`-Ilibzen/include`，即可通过`#include <zen/containers/vector.h>`调用。
- **src/**：存放所有实现代码（.cpp）和内部头文件（仅模块内使用，不对外暴露），保证内部实现的封装性，后续修改实现不影响外部接口。

### 2. 模块细化：按「功能子域」拆分子目录
- 原有目录（如containers/、threading/）内部按子功能拆分，避免单个目录文件过多，提升可读性和维护性。
- 新增核心模块（filesystem/、net/、timer/、serialize/）均为 Linux 工业级开发必备，填补原有目录在系统交互、高并发场景的空白。

### 3. 工程化配套：支撑「编译、测试、部署、使用」全流程
- **cmake/**：统一配置编译选项、依赖、版本，支持其他项目通过find_package快速集成。
- **examples/**：提供每个模块的极简使用示例，降低上手成本，同时验证库的可用性。
- **tests/**：按模块划分单元测试，保证每个组件的正确性（通过 gtest 自动化测试）。
- **scripts/**：一键化脚本，提升开发 / 部署效率，适配 Linux 环境下的批量操作。
- **docs/**：完善的文档，方便后续自己维护和他人协作。

### 4. 依赖关系：低耦合、自上而下的依赖链

```plaintext
base/ → utility/ → memory/ → iterators/ → containers/ → algorithms/
base/ → math/、type/、error/（最底层依赖）
error/ → crypto/、process/、event/、net/（错误处理依赖）
threading/ → timer/、event/（并发依赖）
event/ → net/（网络编程依赖事件驱动框架）
memory/ → crypto/、proto/（内存管理依赖）
serialize/ → proto/（序列化依赖）
```

严格控制模块间依赖，避免循环依赖，保证编译顺序的合理性。

### 5. Linux 环境适配：关键细节
- 所有系统相关代码（如net/epoll.h、filesystem/directory.h）均基于 Linux 系统调用封装，不兼容 Windows（聚焦核心需求）。
- 编译选项中默认开启`-std=c++11`（可扩展到c++17）、`-Wall`、`-Wextra`、`-O2`，并添加`-lpthread`（线程依赖）、`-lrt`（定时器依赖）。
- scripts/install.sh默认将编译后的静态库（libzen.a）/ 动态库（libzen.so）安装到`/usr/local/lib`，头文件安装到`/usr/local/include/zen`，符合 Linux 系统库的安装规范。

## 快速开始

### 1. 克隆仓库

```bash
git clone https://github.com/yourname/libzen.git
cd libzen
```

### 2. 编译安装

#### 自动编译安装

```bash
# 一键编译
bash scripts/build.sh

# 一键安装（需要管理员权限）
sudo bash scripts/install.sh
```

#### 手动编译安装

```bash
# 创建构建目录
mkdir build
cd build

# 配置CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译
make -j$(nproc)

# 安装
sudo make install
```

### 3. 验证安装

```bash
# 检查头文件是否安装成功
ls -la /usr/local/include/zen/

# 检查库文件是否安装成功
ls -la /usr/local/lib/libzen*
```

### 4. 使用示例

#### 基本示例

```cpp
#include <zen/containers/vector.h>
#include <zen/fmt.h>

int main() {
    // 创建并使用vector容器
    zen::vector<int> vec = {1, 2, 3, 4, 5};
    vec.push_back(6);
    
    // 格式化打印
    zen::print("Vector elements: {}", vec);
    
    // 使用算法
    zen::sort(vec.begin(), vec.end(), std::greater<int>());
    zen::print("Sorted in descending order: {}", vec);
    
    return 0;
}
```

#### 编译并运行

```bash
g++ -std=c++17 example.cpp -o example -lzen
./example
```

#### 输出

```
Vector elements: [1, 2, 3, 4, 5, 6]
Sorted in descending order: [6, 5, 4, 3, 2, 1]
```

## 文档

- 📖 [快速入门指南](./docs/quick_start.md)：5分钟上手libzen
- 🛠️ [编译&安装文档](./docs/build.md)：详细的编译安装说明
- 🔍 [API文档](./docs/api/)：完整的API参考
- 📐 [设计文档](./docs/design/)：核心模块的设计思路

## 测试

### 运行单元测试

```bash
# 一键运行所有测试
bash scripts/test.sh

# 手动运行测试
cd build
ctest -V
```

### 测试覆盖率

```bash
# 生成测试覆盖率报告
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
make -j$(nproc)
ctest
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
lcov --list coverage.info
```

## 贡献指南

我们欢迎社区的贡献！无论是bug修复、新功能开发还是文档改进，都可以通过以下方式参与：

### 1. 提交Issue

- 报告bug
- 提出新功能建议
- 讨论技术问题

### 2. 提交Pull Request

1. Fork本仓库
2. 创建特性分支：`git checkout -b feature/your-feature`
3. 提交更改：`git commit -m "Add your feature"`
4. 推送分支：`git push origin feature/your-feature`
5. 提交Pull Request

### 3. 代码规范

- 遵循Google C++风格指南
- 确保所有代码都有适当的注释
- 为新功能添加单元测试
- 确保所有测试通过

## 版本管理

本项目使用语义化版本控制（Semantic Versioning）：

- **主版本号**：重大变更，不兼容的API修改
- **次版本号**：新功能，向下兼容
- **修订号**：bug修复，向下兼容

## 许可证

本项目采用MIT许可证，详见[LICENSE](./LICENSE)文件。

## 联系方式

- 📧 邮箱：your.email@example.com
- 🔗 GitHub：https://github.com/yourname/libzen
- 💬 讨论组：https://groups.google.com/g/libzen

## 致谢

感谢所有为libzen项目做出贡献的开发者和社区成员！

## 更新日志

### v1.0.0 (2025-12-28)

- 初始版本发布
- 实现核心功能模块
- 完善测试和文档

## 特别声明

本项目仅供学习和研究使用，请勿用于生产环境。如有问题或建议，欢迎提出Issue或Pull Request。

---

**libzen** - 从零开始搭建的C++基础库，让每一行代码都知其然，知其所以然。