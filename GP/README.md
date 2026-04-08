# 🌍 GPS-Based Global Planner (ROS2)

## 📌 Overview

This project implements a GPS-based global navigation system for a differential drive robot in Gazebo using ROS2.
The robot receives geographic coordinates (latitude, longitude) as input and autonomously navigates to the target location.

The system converts GPS coordinates into local ENU (East-North-Up) coordinates and uses odometry feedback to control motion in a closed-loop manner.

---

## 🎯 Objective

* Accept GPS coordinates as input
* Convert GPS → local coordinates
* Navigate robot autonomously to goal
* Implement stable and smooth motion control

---

## ⚙️ ROS Node

### 🔹 `global_planner_node`

This is the main node responsible for navigation.

#### Subscribes to:

* `/tars/odom` (`nav_msgs/msg/Odometry`)
  → Provides robot position and orientation

* `/goal_gps` (`geometry_msgs/msg/Point`)
  → Target GPS coordinates (latitude, longitude)

#### Publishes:

* `/tars/cmd_vel` (`geometry_msgs/msg/Twist`)
  → Velocity commands for robot motion

---

## 📡 Topics and Messages

| Topic           | Message Type              | Description                |
| --------------- | ------------------------- | -------------------------- |
| `/tars/odom`    | `nav_msgs/msg/Odometry`   | Robot pose and orientation |
| `/goal_gps`     | `geometry_msgs/msg/Point` | Input GPS goal             |
| `/tars/cmd_vel` | `geometry_msgs/msg/Twist` | Velocity commands          |

---

## 🧠 Working Principle

1. User sends GPS goal via `/goal_gps`
2. Node converts GPS → ENU coordinates
3. Robot rotates to face the goal
4. Robot moves toward the goal
5. Stops when within threshold distance

The system uses a **state machine**:

* IDLE → ROTATING → DRIVING → DONE

---

## 📊 RQT Graph

The RQT graph shows the communication between nodes and topics.

### 🔹 Key Flow:

* `/goal_gps` → `global_planner_node`
* `/tars/odom` → `global_planner_node`
* `global_planner_node` → `/tars/cmd_vel`
* `/tars/cmd_vel` → `/tars/diff_drive`
* `/tars/diff_drive` → `/tars/odom`

This forms a **closed-loop navigation system**.

![RQT Graph](images/rqt_graph.png)

---

## 🎥 Demo Video

📺 YouTube Link: https://youtu.be/4G9wQb5Gf2w
---

## 🚀 How to Run

### Build:

```bash
colcon build --packages-select global_planner
source install/setup.bash
```

### Launch:

```bash
ros2 launch global_planner global_planner.launch.py
```

### Send Goal:

```bash
ros2 topic pub /goal_gps geometry_msgs/msg/Point "{x: 12.9717, y: 77.5947, z: 0.0}"
```

---

## ✨ Features

* GPS to ENU coordinate transformation
* Odometry-based feedback control
* Smooth and stable navigation
* State machine implementation
* Fully ROS2-compliant architecture

---

## ⚠️ Challenges Faced

* Topic mismatch (`/odom` vs `/tars/odom`)
* Frame consistency between GPS and odometry
* Control instability and oscillations
* Debugging ROS communication

---

## ✅ Solutions

* Corrected topic names
* Ensured consistent coordinate frames
* Implemented smooth control logic
* Used RQT graph for debugging

---

## 🧩 Conclusion

This project demonstrates a complete GPS-based navigation pipeline in ROS2, including perception, planning, and control.
It lays the foundation for advanced systems such as Nav2 and autonomous robotics applications.

---
