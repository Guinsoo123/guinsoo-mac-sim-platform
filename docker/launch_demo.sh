#!/usr/bin/env bash
set -e

source "/opt/ros/${ROS_DISTRO:-jazzy}/setup.bash"

if [ -f "/workspace/robot_ws/install/setup.bash" ]; then
  source "/workspace/robot_ws/install/setup.bash"
fi

export RMW_IMPLEMENTATION="${RMW_IMPLEMENTATION:-rmw_cyclonedds_cpp}"
export ROS_DOMAIN_ID="${ROS_DOMAIN_ID:-0}"
export FOXGLOVE_PORT="${FOXGLOVE_PORT:-8765}"

echo "启动 Foxglove Bridge，端口：${FOXGLOVE_PORT}"
ros2 launch foxglove_bridge foxglove_bridge_launch.xml port:="${FOXGLOVE_PORT}" &
BRIDGE_PID=$!

echo "启动 ROS 2 demo topic：/chatter"
ros2 run demo_nodes_cpp talker &
TALKER_PID=$!

echo "启动 Gazebo 空世界服务端，用于验证 Gazebo Harmonic 后端依赖"
gz sim -s empty.sdf &
GAZEBO_PID=$!

trap 'kill ${BRIDGE_PID} ${TALKER_PID} ${GAZEBO_PID} 2>/dev/null || true' EXIT
wait -n
