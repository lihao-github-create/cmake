# 第三阶段：建立 Target 思维

## 学习目标

完成本章后，应当能够：

- 将 target 理解为 CMake 中的核心构建单元。
- 将源文件、头文件搜索路径和 C++ 标准绑定到具体 target。
- 从 verbose 构建输出中识别编译和链接命令。
- 理解同一 target 中的源文件如何共享编译属性。

## 练习项目

本阶段的练习结构是：

```text
exercises/02_target/
├── CMakeLists.txt
├── include/
│   └── math.hpp
└── src/
    ├── main.cpp
    └── math.cpp
```

`math.hpp` 声明加法函数：

```cpp
#pragma once

int add(int left, int right);
```

`math.cpp` 提供函数实现，`main.cpp` 调用该函数并输出计算结果。

## Target 是什么

本项目使用以下命令创建可执行 target：

```cmake
add_executable(target_demo
    src/main.cpp
    src/math.cpp
)
```

`target_demo` 不应只被理解为最终的可执行文件名。它是一个逻辑构建单元，集中描述生成该程序所需的信息：

```text
target_demo
├── 源文件
│   ├── src/main.cpp
│   └── src/math.cpp
├── 头文件搜索路径
└── C++ 标准要求
```

现代 CMake 的基本工作方式是：

```text
创建 target
    ↓
给 target 添加构建属性
    ↓
CMake 将属性转换成编译和链接命令
```

## 完整的构建描述

本阶段的 `CMakeLists.txt` 是：

```cmake
cmake_minimum_required(VERSION 3.16)

project(target_demo LANGUAGES CXX)

add_executable(target_demo
    src/main.cpp
    src/math.cpp
)

target_include_directories(target_demo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(target_demo
    PRIVATE
        cxx_std_17
)
```

几个 `target_*` 命令的第一个参数都是被配置的 target 名称：

```cmake
target_include_directories(target_demo ...)
target_compile_features(target_demo ...)
```

如果以后向 `target_demo` 增加新的源文件，该文件会自动获得这个 target 的相关编译属性，不需要逐个源文件重复设置。

## 头文件搜索路径

```cmake
target_include_directories(target_demo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

`${CMAKE_CURRENT_SOURCE_DIR}` 表示当前 `CMakeLists.txt` 所在的源码目录。因此，CMake 会得到完整的 `include/` 路径，并为 `target_demo` 的编译命令生成类似参数：

```text
-I/workspace/exercises/02_target/include
```

这样，源文件就能直接使用：

```cpp
#include "math.hpp"
```

此处的 `PRIVATE` 表示这个 include 路径供 `target_demo` 自己构建时使用。它对依赖 target 的传播含义会在后续阶段详细学习。

## C++ 标准要求

```cmake
target_compile_features(target_demo
    PRIVATE
        cxx_std_17
)
```

`cxx_std_17` 表示 `target_demo` 至少需要 C++17。CMake 会根据当前编译器决定是否添加 `-std=...` 参数。

当前练习使用 GCC 11，它的默认语言模式已经满足 C++17，所以真实编译命令中可能看不到 `-std=c++17`。这不代表配置没有生效，而是编译器默认设置已经满足 target 的要求。

## 编译和链接过程

完整重新构建并显示底层命令：

```bash
cmake --build build --clean-first --verbose
```

- `--clean-first` 先清理已有构建产物，再执行构建。
- `--verbose` 显示 Make 和编译器实际执行的命令。

构建过程可以概括为：

```text
src/main.cpp ──编译──→ main.cpp.o ┐
                                  ├──链接──→ target_demo
src/math.cpp ──编译──→ math.cpp.o ┘
```

两条编译命令都包含 target 的 include 路径：

```text
/usr/bin/c++ -I/workspace/exercises/02_target/include ... -c src/main.cpp
/usr/bin/c++ -I/workspace/exercises/02_target/include ... -c src/math.cpp
```

链接命令则将两个目标文件组合成一个可执行程序：

```text
/usr/bin/c++ main.cpp.o math.cpp.o -o target_demo
```

编译命令中的 `-MD`、`-MT` 和 `-MF` 用于记录头文件依赖。修改 `math.hpp` 后，构建工具可以据此判断哪些源文件需要重新编译。

## 常见的 target 命令

后续工程会继续使用以下模式：

```cmake
target_include_directories(target_name ...)
target_compile_features(target_name ...)
target_compile_definitions(target_name ...)
target_compile_options(target_name ...)
target_link_libraries(target_name ...)
```

这些命令分别描述 target 的头文件路径、语言特性、宏定义、编译参数和链接依赖。相比全局设置，target 级设置能更准确地控制属性作用范围。

## 本章结论

现代 CMake 的核心思维是：

```text
target = 构建产物 + 源文件 + 构建属性 + 依赖关系
```

先使用 `add_executable()` 或 `add_library()` 创建 target，再通过 `target_*` 命令描述它需要的属性。CMake 最终会把这些声明转换为具体的编译器和链接器命令。
