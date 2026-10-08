# 第五阶段：PRIVATE、PUBLIC 与 INTERFACE

## 学习目标

完成本章后，应当能够：

- 从当前 target 和使用者两个角度判断属性作用范围。
- 正确选择 `PRIVATE`、`PUBLIC` 或 `INTERFACE`。
- 理解使用要求如何沿 target 依赖关系传播。
- 使用 `INTERFACE` 库表达只有头文件的依赖。

## 两个判断问题

为 target 配置 include 路径、编译特性、宏定义或链接依赖时，应当回答：

1. 当前 target 自己是否需要这个属性？
2. 依赖当前 target 的其他 target 是否需要这个属性？

对应规则是：

| 关键字 | 当前 target | 依赖它的 target |
|---|---:|---:|
| `PRIVATE` | ✓ | ✗ |
| `PUBLIC` | ✓ | ✓ |
| `INTERFACE` | ✗ | ✓ |

可以记成：

```text
只有当前 target 需要       → PRIVATE
当前 target 和使用者都需要 → PUBLIC
只有使用者需要             → INTERFACE
```

## 始终从命令的第一个 target 判断

例如：

```cmake
target_include_directories(calculator PUBLIC include)
                           ↑
                  从 calculator 的角度判断
```

而：

```cmake
target_link_libraries(app PRIVATE calculator)
                      ↑
                 从 app 的角度判断
```

这里的“使用者”或“依赖它的 target”通常是通过 `target_link_libraries()` 建立依赖的 target，而不是简单地指某个包含了头文件的源文件。

## `PRIVATE`：只供当前 target 使用

假设 `calculator` 有一个仅供内部实现使用的头文件目录：

```text
src/detail/
└── internal.hpp
```

只有 `calculator.cpp` 会包含这些头文件，库的使用者不应接触它们：

```cmake
target_include_directories(calculator
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/detail
)
```

`calculator` 自己获得该路径，依赖 `calculator` 的 `app` 不会获得它。

本阶段还将公开头文件目录暂时改成了 `PRIVATE`：

```cmake
target_include_directories(calculator
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

实验结果是：

```text
calculator.cpp 编译成功
libcalculator.a 生成成功
app/main.cpp 编译失败：calculator.hpp: No such file or directory
```

这证明 `PRIVATE` 属性不会传播给使用者。

## `PUBLIC`：当前 target 和使用者都需要

`calculator.hpp` 是库的公开头文件：

- `calculator.cpp` 包含它，所以库自己需要 `include/`。
- `app/main.cpp` 包含它，所以库的使用者也需要 `include/`。

因此正确配置是：

```cmake
target_include_directories(calculator
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

当 `app` 链接 `calculator` 时：

```cmake
target_link_libraries(app PRIVATE calculator)
```

`app` 会自动接收 `calculator` 的公开 include 路径，不需要重复配置 `target_include_directories(app ...)`。

传播关系是：

```text
calculator 的 PUBLIC 使用要求
                 ↓
                app
```

## `INTERFACE`：只供使用者使用

把 `calculator` 的公开 include 路径暂时改为：

```cmake
target_include_directories(calculator
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

实验中，`calculator.cpp` 立即因为找不到 `calculator.hpp` 而失败。这是因为 `INTERFACE` 不作用于 `calculator` 自己。

从属性语义看，`app` 会获得该 include 路径；但由于 `app` 依赖 `calculator`，整个构建会在库编译失败后停止，通常还没机会编译 `app`。

`INTERFACE` 最典型的用途是只有头文件、没有独立编译单元的库。

## 属性在 CMake 中的两个部分

以 include 路径为例，可以把作用域理解成 target 的两组属性：

```text
PRIVATE   → 当前 target 的 INCLUDE_DIRECTORIES
INTERFACE → 对外的 INTERFACE_INCLUDE_DIRECTORIES
PUBLIC    → 同时写入以上两组属性
```

依赖 target 时，CMake 会读取依赖项的 `INTERFACE_*` 使用要求，并将它们应用到使用者。编译特性、宏定义、编译选项和链接依赖也采用类似模型。

## 创建头文件库

本阶段创建了下面的练习：

```text
exercises/04_interface/
├── CMakeLists.txt
├── app/
│   └── main.cpp
└── include/
    └── square.hpp
```

`square.hpp` 直接提供函数实现：

```cpp
#pragma once

constexpr int square(int value)
{
    return value * value;
}
```

它没有对应的 `.cpp` 文件，因此使用 `INTERFACE` 库表达其使用要求：

```cmake
add_library(math_headers INTERFACE)

target_include_directories(math_headers
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(math_headers
    INTERFACE
        cxx_std_17
)
```

这里有两个相关但不同的 `INTERFACE`：

```cmake
add_library(math_headers INTERFACE)
```

表示创建不产生二进制文件的头文件库 target。

```cmake
target_include_directories(math_headers INTERFACE ...)
```

表示 include 路径只提供给使用 `math_headers` 的 target。因为 `math_headers` 自己没有编译步骤，所以不需要该路径。

## 使用头文件库

```cmake
add_executable(interface_app
    app/main.cpp
)

target_link_libraries(interface_app
    PRIVATE
        math_headers
)
```

这里的 `PRIVATE` 从 `interface_app` 的角度理解：

- `interface_app` 自己需要 `math_headers`。
- 不要求 `interface_app` 再把这个依赖公开给其他 target。

构建只出现两个主要步骤：

```text
Building CXX object .../app/main.cpp.o
Linking CXX executable interface_app
```

不会出现 `math_headers` 的编译步骤，也不会生成 `libmath_headers.a` 或 `libmath_headers.so`。它只负责将 include 路径和 C++17 要求传给 `interface_app`。

## 传播链

使用要求可以沿 target 依赖关系继续传播。例如：

```text
app ──PRIVATE──> calculator ──PUBLIC──> math_headers
```

`calculator` 以 `PUBLIC` 方式依赖 `math_headers` 时，`math_headers` 的使用要求既用于 `calculator`，也会继续传递给 `app`。如果改为 `PRIVATE`，则只用于 `calculator`，不会继续传播。

## 本章结论

选择作用域时，不要根据习惯或“库看起来应该公开”来猜，而是逐项判断：

```text
当前 target 是否需要？
它的使用者是否需要？
```

现代 CMake 将公开使用要求保存在 target 上，再通过 `target_link_libraries()` 构成的依赖图自动传播。正确使用这三个关键字，可以避免全局 include 路径、重复配置和依赖泄漏。
