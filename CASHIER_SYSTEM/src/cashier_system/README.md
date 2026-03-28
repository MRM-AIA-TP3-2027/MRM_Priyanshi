# Cashier System using ROS2 (C++)

This project implements a basic cashier and inventory management system using ROS2 in C++. The system simulates how a supermarket billing system works by distributing functionality across multiple ROS2 nodes.

The system consists of three main nodes:

* **Billing Node**: Takes user input (item name and quantity) and publishes a bill.
* **Inventory Node**: Receives the bill, updates the inventory and total income, and publishes the updated system status.
* **Display Node**: Subscribes to system status updates and displays the current inventory and income.

The approach follows a modular ROS2 architecture where nodes communicate using topics. A custom message (`Bill.msg`) is used to transfer structured data between nodes. The inventory node maintains shared state (stock and prices) and ensures consistency across the system.

---

## ROS Topics Published

| Topic Name | Publisher Node | Subscriber Node | Description                              |
| ---------- | -------------- | --------------- | ---------------------------------------- |
| `/bill`    | Billing Node   | Inventory Node  | Sends item name and quantity             |
| `/status`  | Inventory Node | Display Node    | Sends updated inventory and total income |

---

## ROS Messages and Services Used

### Custom Message

**Bill.msg**

```
string item_name
int32 quantity
float32 price
```

* `item_name`: Name of the product
* `quantity`: Number of units purchased
* `price`: Not used in final logic (price is handled in inventory)

---

### Standard Messages

* `std_msgs/msg/String`

  * Used to send formatted inventory and income data from Inventory Node to Display Node

---

### Services

* No ROS services were used in this implementation

---

## 📊 RQT Graph

The system architecture can be visualized using:

```
rqt_graph
```

Expected graph structure:

```
Billing Node → /bill → Inventory Node → /status → Display Node
```

This shows how data flows between nodes using ROS topics.

---

## Demo Video

(Add your YouTube link here after recording)

---

## How to Run

Open three terminals and run the following:

### Terminal 1:

```
ros2 run cashier_system display_node
```

### Terminal 2:

```
ros2 run cashier_system inventory_node
```

### Terminal 3:

```
ros2 run cashier_system billing_node
```

---

## Key Features

* Multi-node ROS2 architecture
* Custom message communication
* Real-time inventory updates
* Fixed pricing system
* Input validation (item existence and stock availability)

---
## RQT Graph Screenshot
![RQT Graph](rqt_graph.png)

## Video link
https://youtu.be/7TRpg1vHo1g

