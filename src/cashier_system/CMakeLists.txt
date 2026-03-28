cmake_minimum_required(VERSION 3.8)
project(cashier_system)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(std_msgs REQUIRED)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/Bill.msg"
)
add_executable(billing_node src/billing_node.cpp)
add_executable(inventory_node src/inventory_node.cpp)
add_executable(display_node src/display_node.cpp)

ament_target_dependencies(billing_node rclcpp)
ament_target_dependencies(inventory_node rclcpp std_msgs)
ament_target_dependencies(display_node rclcpp std_msgs)

rosidl_target_interfaces(billing_node ${PROJECT_NAME} "rosidl_typesupport_cpp")
rosidl_target_interfaces(inventory_node ${PROJECT_NAME} "rosidl_typesupport_cpp")

install(TARGETS
  billing_node
  inventory_node
  display_node
  DESTINATION lib/${PROJECT_NAME}
)
ament_package()
