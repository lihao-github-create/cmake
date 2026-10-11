# 第八阶段：Debug / Release 与构建配置

## 学习目标

- 理解 Debug、Release 和 RelWithDebInfo 的用途。
- 区分单配置生成器与多配置生成器。
- 在正确的阶段选择构建配置。
- 通过详细构建日志验证实际编译选项和输出位置。

## 常见构建配置

构建配置决定同一份源码使用哪些编译、链接选项。以下是本次 GCC 工具链下的常见默认选项，其他编译器或自定义工具链可能不同：

| 配置 | 主要用途 | 常见选项 |
|---|---|---|
| Debug | 调试程序、单步执行和查看变量 | `-g` |
| Release | 优化运行性能 | `-O3 -DNDEBUG` |
| RelWithDebInfo | 优化，同时保留调试信息 | `-O2 -g -DNDEBUG` |
| MinSizeRel | 优先优化程序体积 | `-Os -DNDEBUG` |

其中：

- `-g` 生成调试信息，帮助调试器将机器指令对应到源码。
- `-O2`、`-O3` 启用不同等级的优化，`-Os` 侧重体积优化。
- `-DNDEBUG` 定义 `NDEBUG` 宏，会禁用标准 `assert()` 检查。

Debug 不会自动启动调试器，Release 也不保证所有程序都更快。RelWithDebInfo 虽然保留调试信息，但优化仍可能使某些变量不可见，或使单步执行顺序与源码不完全一致。

## 单配置生成器：配置时选择

`Unix Makefiles` 和普通 `Ninja` 是单配置生成器。同一个构建目录在同一时间对应一种配置，通过配置阶段的 `CMAKE_BUILD_TYPE` 选择。

继续使用上一阶段的 `exercises/06_external_library`。在容器中进入该目录，分别构建：

```bash
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --verbose
./build-debug/app
```

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --verbose
./build-release/app
```

以上沿用本次环境默认的 `Unix Makefiles` 生成器；也可以在首次配置时显式指定 `-G "Unix Makefiles"`。

两种配置的程序都应输出：

```text
5 7 9
```

本次 Release 编译命令包含：

```text
/usr/bin/c++ -isystem /usr/include/eigen3 -O3 -DNDEBUG ... -c .../main.cpp
```

不同构建目录分别保存缓存、目标文件和可执行文件，便于保留和比较不同配置，无须修改 `CMakeLists.txt`。

目录名本身不决定构建类型。即使目录叫 `build-debug`，也必须通过配置参数或缓存确认实际类型。

## 未指定构建类型不等于 Debug

对于单配置生成器，执行：

```bash
cmake -S . -B build
```

不能据此认为正在使用 Debug。没有其他设置时，新构建目录的 `CMAKE_BUILD_TYPE` 通常为空；已有目录则可能沿用缓存值。

查看缓存：

```bash
cmake -LA -N build | grep CMAKE_BUILD_TYPE
```

本次实验输出：

```text
CMAKE_BUILD_TYPE:STRING=
```

空值表示没有指定构建类型，与显式选择 Debug 不同。

## 多配置生成器：构建时选择

`Ninja Multi-Config` 和 Visual Studio 是多配置生成器。一个构建目录可容纳多种配置，在构建阶段通过 `--config` 选择，不使用 `CMAKE_BUILD_TYPE` 选择配置。

容器已安装 Ninja，在同一练习目录执行：

```bash
cmake -S . -B build-multi -G "Ninja Multi-Config"
```

`-G` 指定生成器。这里使用新目录，因为不能在已有构建目录中直接将 `Unix Makefiles` 切换为另一种生成器。

分别构建和运行：

```bash
cmake --build build-multi --config Debug --verbose
cmake --build build-multi --config Release --verbose
./build-multi/Debug/app
./build-multi/Release/app
```

再构建带调试信息的优化版本：

```bash
cmake --build build-multi --config RelWithDebInfo --verbose
./build-multi/RelWithDebInfo/app
```

本次实验实际验证了以下结果，路径均相对于 `build-multi`：

| 配置 | 编译选项 | 目标文件 | 可执行文件 |
|---|---|---|---|
| Debug | `-g` | `CMakeFiles/app.dir/Debug/main.cpp.o` | `Debug/app` |
| Release | `-O3 -DNDEBUG` | `CMakeFiles/app.dir/Release/main.cpp.o` | `Release/app` |
| RelWithDebInfo | `-O2 -g -DNDEBUG` | `CMakeFiles/app.dir/RelWithDebInfo/main.cpp.o` | `RelWithDebInfo/app` |

不同配置不仅分开保存可执行文件，也分开保存中间目标文件，因此能够在同一构建目录中共存。以上是本练习未自定义输出目录时的布局。

三种配置的编译命令都保留了 `-isystem /usr/include/eigen3`，说明 Eigen target 的头文件使用要求在这些配置下都正常生效。

## 两种模型的对照

| 对比项 | 单配置生成器 | 多配置生成器 |
|---|---|---|
| 例子 | Unix Makefiles、Ninja | Ninja Multi-Config、Visual Studio |
| 选择配置的阶段 | CMake 配置阶段 | 构建阶段 |
| 选择方式 | `-DCMAKE_BUILD_TYPE=Release` | `--config Release` |
| 同一构建目录 | 同一时间对应一种配置 | 可容纳多种配置 |

典型命令：

```bash
# 单配置
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release

# 多配置
cmake -S . -B build-multi -G "Ninja Multi-Config"
cmake --build build-multi --config Release
```

对于本次使用 Makefiles 的 `build-debug`，执行：

```bash
cmake --build build-debug --config Release
```

不会将它切换为 Release；它仍由配置阶段设置的 `CMAKE_BUILD_TYPE` 决定。

## 常见问题与检查方法

| 问题 | 检查方法 |
|---|---|
| 不确定单配置目录使用哪种配置 | 查看缓存中的 `CMAKE_BUILD_TYPE` |
| 目录叫 `build-debug`，却没有调试选项 | 检查配置参数及实际编译命令，目录名不决定配置 |
| 对 Makefiles 使用 `--config Release` 没有切换效果 | 在配置阶段设置 `CMAKE_BUILD_TYPE` |
| 切换生成器时报错 | 使用新的构建目录 |
| 多配置构建后找不到程序 | 检查 `Debug/`、`Release/` 等配置子目录 |
| `--verbose` 没有显示编译命令 | 目标可能已是最新状态，需要时加 `--clean-first` 重新构建 |

## 本章结论

先确认生成器属于哪种模型，再在对应阶段选择配置。最后通过 `--verbose` 查看真正执行的编译和链接命令，验证配置是否生效。

本章通过同一个 Eigen 程序验证了 Release 优化选项、空构建类型，以及多配置目录下 Debug、Release、RelWithDebInfo 的选项和输出位置。
