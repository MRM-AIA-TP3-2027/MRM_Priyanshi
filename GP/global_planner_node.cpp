#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <gazebo_msgs/srv/spawn_entity.hpp>

#include <cmath>

using std::placeholders::_1;

// ───────────────────────────────────────────────
static constexpr double EARTH_R = 6371000.0;
static constexpr double RAD = M_PI / 180.0;

static constexpr double REF_LAT = 12.9716;
static constexpr double REF_LON = 77.5946;

// ───────────────────────────────────────────────
enum class State { IDLE, ROTATING, DRIVING, DONE };

struct ENU { double east, north; };

// GPS → ENU
ENU gpsToENU(double lat, double lon)
{
    double dlat = (lat - REF_LAT) * RAD;
    double dlon = (lon - REF_LON) * RAD;
    double mid  = ((lat + REF_LAT)/2.0) * RAD;

    return {
        EARTH_R * std::cos(mid) * dlon,
        EARTH_R * dlat
    };
}

// Fix: removed extra 0
double wrapAngle(double a)
{
    while (a > M_PI) a -= 2*M_PI;
    while (a < -M_PI) a += 2*M_PI;
    return a;
}

double quatToYaw(double x, double y, double z, double w)
{
    return std::atan2(2*(w*z + x*y), 1 - 2*(y*y + z*z));
}

// ───────────────────────────────────────────────
class GlobalPlanner : public rclcpp::Node
{
public:
    GlobalPlanner() : Node("global_planner_node")
    {
        cmd_pub_ = create_publisher<geometry_msgs::msg::Twist>("/tars/cmd_vel", 10);

        odom_sub_ = create_subscription<nav_msgs::msg::Odometry>(
            "/tars/odom", 10,
            std::bind(&GlobalPlanner::odomCb, this, _1));

        goal_sub_ = create_subscription<geometry_msgs::msg::Point>(
            "/goal_gps", 10,
            std::bind(&GlobalPlanner::goalCb, this, _1));

        spawn_client_ = create_client<gazebo_msgs::srv::SpawnEntity>("/spawn_entity");

        timer_ = create_wall_timer(
            std::chrono::milliseconds(100),
            std::bind(&GlobalPlanner::loop, this));

        RCLCPP_INFO(get_logger(), "🚀 Global Planner READY");
    }

private:
    // ───────────────────────────────────────────────
    State state_ = State::IDLE;

    double cur_x_=0, cur_y_=0, cur_yaw_=0;
    double smooth_yaw_=0;
    bool yaw_init_=false;
    bool odom_ok_=false;

    double start_x_=0, start_y_=0;
    bool start_set_=false;

    double goal_x_=0, goal_y_=0;

    // ROS
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_pub_;
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
    rclcpp::Subscription<geometry_msgs::msg::Point>::SharedPtr goal_sub_;
    rclcpp::Client<gazebo_msgs::srv::SpawnEntity>::SharedPtr spawn_client_;
    rclcpp::TimerBase::SharedPtr timer_;

    // ───────────────────────────────────────────────
    void odomCb(const nav_msgs::msg::Odometry::SharedPtr msg)
    {
        cur_x_ = msg->pose.pose.position.x;
        cur_y_ = msg->pose.pose.position.y;

        double raw = quatToYaw(
            msg->pose.pose.orientation.x,
            msg->pose.pose.orientation.y,
            msg->pose.pose.orientation.z,
            msg->pose.pose.orientation.w);

        if (!yaw_init_) {
            smooth_yaw_ = raw;
            yaw_init_ = true;
        } else {
            double diff = wrapAngle(raw - smooth_yaw_);
            smooth_yaw_ = wrapAngle(smooth_yaw_ + 0.15 * diff);
        }

        cur_yaw_ = smooth_yaw_;
        odom_ok_ = true;
    }

    // ───────────────────────────────────────────────
    void goalCb(const geometry_msgs::msg::Point::SharedPtr msg)
    {
        ENU g = gpsToENU(msg->x, msg->y);

        goal_x_ = g.east;
        goal_y_ = g.north;

        // Save start position once
        if (!start_set_) {
            start_x_ = cur_x_;
            start_y_ = cur_y_;
            start_set_ = true;
        }

        state_ = State::ROTATING;

        spawnMarker(goal_x_, goal_y_);

        RCLCPP_INFO(get_logger(),
            "🎯 Goal: lat=%.6f lon=%.6f → ENU(%.2f, %.2f)",
            msg->x, msg->y, goal_x_, goal_y_);
    }

    // ───────────────────────────────────────────────
    void loop()
    {
        if (!odom_ok_) return;

        switch(state_)
        {
            case State::IDLE: break;
            case State::ROTATING: rotate(); break;
            case State::DRIVING: drive(); break;
            case State::DONE: stop(); break;
        }
    }

    // ───────────────────────────────────────────────
    double dist()
    {
        return hypot(goal_x_-cur_x_, goal_y_-cur_y_);
    }

    double crossTrackError()
    {
        double num = fabs((goal_y_ - start_y_) * cur_x_
                        - (goal_x_ - start_x_) * cur_y_
                        + goal_x_ * start_y_
                        - goal_y_ * start_x_);

        double den = hypot(goal_y_ - start_y_, goal_x_ - start_x_);

        return num / den;
    }

    double bearing()
    {
        return atan2(goal_y_-cur_y_, goal_x_-cur_x_);
    }

    // ───────────────────────────────────────────────
    void rotate()
    {
        double err = wrapAngle(bearing() - cur_yaw_);

        if (fabs(err) < 0.1)
        {
            stop();
            state_ = State::DRIVING;
            return;
        }

        geometry_msgs::msg::Twist cmd;

        double scale = std::min(1.0, fabs(err)/0.3);
        cmd.angular.z = std::copysign(0.6 * scale, err);

        cmd_pub_->publish(cmd);
    }

    // ───────────────────────────────────────────────
    void drive()
    {
        double d = dist();
        double cte = crossTrackError();

        RCLCPP_INFO(get_logger(),
            "Residual: %.3f m | Cross-track: %.3f m",
            d, cte);

        double err = wrapAngle(bearing() - cur_yaw_);

        if (d < 0.5)
        {
            stop();
            state_ = State::DONE;
            RCLCPP_INFO(get_logger(), "✅ GOAL REACHED");
            return;
        }

        if (fabs(err) > 0.5)
        {
            state_ = State::ROTATING;
            return;
        }

        geometry_msgs::msg::Twist cmd;

        double scale = std::min(1.0, d/2.0);
        cmd.linear.x = 0.4 * std::max(scale, 0.3);

        double corrected = (fabs(err) < 0.1) ? 0.0 : err;
        cmd.angular.z = 0.4 * corrected;

        cmd_pub_->publish(cmd);
    }

    void stop()
    {
        cmd_pub_->publish(geometry_msgs::msg::Twist());
    }

    // ───────────────────────────────────────────────
    void spawnMarker(double x, double y)
    {
        if (!spawn_client_->wait_for_service(std::chrono::seconds(2))) return;

        auto req = std::make_shared<gazebo_msgs::srv::SpawnEntity::Request>();

        req->name = "goal_marker";
        req->xml =
            "<sdf version='1.6'>"
            "<model name='goal'>"
            "<static>true</static>"
            "<link name='link'>"
            "<visual name='vis'>"
            "<geometry><sphere><radius>0.4</radius></sphere></geometry>"
            "<material><ambient>1 0 0 1</ambient></material>"
            "</visual>"
            "</link>"
            "</model>"
            "</sdf>";

        req->initial_pose.position.x = x;
        req->initial_pose.position.y = y;
        req->initial_pose.position.z = 0.4;

        spawn_client_->async_send_request(req);
    }
};

// ───────────────────────────────────────────────
int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<GlobalPlanner>());
    rclcpp::shutdown();
    return 0;
}
