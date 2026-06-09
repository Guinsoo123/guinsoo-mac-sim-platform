# 跨平台机器人仿真平台：工具链环境 MVP 设计

## 摘要

第一版聚焦“工具链环境 MVP”：在 **macOS** 或 **Ubuntu 22.04 LTS** 宿主机上，用 Qt/C++ 桌面 GUI 管理 Docker 中的 Linux 工具链容器。容器内提供 ROS 2 Jazzy、Gazebo Harmonic、MoveIt 2 Jazzy、本地 ROS 工作区挂载和 Foxglove Bridge。宿主机不直接安装或链接 ROS 2，而是通过 WebSocket 读取容器内桥接出的 ROS 数据。

**宿主机兼容目标**：同一套仓库、同一套 GUI 与配置模型，在 macOS 与 Ubuntu 22.04 上均可完成「构建镜像 → 启动容器 → 运行 demo → 查看实时 topic」闭环。

## 宿主机兼容策略

### 支持的宿主机矩阵

| 宿主机 | 默认 Docker 后端 | 默认容器架构 | 第一版 GUI |
| --- | --- | --- | --- |
| macOS（Apple Silicon） | Docker Desktop | `linux/arm64` | Qt/C++ 桌面应用 |
| Ubuntu 22.04 LTS（x86_64） | Docker Engine + Compose | `linux/amd64` | Qt/C++ 桌面应用 |
| Ubuntu 22.04 LTS（arm64） | Docker Engine + Compose | `linux/arm64` | Qt/C++ 桌面应用（次要验证目标） |

说明：用户常写「Ubuntu 22.4」，本设计统一指 **Ubuntu 22.04 LTS**。容器基线仍为 **Ubuntu 24.04**（与 ROS 2 Jazzy / Gazebo Harmonic 对齐），**宿主机版本与容器版本解耦**——22.04 宿主机只负责运行 Docker 与 GUI，不要求在宿主机安装 ROS。

### 方案对比（brainstorming 结论）

| 方案 | 做法 | 优点 | 缺点 |
| --- | --- | --- | --- |
| A. 双宿主机 + 统一容器 + 跨平台 Qt（**推荐**） | Mac/Ubuntu 均跑同一 Qt 应用；按 OS/架构选择 Docker CLI 与 build 参数 | 体验一致、与现有 MVP 结构一致、容器内 ROS 栈单一 | 需在 Linux 上验证 Qt 构建与 Docker 权限 |
| B. Mac GUI + Ubuntu 仅脚本 | Ubuntu 22.04 只提供 shell/compose，无 GUI | Linux CI 实现快 | 双端体验分裂，违背「一套环境」 |
| C. 宿主机原生 ROS（无 Docker） | 22.04 直接 apt 装 Humble/Gazebo | 性能/调试直接 | 与 Mac 路径完全不同，维护成本翻倍 |

**采用方案 A**：在现有 Mac MVP 上扩展 **Host Profile**（`macos` / `linux-ubuntu2204`），抽象 Docker 检测、镜像标签、架构与软件源默认值；不在第一版引入宿主机原生 ROS 安装路径。

### Host Profile 行为

- **macOS**：沿用 Docker Desktop；构建/运行脚本默认 ARM64 镜像与国内镜像源（现有 `build-mac-app.sh`、`docker-build-toolchain.sh` 行为）。
- **Ubuntu 22.04**：检测 `docker` / `docker compose`；默认 `linux/amd64` 构建；软件源 build arg 支持 amd64 端口镜像（如 `archive.ubuntu.com` 或国内等价源）；GUI 通过相同配置模型读写 `~/.config/guinsoo/`（路径与 Mac 一致的概念，具体目录取 OS 惯例）。
- **共享**：`compose.yaml`、Foxglove Bridge 端口、工作区挂载路径、ROS_DOMAIN_ID、诊断规则与五页 GUI 信息架构不变；差异仅体现在 Host Profile 的默认值与依赖检查项。

### 不在第一版宿主机兼容范围内

- Intel Mac 作为**独立**验收矩阵（仍属暂缓，不阻塞 Ubuntu 22.04 宿主机支持）。
- Colima、Podman 等非 Docker Desktop / Docker Engine 官方路径。
- 在宿主机上直接运行 `rclpy` / RViz / Gazebo GUI（无容器）。

## 关键设计

- GUI 技术栈：Qt/C++，面向 macOS 与 Linux（Ubuntu 22.04）桌面体验；通过 Host Profile 屏蔽路径与 Docker 后端差异。
- 默认运行时：**macOS → Docker Desktop**；**Ubuntu 22.04 → Docker Engine（及 Compose 插件）**。
- 镜像策略：项目内 Dockerfile 本地构建并缓存，不依赖预构建镜像发布；Ubuntu、ROS、OSRF 软件源通过 build arg 参数化；**按目标平台传入 `TARGETARCH`（arm64/amd64）**，默认脚本在 Mac 上优先 ARM64 与国内源，在 Ubuntu 22.04 上优先 amd64 与可配置的镜像源。
- 版本基线：容器内 Ubuntu 24.04、ROS 2 Jazzy、Gazebo Harmonic、MoveIt 2 Jazzy（两宿主机共用同一容器定义，仅架构与源镜像不同）。
- 数据链路：Docker API/CLI 负责控制面，Foxglove Bridge 负责 ROS topic、TF、状态数据的数据面。
- 工作区模型：用户在 GUI 中选择一个本地 ROS 工作区目录，挂载到容器内运行。

## GUI 与配置

GUI 包含环境概览、设置中心、镜像构建、仿真会话、实时数据五个页面。设置中心采用“全局默认 + 项目覆盖”模型，覆盖 Docker 路径/状态、HTTP/HTTPS 代理、资源上限、端口、镜像标签、Ubuntu/ROS/OSRF 软件源、工作区挂载路径、ROS_DOMAIN_ID、RMW/DDS、Gazebo 资源路径、Foxglove Bridge、日志级别；并增加 **宿主机类型（只读展示）** 与 **目标容器架构（可覆盖，默认随 Host Profile）**。

用户流程为：检查依赖（含宿主机 OS 与 Docker） -> 配置环境 -> 构建镜像 -> 启动容器 -> 启动 Gazebo demo -> 连接 Foxglove Bridge -> 查看实时 topic 数据。

## 诊断与错误处理

诊断分层展示 **宿主机 OS 与 Docker**、镜像构建、容器健康、ROS 环境、Gazebo demo、Foxglove Bridge。Mac 侧继续区分 Docker Desktop 未启动、资源不足等；Ubuntu 22.04 侧增加 **当前用户不在 docker 组**、**Compose 插件缺失**、**与宿主机架构不匹配的镜像** 等项。每个失败项显示中文原因、关键原始日志片段和建议动作。控制面和数据面分离：Bridge 连接失败时，GUI 仍保留容器日志、健康检查和诊断入口。

## 测试与验收

验收需在 **两类宿主机** 上分别通过（或 CI 中至少覆盖 Ubuntu 22.04 amd64；Mac 手工/本地 CI）：

| 场景 | macOS（Apple Silicon） | Ubuntu 22.04 LTS |
| --- | --- | --- |
| 依赖检查 | Docker Desktop 就绪 | `docker` + compose 就绪 |
| 配置保存/项目覆盖 | 通过 | 通过 |
| 镜像本地构建 | `linux/arm64` | `linux/amd64` |
| 容器启动 | 通过 | 通过 |
| Gazebo demo launch | 通过 | 通过 |
| Foxglove Bridge WebSocket | 通过 | 通过 |
| topic 列表刷新 | 通过 | 通过 |
| 失败诊断展示 | 通过 | 通过 |

成功标准：在 **任一支持的宿主机** 上，GUI 或等价脚本路径能从零构建工具链环境，启动 Gazebo + ROS topics 示例，并显示至少一个实时 ROS topic 数值；且文档明确列出两宿主机的前置条件与命令差异。

## 暂缓范围

完整 RViz/Gazebo GUI 嵌入、rosbag 分析、Colima/Podman、Intel Mac 专项优化、macOS/Ubuntu 宿主机端 DDS 直连 ROS 2 节点、自动修复、Ubuntu 22.04 以外 Linux 发行版官方支持。
