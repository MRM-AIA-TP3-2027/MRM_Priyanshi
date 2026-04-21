#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <sensor_msgs/msg/nav_sat_fix.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>

#include <cmath>
#include <deque>
#include <algorithm>
#include <vector>
#include <random>
#include <string>
#include <limits>

using std::placeholders::_1;

// ═══════════════════════════════════════════════════════════════════════════════
//  Constants
// ═══════════════════════════════════════════════════════════════════════════════
static constexpr double EARTH_R  = 6371000.0;
static constexpr double RAD      = M_PI / 180.0;
static constexpr double DEG      = 180.0 / M_PI;

// Obstacle thresholds (metres)
static constexpr double STOP_D   = 0.55;   // emergency backup
static constexpr double SLOW_D   = 0.80;   // start avoidance / slow down

// LiDAR VFH sectors
static constexpr int    NSEC     = 36;     // 10° per sector

// Goal / heading tolerance
static constexpr double GOAL_TOL = 0.40;   // metres  (matches global_planner)
static constexpr double HEAD_TOL = 0.08;   // radians (~4.5°)

// Depth camera: centre strip fraction used for obstacle check
static constexpr double DEPTH_STRIP = 0.30; // central 30% columns
static constexpr double DEPTH_MAX   = 8.0;  // ignore readings beyond this

// Wall-Following parameters
static constexpr double WF_WALL_DIST     = 0.70;  // desired distance to left wall [m]
static constexpr double WF_WALL_GAIN     = 1.2;   // P-gain for wall-distance error
static constexpr double WF_FRONT_TURN    = 0.65;  // front dist that triggers a right turn [m]
static constexpr double CLEAR_BEARING_TOL = 1.05; // ~60° — max |bearingError| to exit WF [rad]
static constexpr double WF_MIN_TRAVEL    = 0.60;  // min metres moved before WF can exit [m]

// ═══════════════════════════════════════════════════════════════════════════════
//  State Machine
// ═══════════════════════════════════════════════════════════════════════════════
enum class State { IDLE, ALIGNING, DRIVING, WALL_FOLLOWING, ESCAPING, DONE };

// ═══════════════════════════════════════════════════════════════════════════════
//  ENU helper
// ═══════════════════════════════════════════════════════════════════════════════
struct ENU { double east = 0, north = 0; };

// ═══════════════════════════════════════════════════════════════════════════════
//  GPSPose  — position from /gps/fix  (lat/lon → local ENU)
//             heading from /imu
// ═══════════════════════════════════════════════════════════════════════════════
class GPSPose
{
    double ref_lat_ = 0, ref_lon_ = 0;
    bool   origin_  = false;
    double e_ = 0, n_ = 0, yaw_ = 0;
    bool   gps_ok_ = false, imu_ok_ = false;

public:
    // ── GPS callback ──────────────────────────────────────────────────────────
    void updateGPS(const sensor_msgs::msg::NavSatFix & msg)
    {
        if (!origin_) {
            ref_lat_ = msg.latitude;
            ref_lon_ = msg.longitude;
            origin_  = true;
        }
        ENU p = toENU(msg.latitude, msg.longitude);
        e_ = p.east;  n_ = p.north;
        gps_ok_ = true;
    }

    // ── IMU callback ──────────────────────────────────────────────────────────
    void updateIMU(const sensor_msgs::msg::Imu & msg)
    {
        tf2::Quaternion q(msg.orientation.x, msg.orientation.y,
                          msg.orientation.z, msg.orientation.w);
        double r, p, yaw;
        tf2::Matrix3x3(q).getRPY(r, p, yaw);
        // Apply same +π correction used in global_planner_node.cpp
        yaw_ = wrap(yaw + M_PI);
        imu_ok_ = true;
    }

    ENU toENU(double lat, double lon) const
    {
        double dlat = (lat - ref_lat_) * RAD;
        double dlon = (lon - ref_lon_) * RAD;
        double mid  = ((lat + ref_lat_) / 2.0) * RAD;
        return { EARTH_R * std::cos(mid) * dlon,
                 EARTH_R * dlat };
    }

    bool   ready()     const { return gps_ok_ && imu_ok_ && origin_; }
    bool   originSet() const { return origin_; }
    double e()         const { return e_;   }
    double n()         const { return n_;   }
    double yaw()       const { return yaw_; }

    static double wrap(double a)
    {
        while (a >  M_PI) a -= 2 * M_PI;
        while (a < -M_PI) a += 2 * M_PI;
        return a;
    }
};

// ═══════════════════════════════════════════════════════════════════════════════
//  LidarVFH  — Vector Field Histogram from /scan
//
//  Sector layout  (NSEC=36, 10° each, robot forward = 0°)
//  ─────────────────────────────────────────────────────
//  Robot forward (+X) maps to sector index 18 (angle ≈ 0°).
//  Left  (+Y in robot frame) maps to sectors ≈ 5–13  ( 45°–135°).
//  Right (-Y in robot frame) maps to sectors ≈ 23–31 (-45°–-135°).
//
//  left_dist()  : min of sectors  5..13  (left  side wall detection)
//  right_dist() : min of sectors 23..31  (right side wall detection)
// ═══════════════════════════════════════════════════════════════════════════════
class LidarVFH
{
    std::vector<float> sec_;
    bool ok_ = false;

    int angleToSector(float angle) const
    {
        float norm = angle + static_cast<float>(M_PI);  // [0, 2π)
        int s = static_cast<int>(norm / (2 * M_PI) * NSEC);
        return std::clamp(s, 0, NSEC - 1);
    }

public:
    LidarVFH() : sec_(NSEC, 99.f) {}

    void update(const sensor_msgs::msg::LaserScan & scan)
    {
        std::fill(sec_.begin(), sec_.end(), 99.f);
        int n = static_cast<int>(scan.ranges.size());
        for (int i = 0; i < n; ++i) {
            float r = scan.ranges[i];
            if (!std::isfinite(r) || r < scan.range_min || r > scan.range_max)
                continue;
            float ang = scan.angle_min + i * scan.angle_increment;
            int   s   = angleToSector(ang);
            sec_[s]   = std::min(sec_[s], r);
        }
        ok_ = true;
    }

    // Angle (robot frame) of sector s
    static double sectorAngle(int s)
    {
        return -M_PI + (s + 0.5) * (2 * M_PI / NSEC);
    }

    // Obstacle closer than thr in the ±25° frontal cone?
    bool frontBlocked(double thr) const
    {
        if (!ok_) return false;
        for (int s = 0; s < NSEC; ++s)
            if (std::abs(GPSPose::wrap(sectorAngle(s))) < M_PI / 7.2 &&
                sec_[s] < thr)
                return true;
        return false;
    }

    // Min distance in ±25° frontal cone
    float frontDist() const
    {
        float md = 99.f;
        for (int s = 0; s < NSEC; ++s)
            if (std::abs(GPSPose::wrap(sectorAngle(s))) < M_PI / 7.2)
                md = std::min(md, sec_[s]);
        return md;
    }

    // ── NEW: Left side wall distance ──────────────────────────────────────────
    // Sectors covering the robot's left flank (45° to 135° in robot frame).
    // These sectors detect walls to the left during maze traversal.
    float left_dist() const
    {
        float md = 99.f;
        // Sector angles sweep from -π. Left side = positive angles ≈ π/4 to 3π/4.
        // For NSEC=36: sector 5 ≈ +50°, sector 13 ≈ +130°
        for (int s = 5; s <= 13; ++s)
            md = std::min(md, sec_[s]);
        return md;
    }

    // ── NEW: Right side wall distance ─────────────────────────────────────────
    // Sectors covering the robot's right flank (-45° to -135° in robot frame).
    // These sectors detect walls to the right during maze traversal.
    float right_dist() const
    {
        float md = 99.f;
        // Right side = negative angles ≈ -π/4 to -3π/4.
        // For NSEC=36: sector 23 ≈ -130°, sector 31 ≈ -50°
        for (int s = 23; s <= 31; ++s)
            md = std::min(md, sec_[s]);
        return md;
    }

    // Best free steering angle (robot frame) biased toward goalAngle
    // Only checks ±90° forward sectors (no phantom avoidance from sides)
    double bestAngle(double goalAngle) const
    {
        if (!ok_) return goalAngle;
        int best = -1; double bs = -1e9;
        for (int s = 0; s < NSEC; ++s) {
            double sa = sectorAngle(s);
            if (std::abs(GPSPose::wrap(sa))             > M_PI / 2) continue;
            if (sec_[s] < STOP_D)                                    continue;
            double penalty = std::abs(GPSPose::wrap(sa - goalAngle));
            if (penalty > M_PI / 4) continue;          // don't swing too far
            double score = sec_[s] - 1.2 * penalty;
            if (score > bs) { bs = score; best = s; }
        }
        return (best >= 0) ? sectorAngle(best) : goalAngle;
    }

    // Widest-clearance sector (escape direction), biased toward front half
    double widestAngle() const
    {
        int b = 0; float bd = -1e9f;
        for (int s = 0; s < NSEC; ++s) {
            float bonus = (std::abs(GPSPose::wrap(sectorAngle(s))) < M_PI / 2)
                          ? 0.5f : 0.f;
            float score = sec_[s] + bonus;
            if (score > bd) { bd = score; b = s; }
        }
        return sectorAngle(b);
    }

    bool isOk() const { return ok_; }
};

// ═══════════════════════════════════════════════════════════════════════════════
//  DepthObstacle  — lightweight check on /camera/depth/image_raw
//  Supports 32FC1 (float metres) and 16UC1 (uint16, millimetres)
// ═══════════════════════════════════════════════════════════════════════════════
class DepthObstacle
{
    float min_dist_ = 99.f;
    bool  ok_       = false;

public:
    void update(const sensor_msgs::msg::Image & img)
    {
        float md = 99.f;
        int W    = static_cast<int>(img.width);
        int H    = static_cast<int>(img.height);
        int c0   = static_cast<int>(W * (0.5 - DEPTH_STRIP / 2.0));
        int c1   = static_cast<int>(W * (0.5 + DEPTH_STRIP / 2.0));
        int r0   = static_cast<int>(H * 0.25);
        int r1   = static_cast<int>(H * 0.75);

        bool is32 = (img.encoding == "32FC1");
        bool is16 = (img.encoding == "16UC1");
        if (!is32 && !is16) { ok_ = false; return; }  // unsupported format

        for (int r = r0; r < r1; ++r) {
            for (int c = c0; c < c1; ++c) {
                float val = 0.f;
                if (is32) {
                    const float* row = reinterpret_cast<const float*>(
                        img.data.data() + r * img.step);
                    val = row[c];
                } else {
                    const uint16_t* row = reinterpret_cast<const uint16_t*>(
                        img.data.data() + r * img.step);
                    val = row[c] * 0.001f;   // mm → m
                }
                if (std::isfinite(val) && val > 0.05f && val < static_cast<float>(DEPTH_MAX))
                    md = std::min(md, val);
            }
        }
        min_dist_ = md;
        ok_ = true;
    }

    float frontDist()              const { return min_dist_; }
    bool  frontBlocked(double thr) const { return ok_ && min_dist_ < static_cast<float>(thr); }
    bool  isOk()                   const { return ok_; }
};

// ═══════════════════════════════════════════════════════════════════════════════
//  LocalMinimaDetector  — 2 s sliding window
// ═══════════════════════════════════════════════════════════════════════════════
class LocalMinimaDetector
{
    std::deque<std::pair<double, double>> buf_;
    static constexpr int    WIN = 40;    // ~2 s at 20 Hz loop
    static constexpr double THR = 0.25;  // metres moved in window

public:
    void update(double e, double n)
    {
        buf_.push_back({e, n});
        if (static_cast<int>(buf_.size()) > WIN) buf_.pop_front();
    }

    bool stuck(double dist_to_goal) const
    {
        if (static_cast<int>(buf_.size()) < WIN) return false;
        if (dist_to_goal < 1.5) return false;    // near goal — not stuck
        auto& ref = buf_.front();
        double mx = 0;
        for (auto& p : buf_)
            mx = std::max(mx, std::hypot(p.first - ref.first, p.second - ref.second));
        return mx < THR;
    }

    void reset() { buf_.clear(); }
};

// ═══════════════════════════════════════════════════════════════════════════════
//  ObstacleAvoidanceNode
// ═══════════════════════════════════════════════════════════════════════════════
class ObstacleAvoidanceNode : public rclcpp::Node
{
    // ── Core objects ──────────────────────────────────────────────────────────
    State               state_  = State::IDLE;
    GPSPose             pose_;
    LidarVFH            lidar_;
    DepthObstacle       depth_;
    LocalMinimaDetector minima_;
    std::mt19937        rng_{42};

    // ── Goal (ENU) ────────────────────────────────────────────────────────────
    double goal_e_ = 0, goal_n_ = 0;

    // ── Escape state ──────────────────────────────────────────────────────────
    int    esc_phase_  = 0;
    double esc_target_ = 0;   // absolute target yaw for turn phase
    double esc_timer_  = 0;   // countdown (seconds)

    // ── Wall-Following state ───────────────────────────────────────────────────
    // Entry position — used to enforce WF_MIN_TRAVEL before allowing exit.
    double wf_entry_e_       = 0;
    double wf_entry_n_       = 0;
    double wf_dist_moved_    = 0;  // metres moved since WF started
    int    wf_stuck_counter_ = 0;  // counts consecutive stuck ticks in WF

    // ── Tuning ────────────────────────────────────────────────────────────────
    double lin_  = 0.4;   // max forward speed
    double ang_  = 0.6;   // max angular speed

    // ── ROS interfaces ────────────────────────────────────────────────────────
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr          cmd_pub_;
    rclcpp::Subscription<sensor_msgs::msg::NavSatFix>::SharedPtr     gps_sub_;
    rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr           imu_sub_;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr     scan_sub_;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr         depth_sub_;
    rclcpp::Subscription<sensor_msgs::msg::NavSatFix>::SharedPtr     goal_sub_;
    rclcpp::TimerBase::SharedPtr timer_;

public:
    ObstacleAvoidanceNode() : Node("obstacle_avoidance_node")
    {
        // Parameters
        declare_parameter("linear_speed",  0.4);
        declare_parameter("angular_speed", 0.6);
        lin_ = get_parameter("linear_speed").as_double();
        ang_ = get_parameter("angular_speed").as_double();

        auto sq = rclcpp::SensorDataQoS();

        cmd_pub_  = create_publisher<geometry_msgs::msg::Twist>("/tars/cmd_vel", 10);

        gps_sub_  = create_subscription<sensor_msgs::msg::NavSatFix>(
            "/gps/fix", sq,
            [this](sensor_msgs::msg::NavSatFix::SharedPtr m){ pose_.updateGPS(*m); });

        imu_sub_  = create_subscription<sensor_msgs::msg::Imu>(
            "/imu", sq,
            [this](sensor_msgs::msg::Imu::SharedPtr m){ pose_.updateIMU(*m); });

        scan_sub_ = create_subscription<sensor_msgs::msg::LaserScan>(
            "/scan", sq,
            [this](sensor_msgs::msg::LaserScan::SharedPtr m){ lidar_.update(*m); });

        depth_sub_ = create_subscription<sensor_msgs::msg::Image>(
            "/depth/image_raw", sq,
            [this](sensor_msgs::msg::Image::SharedPtr m){ depth_.update(*m); });

        goal_sub_ = create_subscription<sensor_msgs::msg::NavSatFix>(
            "/goal_gps", 10,
            std::bind(&ObstacleAvoidanceNode::goalCb, this, _1));

        // 20 Hz control loop
        timer_ = create_wall_timer(
            std::chrono::milliseconds(50),
            std::bind(&ObstacleAvoidanceNode::loop, this));

        RCLCPP_INFO(get_logger(),
            "🚀 TARS Obstacle Avoidance Node READY\n"
            "   Sensors: GPS + IMU + LiDAR VFH + RealSense Depth\n"
            "   State: IDLE → waiting for /goal_gps\n"
            "   Maze solver: Left-hand wall-following (Bug2-inspired)");
    }

private:
    // ── Helpers ───────────────────────────────────────────────────────────────

    double wrap(double a) const { return GPSPose::wrap(a); }

    double distToGoal() const
    {
        return std::hypot(goal_e_ - pose_.e(), goal_n_ - pose_.n());
    }

    // Bearing to goal in robot frame (= heading error)
    double bearingError() const
    {
        double bearing = std::atan2(goal_n_ - pose_.n(), goal_e_ - pose_.e());
        return wrap(bearing - pose_.yaw());
    }

    // Cross-Track Error (perpendicular distance off the straight-line path)
    double crossTrackError() const
    {
        double d   = distToGoal();
        double err = bearingError();
        return d * std::sin(err);
    }

    // Fused front obstacle distance: min of LiDAR and depth camera
    double fusedFrontDist() const
    {
        double ld = lidar_.isOk() ? static_cast<double>(lidar_.frontDist()) : 99.0;
        double dd = depth_.isOk() ? static_cast<double>(depth_.frontDist()) : 99.0;
        return std::min(ld, dd);
    }

    bool fusedFrontBlocked(double thr) const
    {
        return fusedFrontDist() < thr;
    }

    void stop() { cmd_pub_->publish(geometry_msgs::msg::Twist()); }

    void publish(double linear_x, double angular_z)
    {
        geometry_msgs::msg::Twist cmd;
        cmd.linear.x  = linear_x;
        cmd.angular.z = angular_z;
        cmd_pub_->publish(cmd);
    }

    // ── Goal callback ─────────────────────────────────────────────────────────
    void goalCb(const sensor_msgs::msg::NavSatFix::SharedPtr msg)
    {
        if (!pose_.originSet()) {
            RCLCPP_WARN(get_logger(), "⚠️  No GPS fix yet — goal ignored");
            return;
        }
        ENU g   = pose_.toENU(msg->latitude, msg->longitude);
        goal_e_ = g.east;
        goal_n_ = g.north;
        minima_.reset();
        state_ = State::ALIGNING;
        RCLCPP_INFO(get_logger(),
            "🎯 New goal: lat=%.6f lon=%.6f  dist=%.2f m",
            msg->latitude, msg->longitude, distToGoal());
    }

    // ── Main control loop ─────────────────────────────────────────────────────
    void loop()
    {
        if (!pose_.ready()) {
            RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 3000,
                "⏳ Waiting for GPS + IMU...");
            return;
        }
        if (!lidar_.isOk()) {
            RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 3000,
                "⏳ Waiting for /scan...");
            return;
        }

        // Log dist + CTE every 500 ms (global planner telemetry — always active)
        double d   = distToGoal();
        double cte = crossTrackError();
        RCLCPP_INFO_THROTTLE(get_logger(), *get_clock(), 500,
            "[%s] Dist: %.4f m | CTE: %.4f m | Front: %.2f m",
            stateStr(), d, cte, fusedFrontDist());

        // Update local-minima detector regardless of state
        minima_.update(pose_.e(), pose_.n());

        switch (state_) {
            case State::IDLE:           doIdle();         break;
            case State::ALIGNING:       doAlign();        break;
            case State::DRIVING:        doDrive();        break;
            case State::WALL_FOLLOWING: doWallFollowing();break;
            case State::ESCAPING:       doEscape();       break;
            case State::DONE:           stop();           break;
        }
    }

    // ── IDLE ──────────────────────────────────────────────────────────────────
    void doIdle()
    {
        RCLCPP_INFO_THROTTLE(get_logger(), *get_clock(), 5000,
            "[IDLE] Waiting for /goal_gps ...");
    }

    // ── ALIGNING ──────────────────────────────────────────────────────────────
    // Rotate in place until heading error < HEAD_TOL
    // No forward motion; heading is corrected proportionally.
    void doAlign()
    {
        double err = bearingError();
        RCLCPP_INFO_THROTTLE(get_logger(), *get_clock(), 500,
            "[ALIGN] heading err = %.1f°", err * DEG);

        if (std::abs(err) < HEAD_TOL) {
            stop();
            state_ = State::DRIVING;
            RCLCPP_INFO(get_logger(), "✔ Aligned → DRIVING");
            return;
        }

        double az = std::clamp(0.8 * err, -ang_, ang_);
        publish(0.0, az);
    }

    // ── DRIVING ───────────────────────────────────────────────────────────────
    //
    // Global planner logic (dist + CTE) is ALWAYS active here.
    //
    // When road is clear (front > SLOW_D):
    //   steer = proportional to heading error (same as global_planner_node.cpp)
    //
    // When obstacle detected (front ≤ SLOW_D):
    //   steer = LiDAR VFH best angle biased toward goal bearing
    //   speed  = ramped down proportionally
    //
    // ⚠ NO DRIVING → ALIGNING re-trigger.
    //   Heading is corrected continuously — this prevents oscillation.
    void doDrive()
    {
        double d     = distToGoal();
        double herr  = bearingError();
        double front = fusedFrontDist();

        // ── Goal reached ──────────────────────────────────────────────────────
        if (d < GOAL_TOL) {
            stop();
            state_ = State::DONE;
            RCLCPP_INFO(get_logger(), "✅ GOAL REACHED  (dist=%.4f m)", d);
            return;
        }

        // ── Local minima → switch to wall-following (primary strategy) ────────
        if (minima_.stuck(d)) {
            RCLCPP_WARN(get_logger(),
                "⚠️  Local minima detected → WALL_FOLLOWING (left-hand rule)");
            startWallFollowing();
            return;
        }

        // ── Heading drift — re-align only if severely off course ──────────────
        // (preserved from global_planner_node.cpp: threshold 0.8 rad ≈ 46°)
        if (std::abs(herr) > 0.8) {
            RCLCPP_WARN(get_logger(), "⚠️  Severe heading drift (%.1f°) → re-ALIGNING",
                herr * DEG);
            state_ = State::ALIGNING;
            return;
        }

        // ── Emergency backup ──────────────────────────────────────────────────
        if (fusedFrontBlocked(STOP_D)) {
            double vfh_ang = lidar_.bestAngle(herr);
            double az = std::clamp(1.0 * vfh_ang, -ang_, ang_);
            publish(-0.08, az);
            RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 400,
                "🚨 EMG backup — front=%.2f m", front);
            return;
        }

        // ── Normal driving ────────────────────────────────────────────────────
        double steer;
        if (front > SLOW_D && std::abs(herr) < 0.15) {
            // Clear path: pure proportional heading correction
            // (mirrors global_planner_node.cpp  cmd.angular.z = 1.0 * err)
            steer = std::clamp(1.0 * herr, -ang_, ang_);
        } else if (front > SLOW_D) {
            // Clear path but non-trivial heading error: proportional with CTE boost
            double cte_gain = 0.3 * crossTrackError();
            steer = std::clamp(1.0 * herr + cte_gain, -ang_, ang_);
        } else {
            // Obstacle close: VFH steers around obstacle, still biased toward goal
            double vfh_ang = lidar_.bestAngle(herr);
            steer = std::clamp(0.8 * vfh_ang, -ang_, ang_);
        }

        // Speed: ramp down near obstacles and near goal
        // (mirrors global_planner_node.cpp:  min(0.5, 0.3*dist + 0.1))
        double base_spd = std::min(0.5, 0.3 * d + 0.1);
        double obs_fac  = std::clamp((front - STOP_D) / (SLOW_D - STOP_D), 0.2, 1.0);
        double spd      = base_spd * obs_fac;

        publish(spd, steer);

        RCLCPP_INFO_THROTTLE(get_logger(), *get_clock(), 800,
            "[DRIVE] dist=%.2f m | CTE=%.3f m | hdg_err=%.1f° | steer=%.1f° | "
            "front=%.2f m | spd=%.2f m/s",
            d, crossTrackError(), herr * DEG, steer * DEG, front, spd);
    }

    // ── WALL_FOLLOWING ────────────────────────────────────────────────────────
    //
    // Left-hand rule (Bug2-inspired) maze solver.
    //
    // Logic per tick:
    //   1. EXIT CHECK: if front is clear AND |bearingError| < 60° AND robot
    //      has moved WF_MIN_TRAVEL metres from entry point → go to ALIGNING.
    //   2. FRONT BLOCKED: turn right in place (clockwise) to clear the wall.
    //   3. LEFT WALL PRESENT: P-control to maintain WF_WALL_DIST from left wall.
    //      Steer away (right) if too close, steer left if too far.
    //   4. NO LEFT WALL: steer left to find/hug the next wall segment.
    //
    // Speed is fixed at 55% of lin_ to give the proportional controller time
    // to react in narrow corridors without overshooting.
    //
    void startWallFollowing()
    {
        wf_entry_e_       = pose_.e();
        wf_entry_n_       = pose_.n();
        wf_dist_moved_    = 0.0;
        wf_stuck_counter_ = 0;
        minima_.reset();
        state_ = State::WALL_FOLLOWING;
        RCLCPP_INFO(get_logger(),
            "[WF] Wall-Following STARTED at (%.2f, %.2f) — left-hand rule",
            wf_entry_e_, wf_entry_n_);
    }

    void doWallFollowing()
    {
        double d     = distToGoal();
        double herr  = bearingError();
        double front = fusedFrontDist();
        double left  = static_cast<double>(lidar_.left_dist());
        double right = static_cast<double>(lidar_.right_dist());

        // Update distance moved since WF start
        wf_dist_moved_ = std::hypot(pose_.e() - wf_entry_e_,
                                    pose_.n() - wf_entry_n_);

        RCLCPP_INFO_THROTTLE(get_logger(), *get_clock(), 600,
            "[WF] dist=%.2fm | CTE=%.3fm | hdg_err=%.1f° | "
            "front=%.2fm | left=%.2fm | right=%.2fm | moved=%.2fm",
            d, crossTrackError(), herr * DEG,
            front, left, right, wf_dist_moved_);

        // ── Goal reached ──────────────────────────────────────────────────────
        if (d < GOAL_TOL) {
            stop();
            state_ = State::DONE;
            RCLCPP_INFO(get_logger(), "✅ GOAL REACHED during WF  (dist=%.4f m)", d);
            return;
        }

        // ── EXIT CONDITION ────────────────────────────────────────────────────
        // Leave wall-following when the direct path to goal is clear:
        //   • Front is open  (> SLOW_D)
        //   • Bearing to goal is within ±60°  (robot is roughly facing the goal)
        //   • Robot has moved enough from the entry point (prevents premature exit
        //     at the mouth of a U-trap before the wall carries us around)
        bool front_clear   = (front > SLOW_D);
        bool bearing_clear = (std::abs(herr) < CLEAR_BEARING_TOL);
        bool moved_enough  = (wf_dist_moved_ > WF_MIN_TRAVEL);

        if (front_clear && bearing_clear && moved_enough) {
            stop();
            minima_.reset();
            state_ = State::ALIGNING;
            RCLCPP_INFO(get_logger(),
                "[WF] Exit condition met — bearing=%.1f° front=%.2fm moved=%.2fm → ALIGNING",
                herr * DEG, front, wf_dist_moved_);
            return;
        }

        // ── WF STUCK WITHIN WALL-FOLLOWING ───────────────────────────────────
        // If the robot still isn't moving after a long time in WF, escalate to
        // ESCAPING as a last resort.
        if (minima_.stuck(d)) {
            wf_stuck_counter_++;
            if (wf_stuck_counter_ > 3) {
                RCLCPP_WARN(get_logger(),
                    "[WF] Still stuck after %d retries → ESCAPING (last resort)",
                    wf_stuck_counter_);
                startEscape();
                return;
            }
            // Otherwise, try rotating right to find a way out
            RCLCPP_WARN(get_logger(),
                "[WF] Stuck in WF (%d/3) — spinning right to find opening",
                wf_stuck_counter_);
            minima_.reset();
            publish(0.0, -ang_ * 0.7);
            return;
        } else {
            wf_stuck_counter_ = 0;
        }

        // ── RULE 1: Front blocked → turn right (clockwise) ────────────────────
        if (front < WF_FRONT_TURN) {
            // Turn right to clear the frontal wall — no forward motion
            publish(0.0, -ang_ * 0.75);
            RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 400,
                "[WF] Front blocked (%.2fm) — turning right", front);
            return;
        }

        // ── RULE 2: Left wall present → P-control wall distance ───────────────
        // Target: maintain WF_WALL_DIST to the left wall.
        // Error > 0  : too close  → steer right (negative angular.z)
        // Error < 0  : too far    → steer left  (positive angular.z)
        if (left < 2.0) {
            double wall_err = WF_WALL_DIST - left;   // +: too close, -: too far
            double az = std::clamp(-WF_WALL_GAIN * wall_err, -ang_, ang_);
            double spd = lin_ * 0.55;
            publish(spd, az);
            RCLCPP_INFO_THROTTLE(get_logger(), *get_clock(), 600,
                "[WF] Hugging left wall: left=%.2fm  err=%.2fm  az=%.2f",
                left, wall_err, az);
            return;
        }

        // ── RULE 3: No left wall → arc left to find next wall segment ─────────
        // This happens at open corners: keep moving and sweep left.
        publish(lin_ * 0.45, ang_ * 0.55);
        RCLCPP_INFO_THROTTLE(get_logger(), *get_clock(), 600,
            "[WF] No left wall (%.2fm) — arcing left to find wall", left);
    }

    // ── ESCAPING ──────────────────────────────────────────────────────────────
    // Legacy random-escape, now used only as a last resort when wall-following
    // itself gets stuck (deeply concave obstacles, dead corners, etc.)
    void startEscape()
    {
        double rel   = lidar_.isOk()
                       ? lidar_.widestAngle()
                       : std::uniform_real_distribution<double>(-M_PI, M_PI)(rng_);
        esc_target_  = wrap(pose_.yaw() + rel);
        esc_phase_   = 0;
        esc_timer_   = 2.5;    // seconds for turn phase
        minima_.reset();
        state_ = State::ESCAPING;
        RCLCPP_INFO(get_logger(),
            "[ESC] Escape dir = %.1f° (abs yaw)", esc_target_ * DEG);
    }

    void doEscape()
    {
        constexpr double DT = 0.05;   // loop period
        esc_timer_ -= DT;

        double yerr = wrap(esc_target_ - pose_.yaw());

        if (esc_phase_ == 0) {
            // Turn toward escape direction
            double az = std::copysign(ang_ * 0.7, yerr);
            publish(0.0, az);
            if (std::abs(yerr) < 0.15 || esc_timer_ <= 0.0) {
                esc_phase_ = 1;
                esc_timer_ = 3.0;   // seconds for drive phase
                RCLCPP_INFO(get_logger(), "[ESC] Turned → drive phase");
            }
        } else {
            // Drive in escape direction
            if (!fusedFrontBlocked(STOP_D))
                publish(lin_ * 0.55, 0.0);
            else
                publish(0.0, ang_ * 0.6);

            if (esc_timer_ <= 0.0) {
                minima_.reset();
                state_ = State::ALIGNING;
                RCLCPP_INFO(get_logger(), "[ESC] Done → ALIGNING");
            }
        }
    }

    const char* stateStr() const
    {
        switch (state_) {
            case State::IDLE:           return "IDLE";
            case State::ALIGNING:       return "ALIGNING";
            case State::DRIVING:        return "DRIVING";
            case State::WALL_FOLLOWING: return "WALL_FOLLOWING";
            case State::ESCAPING:       return "ESCAPING";
            case State::DONE:           return "DONE";
        }
        return "?";
    }
};

// ═══════════════════════════════════════════════════════════════════════════════
int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ObstacleAvoidanceNode>());
    rclcpp::shutdown();
    return 0;
}
