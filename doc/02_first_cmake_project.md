# 第二阶段：第一个 CMake 项目

## 学习目标

完成本章后，应当能够：

- 编写最小的 C++ `CMakeLists.txt`。
- 使用 CMake 配置、构建并运行一个可执行程序。
- 理解源码经过编译和链接生成可执行文件的过程。
- 理解 target 的基本含义和增量构建行为。

## 项目结构

本阶段的练习位于：

```text
exercises/01_hello/
├── CMakeLists.txt
└── main.cpp
```

`main.cpp` 是普通的 C++ 源文件：

```cpp
#include <iostream>

int main()
{
    std::cout << "Hello CMake\n";
    return 0;
}
```

`CMakeLists.txt` 描述如何构建这个程序：

```cmake
cmake_minimum_required(VERSION 3.16)

project(hello LANGUAGES CXX)

add_executable(hello main.cpp)
```

## 三个基础命令

### `cmake_minimum_required`

```cmake
cmake_minimum_required(VERSION 3.16)
```

要求使用的 CMake 版本不低于 3.16，并确定与该版本对应的 CMake 策略行为。它通常位于顶层 `CMakeLists.txt` 的开头。

### `project`

```cmake
project(hello LANGUAGES CXX)
```

声明项目名称为 `hello`，并且项目使用 C++。`CXX` 是 CMake 对 C++ 语言的名称。

学习指南中的简写：

```cmake
project(hello)
```

也可以工作，但默认会启用 C 和 C++。明确写出 `LANGUAGES CXX` 可以让项目意图更清楚。

### `add_executable`

```cmake
add_executable(hello main.cpp)
```

创建一个名为 `hello` 的可执行 target，其源文件是 `main.cpp`。这条命令只是在配置阶段向 CMake 描述构建目标，并不会立刻调用编译器。

`project(hello)` 中的 `hello` 是项目名称，`add_executable(hello ...)` 中的 `hello` 是 target 名称。二者可以不同，小项目中通常使用相同名称。

## 配置项目

进入练习目录：

```bash
cd exercises/01_hello
```

使用源码外构建进行配置：

```bash
cmake -S . -B build
```

配置期间，CMake 会：

1. 读取和处理 `CMakeLists.txt`。
2. 查找并检查 C++ 编译器。
3. 检测编译器支持的特性。
4. 建立 target 和源文件之间的关系。
5. 在 `build/` 中生成 Makefile 等构建文件。

输出中的两个关键阶段是：

```text
Configuring done
Generating done
```

- `Configuring` 表示 CMake 已完成项目描述和依赖关系的处理。
- `Generating` 表示 CMake 已生成底层构建工具所需的文件。

## 构建和运行

执行构建：

```bash
cmake --build build
```

源文件会经历两个主要步骤：

```text
main.cpp
   ↓ 编译
main.cpp.o
   ↓ 链接
hello
```

典型输出如下：

```text
Building CXX object CMakeFiles/hello.dir/main.cpp.o
Linking CXX executable hello
Built target hello
```

运行生成的程序：

```bash
./build/hello
```

## 增量构建

CMake 生成的构建系统会记录文件之间的依赖关系。再次执行：

```bash
cmake --build build
```

如果源码没有变化，已有的目标文件和可执行文件会被复用，不会重复编译。

修改 `main.cpp` 后再次构建，构建工具会发现源文件比对应的目标文件更新，于是重新编译 `main.cpp.o`，再重新链接 `hello`。在包含多个 target 的工程中，通常只会重建受到改动影响的部分。

## 本章结论

最小 CMake 项目的完整工作流是：

```bash
cmake -S . -B build
cmake --build build
./build/hello
```

其中：

```text
CMakeLists.txt 描述 target
cmake -S . -B build 配置并生成构建系统
cmake --build build 调用构建工具完成编译和链接
./build/hello 运行生成的可执行文件
```
