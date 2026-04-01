#include "rclcpp/rclcpp.hpp"
#include "cashier_system/msg/bill.hpp"
#include <iostream>

using namespace std::chrono_literals;

class BillingNode : public rclcpp::Node {
public:
    BillingNode() : Node("billing_node") {
        publisher_ = this->create_publisher<cashier_system::msg::Bill>("bill", 10);

        timer_ = this->create_wall_timer(
            5s, std::bind(&BillingNode::generate_bill, this));
    }

private:
    void generate_bill() {
        auto msg = cashier_system::msg::Bill();

        std::string item;
        int qty;

        std::cout << "\nEnter item: ";
        std::cin >> item;
        std::cout << "Quantity: ";
        std::cin >> qty;

        msg.item_name = item;
        msg.quantity = qty;
        msg.price = 0.0;  

        publisher_->publish(msg);
    }

    rclcpp::Publisher<cashier_system::msg::Bill>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<BillingNode>());
    rclcpp::shutdown();
    return 0;
}
