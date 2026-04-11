
import os

from launch import LaunchDescription
from launch.actions import ExecuteProcess, TimerAction
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import xacro


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
                    '-entity', 'TARS_NEW'
                ],
                output='screen'
            )
        ]
    )

   
    # ─── 4. GLOBAL PLANNER NODE ───────────────────────────
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
        planner
    ])
