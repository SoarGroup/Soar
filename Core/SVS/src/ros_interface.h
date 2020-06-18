#ifndef ROS_INTERFACE_H
#define ROS_INTERFACE_H

#include <ros/ros.h>
#include <pcl_ros/point_cloud.h>
#include <pcl/point_types.h>
#include "gazebo_msgs/ModelStates.h"
#include "sensor_msgs/JointState.h"

class ros_interface {
public:
    ros_interface();
    ~ros_interface();
    static void init_ros();
    static void start_ros();
    static void stop_ros();

private:
    void set_up_subscribers();
    void objects_callback(const gazebo_msgs::ModelStates::ConstPtr& msg);
    void joints_callback(const sensor_msgs::JointState::ConstPtr & msg);

    ros::NodeHandle n;
    ros::Subscriber objects_sub;
    ros::Subscriber joints_sub;
    static ros::AsyncSpinner* spinner;
};

#endif
