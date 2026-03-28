#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class DisplayNode : public rclcpp::Node {
public:
    DisplayNode() : Node("display_node") {
        subscription_ = this->create_subscription<std_msgs::msg::String>(
            "status", 10,
            std::bind(&DisplayNode::callback, this, std::placeholders::_1));
    }

private:
    void callback(const std_msgs::msg::String::SharedPtr msg) {
        RCLCPP_INFO(this->get_logger(), "%s", msg->data.c_str());
    }

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DisplayNode>());
    rclcpp::shutdown();
    return 0;
}
