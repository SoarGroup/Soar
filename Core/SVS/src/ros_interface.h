#ifndef ROS_INTERFACE_H
#define ROS_INTERFACE_H

#include <map>
#include <ros/ros.h>
#include <pcl_ros/point_cloud.h>
#include <pcl/point_types.h>
#include "gazebo_msgs/ModelStates.h"
#include "sensor_msgs/JointState.h"

#include "mat.h"

class svs;

class ros_interface {
public:
    ros_interface(svs* sp);
    ~ros_interface();
    static void init_ros();
    static void start_ros();
    static void stop_ros();

private:
    static const double POS_THRESH;
    static const double ROT_THRESH;
    bool t_diff(vec3& p1, vec3& p2);
    bool t_diff(Eigen::Quaterniond& q1, Eigen::Quaterniond& q2);

    void set_up_subscribers();
    void objects_callback(const gazebo_msgs::ModelStates::ConstPtr& msg);
    void joints_callback(const sensor_msgs::JointState::ConstPtr & msg);
    void pc_callback(const pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr& msg);

    ros::NodeHandle n;
    ros::Subscriber objects_sub;
    ros::Subscriber joints_sub;
    ros::Subscriber pc_sub;
    static ros::AsyncSpinner* spinner;

    svs* svs_ptr;
    std::map<std::string, transform3> last_objs;
};

#endif
