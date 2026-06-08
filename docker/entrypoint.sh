#!/usr/bin/env bash
set -e

source "/opt/ros/${ROS_DISTRO}/setup.bash"

if [ -f "/workspace/robot_ws/install/setup.bash" ]; then
  source "/workspace/robot_ws/install/setup.bash"
fi

export RMW_IMPLEMENTATION="${RMW_IMPLEMENTATION:-rmw_cyclonedds_cpp}"
export ROS_DOMAIN_ID="${ROS_DOMAIN_ID:-0}"
export FOXGLOVE_PORT="${FOXGLOVE_PORT:-8765}"

exec "$@"

