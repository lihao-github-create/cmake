# 第四阶段：创建和链接库

## 学习目标

完成本章后，应当能够：

- 使用 `add_library()` 创建库 target。
- 使用 `target_link_libraries()` 建立 target 之间的依赖关系。
- 理解源文件如何先生成静态库，再链接到可执行程序。
- 观察库的 `PUBLIC` 使用要求如何传播给依赖它的 target。

## 练习项目

本阶段将计算功能从可执行程序中拆分成独立库：

```text
exercises/03_library/
├── CMakeLists.txt
├── app/
│   └── main.cpp
├── include/
│   └── calculator.hpp
└── src/
    └── calculator.cpp
```

`calculator.hpp` 对外声明函数：

```cpp
#pragma once

int multiply(int left, int right);
```

`calculator.cpp` 实现函数，`app/main.cpp` 包含公开头文件并调用该函数。

## 创建库 target

```cmake
add_library(calculator
    src/calculator.cpp
)
```

这条命令创建名为 `calculator` 的库 target。当前配置生成了：

```text
build/libcalculator.a
```

它是静态库。`add_library()` 没有明确指定 `STATIC` 或 `SHARED` 时，库类型由 `BUILD_SHARED_LIBS` 决定；该变量未开启时通常生成静态库。

也可以明确指定库类型：

```cmake
add_library(calculator STATIC src/calculator.cpp)
add_library(calculator SHARED src/calculator.cpp)
```

本阶段使用不指定类型的写法，使项目可以通过统一配置决定构建静态库还是共享库。

## 描述库的使用要求

```cmake
target_include_directories(calculator
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(calculator
    PUBLIC
        cxx_std_17
)
```

在当前顶层 `CMakeLists.txt` 中：

```text
${CMAKE_CURRENT_SOURCE_DIR}
= /workspace/exercises/03_library
```

因此 include 路径是：

```text
/workspace/exercises/03_library/include
```

`PUBLIC` 表示这些属性同时用于：

1. 构建 `calculator` 自己。
2. 构建依赖 `calculator` 的 target。

因此，这些属性既让 `calculator.cpp` 找到 `calculator.hpp`，也会在建立依赖后让 `app/main.cpp` 找到同一个头文件。

## 创建并链接可执行 target

```cmake
add_executable(app
    app/main.cpp
)

target_link_libraries(app
    PRIVATE
        calculator
)
```

`target_link_libraries()` 在两个 target 之间建立关系：

```text
calculator
    ↑
    │ link
    │
   app
```

对 CMake target 而言，这个关系不仅表示最终链接 `libcalculator.a`，还表示 `app` 会接收 `calculator` 的 `PUBLIC` 和 `INTERFACE` 使用要求。

这里的 `PRIVATE` 表示 `app` 自己使用 `calculator`，但不会继续把这个依赖作为 `app` 的公开使用要求传播出去。完整传播规则将在下一阶段学习。

## 完整的构建描述

```cmake
cmake_minimum_required(VERSION 3.16)

project(library_demo LANGUAGES CXX)

add_library(calculator
    src/calculator.cpp
)

target_include_directories(calculator
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(calculator
    PUBLIC
        cxx_std_17
)

add_executable(app
    app/main.cpp
)

target_link_libraries(app
    PRIVATE
        calculator
)
```

## 配置、构建和运行

```bash
cd exercises/03_library
cmake -S . -B build
cmake --build build
./build/app
```

程序输出：

```text
6 * 7 = 42
```

构建顺序是：

```text
calculator.cpp
      ↓ 编译
calculator.cpp.o
      ↓ 归档
libcalculator.a
      ↓ 链接
     app
```

典型构建输出如下：

```text
Building CXX object CMakeFiles/calculator.dir/src/calculator.cpp.o
Linking CXX static library libcalculator.a
Built target calculator
Building CXX object CMakeFiles/app.dir/app/main.cpp.o
Linking CXX executable app
Built target app
```

由于 `app` 依赖 `calculator`，CMake 会确保库在链接可执行文件之前可用。

## 从真实命令观察传播和链接

显示完整构建命令：

```bash
cmake --build build --clean-first --verbose
```

也可以只查看编译器命令：

```bash
cmake --build build --clean-first --verbose 2>&1 \
    | grep -F '/usr/bin/c++'
```

`calculator.cpp` 和 `app/main.cpp` 的编译命令都包含：

```text
-I/workspace/exercises/03_library/include
```

`calculator` 自己获得该路径，是因为它的 include 属性是 `PUBLIC`；`app` 获得该路径，是因为它链接了 `calculator`，从而接收其公开使用要求。

最终链接命令类似：

```text
/usr/bin/c++ CMakeFiles/app.dir/app/main.cpp.o -o app libcalculator.a
```

这证明 `app` 的目标文件最终与 `libcalculator.a` 组合成可执行程序。

## `PUBLIC` 与 `PRIVATE` 的初步区别

如果把库的 include 路径改为：

```cmake
target_include_directories(calculator
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

那么：

- `calculator.cpp` 仍能找到 `calculator.hpp`，因为 `PRIVATE` 属性供当前 target 使用。
- `app/main.cpp` 无法再从 `calculator` 获得 include 路径；如果没有单独配置，该源文件会因找不到 `calculator.hpp` 而编译失败。

这说明 target 属性不仅要描述“当前 target 需要什么”，还要描述“使用当前 target 的代码需要什么”。

## 本章结论

库和可执行程序都属于 target：

```text
calculator = 库 target + 源文件 + 公开使用要求
app        = 可执行 target + 源文件 + calculator 依赖
```

核心命令是：

```cmake
add_library(calculator ...)
add_executable(app ...)
target_link_libraries(app PRIVATE calculator)
```

现代 CMake 会根据 target 依赖图同时处理构建顺序、实际链接项和使用要求传播，而不需要手动拼接编译器与链接器参数。
