
# TARS Obstacle Avoidance System (ROS2)

---

## 1. Description of Task

This project implements an autonomous navigation system for the TARS robot using ROS2. The objective is to enable the robot to navigate toward a GPS-defined goal while avoiding obstacles in real time.

The system combines global navigation (GPS + IMU) with local obstacle avoidance (LiDAR and depth camera). It is designed to operate robustly in structured and unstructured environments, including narrow passages and U-shaped obstacles.

### Key Functionalities

* Continuous goal tracking using GPS and IMU
* Real-time obstacle detection using LiDAR and depth camera
* Sensor fusion for reliable obstacle perception
* State-based navigation logic
* Recovery from local minima using wall-following and escape strategies

---

## 2. Description of ROS Nodes, Topics, and Messages

### Main Node

* `obstacle_avoidance_node`
  This node is responsible for:

  * Processing sensor inputs
  * Computing navigation commands
  * Handling obstacle avoidance
  * Managing the navigation state machine 

---

### Subscribed Topics

| Topic                     | Message Type            | Description                                    |
| ------------------------- | ----------------------- | ---------------------------------------------- |
| `/gps/fix`                | `sensor_msgs/NavSatFix` | Provides global position (latitude, longitude) |
| `/imu`                    | `sensor_msgs/Imu`       | Provides robot orientation (yaw)               |
| `/scan`                   | `sensor_msgs/LaserScan` | LiDAR data for obstacle detection              |
| `/camera/depth/image_raw` | `sensor_msgs/Image`     | Depth image for frontal obstacle detection     |
| `/goal_gps`               | `sensor_msgs/NavSatFix` | Target goal position                           |

---

### Published Topics

| Topic           | Message Type          | Description                         |
| --------------- | --------------------- | ----------------------------------- |
| `/tars/cmd_vel` | `geometry_msgs/Twist` | Velocity commands sent to the robot |

---

### System Components

#### Global Navigation

* Converts GPS coordinates into local ENU frame
* Computes:

  * Distance to goal
  * Bearing error
  * Cross-track error (CTE)
* Continuously active, even during obstacle avoidance

#### Local Obstacle Avoidance

* LiDAR-based Vector Field Histogram (VFH)

  * Divides environment into sectors
  * Selects optimal steering direction
* Depth camera processing

  * Uses central region of image
  * Detects frontal obstacles
* Sensor fusion

  * Uses minimum distance from LiDAR and depth

#### State Machine

The navigation system operates using the following states:

* IDLE: Waiting for goal input
* ALIGNING: Rotates robot toward goal direction
* DRIVING: Moves toward goal with obstacle avoidance
* WALL_FOLLOWING: Handles local minima using left-hand rule
* ESCAPING: Fallback strategy for complex traps
* DONE: Goal reached

---

## 3. RQT Graph

Below is the RQT graph of the system showing node and topic connections:
![RQT Graph](images/rqt_graph_oa.png)


### Explanation

* Sensor plugins in Gazebo publish:

  * `/gps/fix` from GPS plugin
  * `/imu` from IMU plugin
  * `/scan` from LiDAR plugin

* These topics are subscribed by:

  * `obstacle_avoidance_node`

* The node processes all inputs and publishes:

  * `/tars/cmd_vel`

* The velocity commands are consumed by:

  * `/tars/diff_drive`, which drives the robot

---

## 4. YouTube Video Link

Add your demonstration video link below:

```
https://youtu.be/Djs7e-BTQqM
```

---

## Summary

This project presents a complete autonomous navigation pipeline combining global planning and local obstacle avoidance. The system is capable of handling complex environments through a structured state machine and robust sensor fusion, ensuring reliable navigation toward a GPS-defined goal.
