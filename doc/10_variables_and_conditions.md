# 第十阶段：变量和条件判断

## 学习目标

- 使用 `set()` 定义普通变量，使用 `${}` 读取变量值。
- 使用 `if()` 控制配置逻辑。
- 区分普通变量和缓存变量，理解命令行 `-D` 的作用。
- 使用 `option()` 提供可配置的布尔开关。
- 区分顶层目录和当前目录、源码目录和构建目录。

## 普通变量与输出

```cmake
set(MY_TOPIC "CMake variables")
message(STATUS "Topic: ${MY_TOPIC}")
```

`set()` 设置普通变量，`${MY_TOPIC}` 读取变量值。`message(STATUS ...)` 输出配置阶段的信息，通常带有 `--` 前缀：

```text
-- Topic: CMake variables
```

这些语句在执行 `cmake -S . -B build` 的配置阶段运行，不是 C++ 程序运行时的逻辑。本阶段只观察配置行为，无须执行 `cmake --build build`。

## 条件判断

```cmake
set(ENABLE_DETAILS ON)

if(ENABLE_DETAILS)
    message(STATUS "Show details")
else()
    message(STATUS "Hide details")
endif()
```

`if(ENABLE_DETAILS)` 直接检查变量的值，不需要写成 `${ENABLE_DETAILS}`。常见布尔值如下：

| 真值 | 假值 |
|---|---|
| `ON`、`TRUE`、`YES`、非零数字 | `OFF`、`FALSE`、`NO`、`0`、空字符串 |

这是常见值的简表；未定义的变量用于 `if(变量名)` 时也为假。

比较字符串是否相等时，使用 `STREQUAL`：

```cmake
set(MY_MODE "study")

if(MY_MODE STREQUAL "study")
    message(STATUS "Study mode")
endif()
```

## 普通变量与缓存变量

命令行参数 `-D` 设置的是缓存变量：

```bash
cmake -S . -B build -DENABLE_DETAILS=OFF
```

缓存保存在 `build/CMakeCache.txt` 中。后续使用同一个构建目录配置时，即使不再传入 `-D`，也会沿用已有缓存值。

如果文件中仍然写着：

```cmake
set(ENABLE_DETAILS ON)
```

那么 `if(ENABLE_DETAILS)` 仍然为真，输出 `Show details`。原因是读取变量时，普通变量优先于同名缓存变量：

```text
缓存变量 ENABLE_DETAILS = OFF
普通变量 ENABLE_DETAILS = ON
    ↓
if() 读取普通变量，条件为真
```

缓存中的 `OFF` 没有被修改，只是被普通变量遮蔽了。

| 对比项 | 普通变量 | 缓存变量 |
|---|---|---|
| 本阶段的设置方式 | `set(NAME value)` | 命令行 `-D`、`option()` |
| 是否自动保存到 CMakeCache.txt | 否 | 是 |
| 下次配置 | 重新执行脚本中的设置语句 | 可以读取已有缓存值 |
| 存在同名变量时 | 读取时优先 | 被普通变量遮蔽 |

## 使用 option() 提供开关

如果希望用户通过命令行控制功能，将普通变量的 `set()` 替换为：

```cmake
option(ENABLE_DETAILS "Show detailed configuration messages" ON)
```

三个参数分别是选项名称、说明和初始默认值。对于本练习没有同名普通变量的情况，缓存中尚无该选项时使用默认值 `ON`；已有缓存值时保留已有值。

在练习目录 `exercises/10_variable_cond` 中，依次执行：

```bash
cmake -S . -B build -DENABLE_DETAILS=OFF
cmake -S . -B build
cmake -S . -B build -DENABLE_DETAILS=ON
```

对应输出：

| 配置操作 | 输出 | 原因 |
|---|---|---|
| 显式设置 `OFF` | `Hide details` | 使用命令行设置 |
| 不再传入 `-D` | `Hide details` | 沿用缓存中的 `OFF` |
| 显式设置 `ON` | `Show details` | 更新缓存为 `ON` |

`option(... ON)` 不会在每次配置时把选项重置为 `ON`。不同构建目录有各自的缓存，换一个全新构建目录才能独立观察初始默认值。

可以通过以下命令查看缓存：

```bash
cmake -LA -N build
```

其中会包含类似条目：

```text
ENABLE_DETAILS:BOOL=ON
```

## 用开关控制子目录

本次练习结构：

```text
exercises/10_variable_cond/
├── CMakeLists.txt
└── extras/
    └── CMakeLists.txt
```

顶层 `CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.16)

project(stage10_variables LANGUAGES NONE)

set(MY_TOPIC "CMake variables")
option(ENABLE_DETAILS "Show detailed configuration messages" ON)

message(STATUS "Topic: ${MY_TOPIC}")

if(ENABLE_DETAILS)
    message(STATUS "Show details")
else()
    message(STATUS "Hide details")
endif()

option(BUILD_EXTRAS "Configure extra modules" ON)

if(BUILD_EXTRAS)
    add_subdirectory(extras)
endif()
```

`LANGUAGES NONE` 表示不启用编译语言，适合本阶段只观察配置逻辑的练习。

`extras/CMakeLists.txt`：

```cmake
message(STATUS "Configuring extras")
message(STATUS "Root source: ${CMAKE_SOURCE_DIR}")
message(STATUS "Current source: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Root binary: ${CMAKE_BINARY_DIR}")
message(STATUS "Current binary: ${CMAKE_CURRENT_BINARY_DIR}")
```

在练习目录中分别执行：

```bash
cmake -S . -B build -DBUILD_EXTRAS=ON
cmake -S . -B build -DBUILD_EXTRAS=OFF
```

开启时出现 `Configuring extras` 和路径输出；关闭时不再出现，因为 CMake 没有执行 `add_subdirectory(extras)`。

真实工程中，如果子目录定义了库或可执行程序，这个开关就能控制是否将这些 target 加入本次生成的构建系统。关闭选项并不意味着自动删除以前生成的文件。

## 常用路径变量

| 变量 | 含义 | 本次在 extras 中的值（省略公共前缀） |
|---|---|---|
| `CMAKE_SOURCE_DIR` | 整个工程的顶层源码目录 | `10_variable_cond` |
| `CMAKE_CURRENT_SOURCE_DIR` | 当前正在处理的源码目录 | `10_variable_cond/extras` |
| `CMAKE_BINARY_DIR` | 整个工程的顶层构建目录 | `10_variable_cond/build` |
| `CMAKE_CURRENT_BINARY_DIR` | 当前源码目录对应的构建目录 | `10_variable_cond/build/extras` |

这些变量的值是绝对路径。在本次容器中，公共前缀为 `/workspace/exercises/`。

进入子目录时，顶层路径保持不变，`CURRENT` 路径随当前处理的目录变化。这里的 `BINARY` 指构建目录，不是某个可执行文件的路径；构建目录也可以存放生成的文件和中间产物。

本例使用 `add_subdirectory(extras)` 的默认目录映射，所以对应构建目录是 `build/extras`。

此前模块中的写法：

```cmake
target_include_directories(math
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

指向的是该模块源码目录下的 `include`，适合让每个模块描述自己的构建需求。

## 另外两个常用变量

| 变量 | 含义 |
|---|---|
| `CMAKE_BUILD_TYPE` | 单配置生成器的构建类型，例如 `Debug`、`Release` |
| `CMAKE_CXX_COMPILER` | CMake 选定的 C++ 编译器路径 |

多配置生成器通过构建时的 `--config` 选择配置，不通过 `CMAKE_BUILD_TYPE` 选择。详见[第八阶段](08_build_configurations.md)。

本练习使用 `LANGUAGES NONE`，没有启用 C++ 编译器检测。可以在使用 `project(... LANGUAGES CXX)` 的工程中，于 `project()` 之后输出 `CMAKE_CXX_COMPILER`。

## 综合判断

假设顶层写成：

```cmake
option(BUILD_EXTRAS "Configure extra modules" ON)
set(BUILD_EXTRAS OFF)

if(BUILD_EXTRAS)
    add_subdirectory(extras)
endif()
```

执行：

```bash
cmake -S . -B build -DBUILD_EXTRAS=ON
```

结果是不会处理 `extras`，但缓存中的 `BUILD_EXTRAS` 仍为 `ON`。普通变量 `OFF` 遮蔽了缓存变量，没有修改缓存。

希望用户通过 `-D` 控制功能时，应使用 `option()`，避免再设置同名普通变量来遮蔽它。下一阶段可以将这种模式用于 `BUILD_TESTS`，控制测试模块是否加入工程。
