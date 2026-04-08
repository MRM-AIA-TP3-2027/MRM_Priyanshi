import math
import os

from launch import LaunchDescription
from launch.actions import ExecuteProcess, TimerAction
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import xacro

# ─────────────────────────────────────────────────────────
# 🌍 GPS → XY (MUST MATCH C++ NODE)
# ─────────────────────────────────────────────────────────
REF_LAT = 12.9716
REF_LON = 77.5946
R = 6371000.0

def gps_to_xy(lat, lon):
    lat_r = math.radians(lat)
    lon_r = math.radians(lon)
    ref_lat_r = math.radians(REF_LAT)
    ref_lon_r = math.radians(REF_LON)

    x = (lon_r - ref_lon_r) * math.cos(ref_lat_r) * R
    y = (lat_r - ref_lat_r) * R
    return x, y

# ✅ Test point (visible marker)
TEST_LAT = 12.9717
TEST_LON = 77.5947
TEST_X, TEST_Y = gps_to_xy(TEST_LAT, TEST_LON)

# ─────────────────────────────────────────────────────────
def generate_launch_description():

    # ─── Robot description ────────────────────────────────
    pkg_desc = get_package_share_directory('robomechtars_description')
    xacro_file = os.path.join(pkg_desc, 'urdf', 'TARS.xacro')

    robot_description = xacro.process_file(xacro_file).toxml()

    # ─── 1. Start Gazebo (WITH ROS plugins) ───────────────
    gazebo = ExecuteProcess(
        cmd=[
            'gazebo',
            '--verbose',
            '-s', 'libgazebo_ros_factory.so',
            '-s', 'libgazebo_ros_init.so'
        ],
        output='screen'
    )

    # ─── 2. Robot State Publisher ─────────────────────────
    rsp = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        parameters=[{'robot_description': robot_description}],
        output='screen'
    )

    # ─── 3. Spawn Robot (AFTER GAZEBO LOADS) ──────────────
    spawn_robot = TimerAction(
        period=4.0,
        actions=[
            Node(
                package='gazebo_ros',
                executable='spawn_entity.py',
                arguments=[
                    '-topic', 'robot_description',
                    '-entity', 'TARS'
                ],
                output='screen'
            )
        ]
    )

    # ─── 4. GREEN TEST MARKER ─────────────────────────────
    marker_script = f"""
import rclpy
from rclpy.node import Node
from visualization_msgs.msg import Marker
import time

rclpy.init()
node = Node('test_marker')

pub = node.create_publisher(
    Marker,
    '/test_marker',
    10
)

time.sleep(1.0)

m = Marker()
m.header.frame_id = 'odom'
m.header.stamp = node.get_clock().now().to_msg()
m.ns = 'test'
m.id = 1
m.type = Marker.SPHERE
m.action = Marker.ADD

m.pose.position.x = {TEST_X}
m.pose.position.y = {TEST_Y}
m.pose.position.z = 0.5

m.pose.orientation.w = 1.0

m.scale.x = 1.0
m.scale.y = 1.0
m.scale.z = 1.0

m.color.r = 0.0
m.color.g = 1.0
m.color.b = 0.0
m.color.a = 1.0

for _ in range(5):
    pub.publish(m)
    time.sleep(0.2)

node.get_logger().info("GREEN marker spawned")
rclpy.shutdown()
"""

    marker = TimerAction(
        period=6.0,
        actions=[
            ExecuteProcess(
                cmd=['python3', '-c', marker_script],
                output='screen'
            )
        ]
    )

    # ─── 5. GLOBAL PLANNER NODE ───────────────────────────
    planner = TimerAction(
        period=8.0,
        actions=[
            Node(
                package='global_planner',   
                executable='global_planner_node',
                name='global_planner_node',
                output='screen',
                emulate_tty=True
            )
        ]
    )

    # ─────────────────────────────────────────────────────
    return LaunchDescription([
        gazebo,
        rsp,
        spawn_robot,
        marker,
        planner
    ])
