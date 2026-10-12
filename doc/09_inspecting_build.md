# 第九阶段：查看 CMake 到底执行了什么

## 学习目标

- 使用 verbose 模式查看真实的编译和链接命令。
- 从命令中识别编译器、头文件路径、宏、编译选项和链接输入。
- 使用 CMake cache 检查配置结果和依赖查找结果。
- 理解 Debug、Release 和增量构建的实际行为。
- 生成 `compile_commands.json`，供 IDE 和静态分析工具使用。

## 为什么要查看实际命令

`CMakeLists.txt` 描述的是构建规则，真正执行编译的是编译器和链接器。构建失败时，最有价值的信息通常在 CMake 最终生成的命令中：

```text
头文件搜索路径是否正确
宏定义是否生效
使用了哪个编译器
编译选项是否正确
链接了哪些目标文件和库
```

因此，遇到“找不到头文件”“宏没有生效”或“链接失败”等问题时，应先查看实际命令。

## 使用 verbose 模式

以 `exercises/02_target` 为例：

```bash
cmake -S exercises/02_target -B build_stage9 \
    -DCMAKE_BUILD_TYPE=Debug

cmake --build build_stage9 --verbose
```

编译命令可能类似：

```text
/usr/bin/c++ \
  -I/.../exercises/02_target/include \
  -g \
  -MD -MT CMakeFiles/target_demo.dir/src/main.cpp.o \
  -MF CMakeFiles/target_demo.dir/src/main.cpp.o.d \
  -o CMakeFiles/target_demo.dir/src/main.cpp.o \
  -c /.../exercises/02_target/src/main.cpp
```

其中：

| 命令部分 | 含义 |
|---|---|
| `/usr/bin/c++` | 实际使用的 C++ 编译器 |
| `-I.../include` | 增加头文件搜索目录；`#include "math.hpp"` 会在这里查找 |
| `-g` | 生成 Debug 调试信息 |
| `-DNAME=value` | 定义预处理宏 |
| `-c` | 只编译，不进行最终链接 |
| `-o ...main.cpp.o` | 指定生成的目标文件 |
| `-MD`、`-MF` | 生成头文件依赖信息 |

链接阶段可能类似：

```text
/usr/bin/c++ -g \
  CMakeFiles/target_demo.dir/src/main.cpp.o \
  CMakeFiles/target_demo.dir/src/math.cpp.o \
  -o target_demo
```

编译器先把每个 `.cpp` 文件编译成 `.o` 目标文件，链接器再把这些 `.o` 文件和库组合成最终的可执行文件。源文件没有变化时，可以复用已有的 `.o` 文件。

## 比较 Debug 和 Release

单配置生成器（如 Unix Makefiles 和普通 Ninja）需要在配置时指定构建类型：

```bash
cmake -S exercises/02_target -B build_debug \
    -DCMAKE_BUILD_TYPE=Debug
cmake --build build_debug --verbose

cmake -S exercises/02_target -B build_release \
    -DCMAKE_BUILD_TYPE=Release
cmake --build build_release --verbose
```

通常可以观察到：

```text
Debug:   -g
Release: -O3 -DNDEBUG
```

`-g` 生成调试信息；`-O3` 启用较高级别的优化；`-DNDEBUG` 定义 `NDEBUG`，通常会使 `assert()` 失效。具体默认选项由编译器和 CMake 版本决定，实际命令才是最终依据。

对于 Visual Studio 等多配置生成器，构建类型通常在构建时选择：

```bash
cmake --build build --config Release --verbose
```

## 查看 CMake cache

列出构建目录中的缓存变量：

```bash
cmake -LA -N build_stage9
```

也可以直接查看：

```text
build_stage9/CMakeCache.txt
```

常用筛选方式：

```bash
cmake -LA -N build_stage9 | \
    grep -E 'CMAKE_BUILD_TYPE|CMAKE_CXX_COMPILER|Python|Eigen|CUDA'
```

典型结果：

```text
CMAKE_BUILD_TYPE:STRING=Release
CMAKE_CXX_COMPILER:FILEPATH=/usr/bin/c++
CMAKE_CXX_FLAGS_RELEASE:STRING=-O3 -DNDEBUG
```

cache 是配置阶段保存的变量和工具路径。`find_package()` 找到的依赖位置也可能被缓存，例如 `Python3_EXECUTABLE` 或 `Eigen3_DIR`。因此修改环境或依赖路径后，旧 cache 可能继续影响结果。

需要完全重新配置时，可以使用新的构建目录；也可以删除旧的构建目录后重新执行配置：

```bash
rm -rf build_stage9
cmake -S exercises/02_target -B build_stage9
```

只删除构建目录，不要删除源码目录。

## 观察增量构建

如果只修改 `src/math.cpp`，再次执行：

```bash
cmake --build build_release --verbose
```

通常只会重新编译：

```text
src/math.cpp.o
```

`main.cpp.o` 会被复用。之后由于目标文件发生变化，链接阶段仍会重新执行：

```text
main.cpp.o + math.cpp.o -> target_demo
```

编译器生成的 `.d` 文件记录头文件依赖，构建工具据此判断哪些源文件需要重新编译。修改一个被多个源文件包含的头文件时，所有依赖该头文件的源文件都可能重新编译。

## 生成 compile_commands.json

配置时打开 `CMAKE_EXPORT_COMPILE_COMMANDS`：

```bash
cmake -S exercises/02_target -B build_commands \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build_commands
```

生成的文件是：

```text
build_commands/compile_commands.json
```

内容类似：

```json
[
  {
    "directory": "/tmp/build_commands",
    "command": "/usr/bin/c++ -I/.../include -o .../main.cpp.o -c .../main.cpp",
    "file": "/.../main.cpp"
  }
]
```

每个条目描述一个源文件的完整编译命令。`clangd`、VSCode C/C++ 插件和静态分析工具可以使用它，从而准确知道头文件目录、宏和编译选项。

## 常见排查流程

遇到构建问题时，可以按下面顺序检查：

1. 重新配置，确认没有使用错误的构建目录或旧 cache。
2. 使用 `cmake --build build --clean-first --verbose` 查看完整命令。
3. 检查实际编译器、`-I` 头文件目录和 `-D` 宏定义。
4. 检查链接阶段的 `.o` 文件、库文件和链接顺序。
5. 使用 `cmake -LA -N build` 检查 `CMAKE_*` 和依赖相关缓存变量。
6. 必要时打开 `CMAKE_EXPORT_COMPILE_COMMANDS`，查看每个源文件的编译命令。

`cmake --build build --clean-first --verbose` 会先清理目标，再显示完整编译过程，适合验证配置是否真正传递到了所有源文件。

## 本阶段核心

```text
CMakeLists.txt
    ↓ 配置和生成
构建系统文件 + CMakeCache.txt
    ↓ 执行 verbose 构建
真实的编译命令和链接命令
```

现代 CMake 的排错方式不是猜测 target 属性是否生效，而是查看最终命令，结合 cache 和依赖文件确认实际行为。
