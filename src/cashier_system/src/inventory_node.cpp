#include "rclcpp/rclcpp.hpp"
#include "cashier_system/msg/bill.hpp"
#include "std_msgs/msg/string.hpp"
#include <map>

class InventoryNode : public rclcpp::Node {
public:
    InventoryNode() : Node("inventory_node") {
        subscription_ = this->create_subscription<cashier_system::msg::Bill>(
            "bill", 10,
            std::bind(&InventoryNode::callback, this, std::placeholders::_1));

        publisher_ = this->create_publisher<std_msgs::msg::String>("status", 10);

        inventory_["apple"] = 50;
        inventory_["banana"] = 40;
        inventory_["milk"] = 30;
        inventory_["bread"] = 25;
        inventory_["eggs"] = 60;
        inventory_["rice"] = 100;
        inventory_["wheat"] = 80;
        inventory_["sugar"] = 70;
        inventory_["salt"] = 90;
        inventory_["oil"] = 45;
        inventory_["butter"] = 20;
        inventory_["cheese"] = 15;
        inventory_["chocolate"] = 35;
        inventory_["biscuits"] = 55;
        inventory_["soap"] = 40;
        inventory_["shampoo"] = 30;
        inventory_["toothpaste"] = 50;
        inventory_["detergent"] = 25;
        inventory_["tea"] = 60;
        inventory_["coffee"] = 45;

        prices_["apple"] = 10;
        prices_["banana"] = 5;
        prices_["milk"] = 25;
        prices_["bread"] = 30;
        prices_["eggs"] = 6;
        prices_["rice"] = 50;
        prices_["wheat"] = 40;
        prices_["sugar"] = 45;
        prices_["salt"] = 20;
        prices_["oil"] = 120;
        prices_["butter"] = 60;
        prices_["cheese"] = 80;
        prices_["chocolate"] = 25;
        prices_["biscuits"] = 10;
        prices_["soap"] = 30;
        prices_["shampoo"] = 150;
        prices_["toothpaste"] = 90;
        prices_["detergent"] = 200;
        prices_["tea"] = 100;
        prices_["coffee"] = 250;
    }

private:
    void callback(const cashier_system::msg::Bill::SharedPtr msg) {
        std_msgs::msg::String status;

        if (inventory_.find(msg->item_name) == inventory_.end()) {
            status.data = "Item not found";
            publisher_->publish(status);
            return;
        }

        if (inventory_[msg->item_name] < msg->quantity) {
            status.data = "Not enough stock";
            publisher_->publish(status);
            return;
        }

        inventory_[msg->item_name] -= msg->quantity;

        float price = prices_[msg->item_name];
        income_ += msg->quantity * price;

        std::string data = "\n===== SUPERMARKET STATUS =====\n";

        for (auto &item : inventory_) {
            data += item.first + " : " + std::to_string(item.second) + "\n";
        }

        data += "-----------------------------\n";
        data += "Total Income: " + std::to_string(income_) + "\n";

        status.data = data;

        publisher_->publish(status);
    }

    std::map<std::string, int> inventory_;
    std::map<std::string, float> prices_;
    float income_ = 0;

    rclcpp::Subscription<cashier_system::msg::Bill>::SharedPtr subscription_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<InventoryNode>());
    rclcpp::shutdown();
    return 0;
}
