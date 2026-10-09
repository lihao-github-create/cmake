# 第六阶段：多目录工程

## 学习目标

完成本章后，应当能够：

- 使用 `add_subdirectory()` 组织多目录工程。
- 让每个模块通过自己的 `CMakeLists.txt` 描述构建需求。
- 在不同目录中创建和使用 target。
- 理解顶层目录与子目录的职责边界。
- 通过 target 依赖传播公开的 include 路径和其他使用要求。

## 为什么需要多目录工程

真实项目通常会把应用程序、库和测试拆分到不同目录中。一个目录只负责一个相对独立的模块，可以减少顶层 `CMakeLists.txt` 的复杂度，也让模块更容易复用。

本阶段的练习目录如下：

```text
exercises/05_multi_directory/
├── CMakeLists.txt
├── app/
│   ├── CMakeLists.txt
│   └── main.cpp
└── libs/
    └── math/
        ├── CMakeLists.txt
        ├── include/
        │   └── math.hpp
        └── src/
            └── math.cpp
```

模块关系是：

```text
app ──PRIVATE──> math
```

其中 `math` 是静态库 target，`app` 是可执行程序 target。

## 顶层负责组织模块

顶层 `CMakeLists.txt` 不直接列出所有源文件，而是组织子目录：

```cmake
cmake_minimum_required(VERSION 3.16)

project(multi_directory_demo LANGUAGES CXX)

add_subdirectory(libs/math)
add_subdirectory(app)
```

`add_subdirectory()` 会让 CMake 进入指定目录并处理其中的 `CMakeLists.txt`。因此顶层文件表达的是工程结构，而不是每个模块的实现细节。

这里先添加 `libs/math`，再添加 `app`。这样处理 `app/CMakeLists.txt` 时，`math` target 已经创建，可以直接建立 target 依赖。

## 库模块自己描述构建需求

`libs/math/CMakeLists.txt`：

```cmake
add_library(math
    src/math.cpp
)

target_include_directories(math
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(math
    PUBLIC
        cxx_std_17
)
```

这里的 `${CMAKE_CURRENT_SOURCE_DIR}` 是当前 `CMakeLists.txt` 所在目录，即：

```text
.../exercises/05_multi_directory/libs/math
```

所以 `math` 的公开头文件目录是：

```text
.../exercises/05_multi_directory/libs/math/include
```

`PUBLIC` 的含义是：

- 编译 `math.cpp` 时需要这个目录；
- 使用 `math` 的 target 也需要这个目录。

## 应用模块使用库模块

`app/CMakeLists.txt`：

```cmake
add_executable(app
    main.cpp
)

target_link_libraries(app
    PRIVATE
        math
)
```

`target_link_libraries()` 建立了 `app` 到 `math` 的 target 依赖。这个命令不只表示最终链接 `libmath.a`，还会让 `app` 接收 `math` 的 `PUBLIC` 和 `INTERFACE` 使用要求。

因此 `app/main.cpp` 可以直接写：

```cpp
#include "math.hpp"
```

而不需要再次调用 `target_include_directories(app ...)`。

这里的 `PRIVATE` 是从 `app` 的角度判断的：`app` 自己需要 `math`，但不要求将这个依赖继续作为 `app` 的公开使用要求传播给其他 target。

## 构建和运行

在练习目录执行：

```bash
cmake -S . -B build
cmake --build build
./build/app/app
```

程序输出：

```text
5
6
```

因为 `app` 是在 `app` 子目录中创建的，单配置生成器下可执行文件默认位于：

```text
build/app/app
```

构建顺序由 target 依赖决定：

```text
math.cpp
    ↓ 编译
libmath.a
    ↓ 链接
app
```

## 用 verbose 构建验证传播

使用下面的命令查看实际执行的编译和链接命令：

```bash
cmake --build build --clean-first --verbose
```

`app/main.cpp` 的编译命令中应出现：

```text
-I.../exercises/05_multi_directory/libs/math/include
```

这证明 `math` 的 `PUBLIC` include 目录已经传播到 `app`。

`app` 的链接命令中应出现：

```text
.../libs/math/libmath.a
```

这证明 `target_link_libraries(app PRIVATE math)` 已经生效。

如果只执行 `cmake --build build --verbose`，而源码没有变化，CMake 可能只显示依赖检查和 `Nothing to be done`，因为目标已经是最新状态。`--clean-first` 会先清理目标文件，再显示完整的编译和链接命令。

## 常见错误与原因

### 忘记链接 `math`

如果删除：

```cmake
target_link_libraries(app PRIVATE math)
```

那么 `app` 不会获得 `math` 的公开 include 目录，通常会在编译阶段报错：

```text
fatal error: math.hpp: No such file or directory
```

这不是链接器错误，而是头文件搜索路径没有通过 target 依赖传播。

### 手动添加 include 但不链接库

如果给 `app` 手动添加 `math/include`，编译可能成功，但没有链接 `math` 时，最终链接阶段会找不到 `add()` 或 `multiply()` 的实现。正确做法是描述 target 依赖，而不是分别手动拼接头文件路径和库文件路径。

### 子目录职责混乱

不推荐在顶层集中写出所有模块的源文件和 include 路径。更清晰的分工是：

```text
顶层 CMakeLists.txt       → 组织模块
libs/math/CMakeLists.txt  → 描述 math target
app/CMakeLists.txt        → 描述 app target 及其依赖
```

## 本章结论

多目录工程的核心不是“每个目录放一个 CMakeLists.txt”，而是让目录边界与 target 边界一致：

```text
每个模块自己描述构建需求
顶层负责组织模块
target_link_libraries() 建立依赖
PUBLIC / INTERFACE 使用要求沿依赖图传播
```

掌握 `add_subdirectory()`、target 依赖和使用要求传播后，就可以自然地组织应用程序、库和测试目录，为后续接入第三方库和安装打包做准备。
