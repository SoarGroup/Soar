#include "ros_interface.h"
#include "svs.h"
#include <iostream>

ros::AsyncSpinner* ros_interface::spinner = 0;

ros_interface::ros_interface(svs* sp) {
    svs_ptr = sp;
    set_up_subscribers();
}

ros_interface::~ros_interface() {
    stop_ros();
}

void ros_interface::init_ros() {
    int argc = 0;
    char* argv;

    ros::init(argc, &argv, "soar_svs");
}

void ros_interface::start_ros() {
    if (!spinner) {
        spinner = new ros::AsyncSpinner(4);
        spinner->start();
    }
}

void ros_interface::stop_ros() {
    if (spinner) spinner->stop();
}

// Subscribes to the necessary ROS topics
void ros_interface::set_up_subscribers() {
    objects_sub = n.subscribe("gazebo/model_states", 5, &ros_interface::objects_callback, this);
    joints_sub = n.subscribe("joint_states", 5, &ros_interface::joints_callback, this);

    pc_sub = n.subscribe("head_camera/depth_registered/points", 5, &ros_interface::pc_callback, this);
}

// (Will do something?) when a new world state is received
void ros_interface::objects_callback(const gazebo_msgs::ModelStates::ConstPtr& msg) {
    //std::cout << "Received objects!" << std::endl;
}

// (Will do something?) when a new arm position is received
void ros_interface::joints_callback(const sensor_msgs::JointState::ConstPtr& msg) {
    //std::cout << "Received joints!" << std::endl;
}

// Updates the images in SVS states when a new point cloud is received
void ros_interface::pc_callback(const pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr& msg) {
    svs_ptr->image_callback(msg);
}
