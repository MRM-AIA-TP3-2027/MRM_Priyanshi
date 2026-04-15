from launch import LaunchDescription
from launch_ros.parameter_descriptions import ParameterValue
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import Command, LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
import os

def generate_launch_description():

    # Package setup
    desc_pkg = FindPackageShare("robomechtars_description")
    gazebo_pkg = FindPackageShare("gazebo_ros")

    # File paths
    xacro_file = PathJoinSubstitution([desc_pkg, "urdf", "TARS.xacro"])
    # Pointing to your newly created maze world
    world_file = PathJoinSubstitution([desc_pkg, "worlds", "tars_maze.world"])
    gazebo_launch = PathJoinSubstitution([gazebo_pkg, "launch", "gazebo.launch.py"])

    use_sim_time = LaunchConfiguration("use_sim_time")

    return LaunchDescription([

        DeclareLaunchArgument("use_sim_time", default_value="true"),

        # 1. Launch Gazebo with your Maze
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(gazebo_launch),
            launch_arguments={"world": world_file}.items(),
        ),

        # 2. Robot State Publisher (Broadcasts TARS URDF)
        Node(
            package="robot_state_publisher",
            executable="robot_state_publisher",
            parameters=[{
                "robot_description": ParameterValue(
                    Command(["xacro ", xacro_file]), 
                    value_type=str
                ),
                "use_sim_time": use_sim_time
            }],
            output="screen"
        ),

        # 3. Spawn TARS inside the maze
        # Added -x, -y, and -z arguments to control spawn location
        TimerAction(
            period=3.0,
            actions=[
                Node(
                    package="gazebo_ros",
                    executable="spawn_entity.py",
                    arguments=[
                        "-topic", "robot_description",
                        "-entity", "TARS",
                        "-x", "0.0", 
                        "-y", "0.0", 
                        "-z", "0.1" 
                    ],
                    output="screen"
                )
            ]
        ),

        # 4. Obstacle Avoidance Brain
        Node(
            package="obstacle_avoidance",
            executable="obstacle_avoidance_node",
            output="screen"
        )
    ])
