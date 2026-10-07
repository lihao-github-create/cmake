# 第一阶段：理解 CMake 在做什么

## 学习目标

完成本章后，应当能够：

- 区分 CMake、构建工具和编译器的职责。
- 区分“配置”和“构建”两个步骤。
- 使用源码外构建，保持源码目录整洁。
- 在删除构建目录后重新生成并构建项目。

## CMake 的定位

CMake 不是编译器，而是**构建系统生成器**。典型构建流程如下：

```text
CMakeLists.txt
      ↓ CMake 配置和生成
Makefile / Ninja files / Visual Studio solution
      ↓ Make / Ninja / Visual Studio 执行构建规则
GCC / Clang / MSVC 编译和链接
      ↓
可执行文件或库
```

各工具的职责：

| 工具 | 职责 |
|---|---|
| CMake | 读取 `CMakeLists.txt`，生成适合当前平台的构建系统 |
| Make、Ninja 等 | 读取构建规则，判断并安排需要执行的构建任务 |
| GCC、Clang、MSVC 等 | 真正编译源文件并链接程序或库 |

当前练习环境包含 CMake 3.22.1、GNU Make 4.3 和 G++ 11.4.0，因此默认可以使用下面这条构建链：

```text
CMake → Unix Makefiles → GNU Make → G++
```

Ninja 没有安装并不影响本阶段学习。

## 配置与构建

在项目源码根目录执行：

```bash
cmake -S . -B build
```

这一步称为**配置和生成**：

- `-S .` 指定当前目录为源码目录。
- `-B build` 指定 `build/` 为构建目录；目录不存在时 CMake 会创建它。
- CMake 检查编译器和依赖，并在构建目录中生成 Makefile 等文件。

然后执行：

```bash
cmake --build build
```

这一步称为**构建**。CMake 会调用构建目录对应的工具（当前环境通常是 GNU Make），再由构建工具调用 G++ 完成编译和链接。

推荐使用 `cmake --build build`，而不是直接运行 `make -C build`。前者不依赖具体构建工具，切换到 Ninja 或其他生成器后仍然适用。

## 源码外构建

推荐的目录结构是：

```text
project/
├── CMakeLists.txt
├── include/
├── src/
└── build/
```

这种方式称为 **out-of-source build（源码外构建）**。生成的缓存、Makefile、中间目标文件和最终产物都位于 `build/` 中，不会污染源码目录。

不推荐在源码目录中执行：

```bash
cmake .
```

该命令会将生成文件直接写入源码目录，使清理和管理变得困难。

## 重新生成构建目录

采用源码外构建后，`build/` 是可以重新生成的。如果它被删除或其中的配置需要重建，只要源码和 `CMakeLists.txt` 仍然存在，就可以重新执行：

```bash
cmake -S . -B build
cmake --build build
```

也可以在 shell 中写成：

```bash
cmake -S . -B build && cmake --build build
```

`&&` 表示只有配置成功后才执行构建。Linux 命令区分大小写，命令名必须写成小写的 `cmake`。

## 本章结论

```text
CMake 负责生成构建系统；
Make/Ninja 负责执行构建规则；
GCC/Clang/MSVC 负责真正的编译和链接。
```

最基本且推荐的工作流是：

```bash
cmake -S . -B build
cmake --build build
```
