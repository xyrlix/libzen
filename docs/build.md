# 编译与安装文档

## 环境要求

- C++17 兼容编译器
  - GCC 7.1+ 或 Clang 5.0+ 或 MSVC 2019+
- CMake 3.16+
- Git

## 编译步骤

### 1. 克隆仓库

```bash
git clone https://github.com/xyrlix/libzen.git
cd libzen
```

### 2. 创建构建目录

```bash
mkdir build
cd build
```

### 3. 运行 CMake 配置

```bash
# 基本配置
cmake ..

# 指定构建类型（Debug/Release）
cmake .. -DCMAKE_BUILD_TYPE=Release

# 指定安装路径
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install

# 禁用测试
cmake .. -DBUILD_TESTS=OFF

# 禁用示例
cmake .. -DBUILD_EXAMPLES=OFF
```

### 4. 编译

```bash
# 基本编译
make

# 并行编译（加快速度）
make -j$(nproc)

# 在 Windows 上使用 MSBuild
msbuild libzen.sln
```

## 安装

### 1. 安装到系统目录

```bash
# 需要管理员权限
sudo make install
```

### 2. 安装到指定目录

```bash
# 提前在 CMake 配置时指定 -DCMAKE_INSTALL_PREFIX
make install
```

## 验证安装

### 1. 检查头文件

```bash
ls -la /usr/local/include/zen/
```

### 2. 检查库文件

```bash
ls -la /usr/local/lib/libzen*
```

### 3. 检查 CMake 配置文件

```bash
ls -la /usr/local/lib/cmake/zen/
```

## 卸载

```bash
cd build
sudo make uninstall
```

## 在项目中使用

### CMake 项目

```cmake
find_package(zen REQUIRED)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE zen::zen)
```

### 手动链接

```bash
g++ -std=c++17 main.cpp -o myapp -I/usr/local/include -L/usr/local/lib -lzen
```

## 常见问题

### 1. 找不到 CMake

确保 CMake 已正确安装并添加到 PATH 环境变量中。

### 2. 编译错误

- 检查编译器版本是否符合要求
- 检查是否启用了 C++17 支持
- 尝试清理构建目录并重新配置

### 3. 链接错误

- 检查库文件是否已正确安装
- 检查链接路径是否正确

## 跨平台构建

### Linux

```bash
mkdir -p build/linux
cd build/linux
cmake ../.. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### macOS

```bash
mkdir -p build/macos
cd build/macos
cmake ../.. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
```

### Windows

```bash
mkdir -p build/windows
cd build/windows
cmake ../.. -G "Visual Studio 16 2019" -A x64
msbuild libzen.sln /p:Configuration=Release
```
