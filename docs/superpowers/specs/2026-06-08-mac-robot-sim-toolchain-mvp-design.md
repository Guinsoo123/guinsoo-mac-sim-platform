# Mac 全栈机器人仿真平台：工具链环境 MVP 设计

## 摘要

第一版聚焦“工具链环境 MVP”：在 Apple Silicon Mac 上，用 Qt/C++ 桌面 GUI 管理 Docker Desktop 中的 Ubuntu 24.04 ARM64 容器。容器内提供 ROS 2 Jazzy、Gazebo Harmonic、MoveIt 2 Jazzy、本地 ROS 工作区挂载和 Foxglove Bridge。Mac 端不直接安装或链接 ROS 2，而是通过 WebSocket 读取容器内桥接出的 ROS 数据。

## 关键设计

- GUI 技术栈：Qt/C++，面向 macOS 桌面体验，同时保留未来跨平台潜力。
- 默认运行时：Docker Desktop；第一版暂不支持 Colima、Intel Mac、多架构镜像。
- 镜像策略：项目内 Dockerfile 本地构建并缓存，不依赖预构建镜像发布；Ubuntu、ROS、OSRF 软件源通过 build arg 参数化，默认脚本优先兼顾国内网络下的 ARM64 包下载稳定性。
- 版本基线：Ubuntu 24.04 ARM64、ROS 2 Jazzy、Gazebo Harmonic、MoveIt 2 Jazzy。
- 数据链路：Docker API/CLI 负责控制面，Foxglove Bridge 负责 ROS topic、TF、状态数据的数据面。
- 工作区模型：用户在 GUI 中选择一个本地 ROS 工作区目录，挂载到容器内运行。

## GUI 与配置

GUI 包含环境概览、设置中心、镜像构建、仿真会话、实时数据五个页面。设置中心采用“全局默认 + 项目覆盖”模型，覆盖 Docker 路径/状态、HTTP/HTTPS 代理、资源上限、端口、镜像标签、Ubuntu/ROS/OSRF 软件源、工作区挂载路径、ROS_DOMAIN_ID、RMW/DDS、Gazebo 资源路径、Foxglove Bridge、日志级别。

用户流程为：检查依赖 -> 配置环境 -> 构建镜像 -> 启动容器 -> 启动 Gazebo demo -> 连接 Foxglove Bridge -> 查看实时 topic 数据。

## 诊断与错误处理

诊断分层展示 Mac 依赖、Docker Desktop、镜像构建、容器健康、ROS 环境、Gazebo demo、Foxglove Bridge。每个失败项显示中文原因、关键原始日志片段和建议动作。控制面和数据面分离：Bridge 连接失败时，GUI 仍保留容器日志、健康检查和诊断入口。

## 测试与验收

验收环境为 Apple Silicon Mac + Docker Desktop。测试场景包括配置保存/项目覆盖、镜像本地构建、容器启动、Gazebo demo launch、Foxglove Bridge WebSocket 连接、topic 列表刷新、失败诊断展示。

成功标准：GUI 能从零构建工具链环境，启动 Gazebo + ROS topics 示例，并显示至少一个实时 ROS topic 数值。

## 暂缓范围

完整 RViz/Gazebo GUI 嵌入、rosbag 分析、多 Docker 运行时、Intel Mac、macOS 端 DDS 直连 ROS 2 节点、自动修复。
