# CMake 通用学习环境

本仓库提供一个基于 Ubuntu 22.04 的容器化开发环境。它不绑定某个示例项目，可以供后续所有 CMake 练习共用。

环境包含：

- GCC/G++ 和基础编译工具
- Clang/Clang++
- CMake
- GNU Make
- Ninja
- GDB
- Git、pkg-config 和 Python 3

## 启动环境

在仓库根目录执行：

```bash
DEV_UID=$(id -u) DEV_GID=$(id -g) docker compose up -d --build
```

`DEV_UID` 和 `DEV_GID` 会让容器进程使用当前宿主机用户的身份，避免容器在挂载目录中生成属于 `root` 的文件。未显式传入时，两者默认使用 `1000`。

进入开发环境：

```bash
docker compose exec dev bash
```

仓库在容器内挂载到 `/workspace`，进入容器后可以使用标准 CMake 工作流：

```bash
cmake -S . -B build
cmake --build build
```

当前仓库根目录还没有 `CMakeLists.txt` 时，第一条命令失败是正常现象；完成第二阶段的第一个项目后再执行即可。

## 检查工具

进入容器后可以查看各工具版本：

```bash
cmake --version
g++ --version
clang++ --version
make --version
ninja --version
gdb --version
```

## 停止环境

退出容器 shell：

```bash
exit
```

停止并删除开发容器和默认网络：

```bash
docker compose down
```

仓库文件位于宿主机挂载目录中，不会随容器删除。构建镜像会继续保留，后续可以直接复用。

## 常用操作

重新构建镜像：

```bash
docker compose build
```

查看容器状态：

```bash
docker compose ps
```

不启动长期运行的容器，临时打开一个 shell：

```bash
DEV_UID=$(id -u) DEV_GID=$(id -g) docker compose run --rm dev bash
```
