#ifndef ROS_INTERFACE_H
#define ROS_INTERFACE_H

#include <map>
#include <ros/ros.h>
#include <pcl_ros/point_cloud.h>
#include <pcl/point_types.h>
#include "gazebo_msgs/ModelStates.h"
#include "sensor_msgs/JointState.h"

#include "mat.h"
#include "cliproxy.h"

class svs;

/*
 * ros_interface class
 *
 * Provides the necessary boilerplate to make SVS into a ROS
 * node, such as the NodeHandle, init functionality, and subscribers.
 * Includes callback functions that take ROS input and put point
 * cloud data into the image holders and update the scene graph from
 * Gazebo objects.
 *
 * CLI USAGE:
 *
 * svs ros - Prints all inputs and whether they're enabled or disabled
 * svs ros.enable_all - Enables all inputs
 * svs ros.disable_all - Disables all inputs
 * svs ros.enable_image - Enables the image input
 * svs ros.disable_image - Disables the image input
 * svs ros.enable_sg - Enables the scene graph input
 * svs ros.disable_sg - Disables the scene graph input
 *
 * XXX: Update interface so that the input name is an argument.
 */

class ros_interface : public cliproxy {
public:
    ros_interface(svs* sp);
    ~ros_interface();
    static void init_ros();
    void start_ros();
    void stop_ros();

    std::string get_image_source() { return image_source; }

private:
    static const double POS_THRESH;
    static const double ROT_THRESH;
    bool t_diff(vec3& p1, vec3& p2);
    bool t_diff(Eigen::Quaterniond& q1, Eigen::Quaterniond& q2);

    void subscribe_image();
    void unsubscribe_image();
    void subscribe_sg();
    void unsubscribe_sg();
    void objects_callback(const gazebo_msgs::ModelStates::ConstPtr& msg);
    void joints_callback(const sensor_msgs::JointState::ConstPtr & msg);
    void pc_callback(const pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr& msg);

    void proxy_get_children(std::map<std::string, cliproxy*>& c);
    void proxy_use_sub(const std::vector<std::string>& args, std::ostream& os);
    void enable_all(const std::vector<std::string>& args, std::ostream& os);
    void disable_all(const std::vector<std::string>& args, std::ostream& os);
    void enable_image(const std::vector<std::string>& args, std::ostream& os);
    void disable_image(const std::vector<std::string>& args, std::ostream& os);
    void enable_sg(const std::vector<std::string>& args, std::ostream& os);
    void disable_sg(const std::vector<std::string>& args, std::ostream& os);

    ros::NodeHandle n;
    bool update_image, update_sg;
    ros::Subscriber objects_sub;
    ros::Subscriber joints_sub;
    ros::Subscriber pc_sub;
    std::string image_source;
    ros::AsyncSpinner* spinner;

    svs* svs_ptr;
    std::map<std::string, transform3> last_objs;
};

#endif
