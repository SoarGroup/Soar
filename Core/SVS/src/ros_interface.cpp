#include "ros_interface.h"

#include <iostream>
#include <string>
#include <sstream>
#include <math.h>

#include "svs.h"

const double ros_interface::POS_THRESH = 0.001; // 1 mm
const double ros_interface::ROT_THRESH = 0.017; // approx 1 deg


ros_interface::ros_interface(svs* sp) : image_source("none") {
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
    image_source = "none";
}

bool ros_interface::t_diff(vec3& p1, vec3& p2) {
    if (sqrt(pow(p1.x() - p2.x(), 2) +
             pow(p1.y() - p2.y(), 2) +
             pow(p1.z() - p2.z(), 2)) > POS_THRESH) return true;
    return false;
}

bool ros_interface::t_diff(Eigen::Quaterniond& q1, Eigen::Quaterniond& q2) {
    double a = 2 * acos(q1.dot(q2));
    if (a > ROT_THRESH) return true;
    return false;
}

// Subscribes to the necessary ROS topics
void ros_interface::set_up_subscribers() {
    objects_sub = n.subscribe("gazebo/model_states", 5, &ros_interface::objects_callback, this);
    joints_sub = n.subscribe("joint_states", 5, &ros_interface::joints_callback, this);

    pc_sub = n.subscribe("head_camera/depth_registered/points", 5, &ros_interface::pc_callback, this);
    image_source = "fetch";
}

// when a new world state is received
void ros_interface::objects_callback(const gazebo_msgs::ModelStates::ConstPtr& msg) {
    std::map<std::string, transform3> current_objs;

    for (int i = 0; i <  msg->name.size(); i++) {
        geometry_msgs::Pose pose = msg->pose[i];
        std::string n = msg->name[i];

        vec3 p(pose.position.x,
               pose.position.y,
               pose.position.z);
        Eigen::Quaterniond q(pose.orientation.w,
                             pose.orientation.x,
                             pose.orientation.y,
                             pose.orientation.z);
        // XXX: Is this right? Euler order in exsiting SVS?
        vec3 r = q.toRotationMatrix().eulerAngles(0, 1, 2);

        transform3 t(p, r, vec3(1, 1, 1));
        current_objs.insert(std::pair<std::string, transform3>(n, t));
    }

    std::stringstream cmds;
    bool objs_changed = false;

    // Add commands
    for (std::map<std::string, transform3>::iterator i = current_objs.begin();
         i != current_objs.end(); i++) {
        if (last_objs.count(i->first) == 0) {
            objs_changed = true;
            std::string n = i->first;
            vec3 cur_pose;
            i->second.position(cur_pose);
            Eigen::Quaterniond rq;
            i->second.rotation(rq);
            vec3 cur_rot = rq.toRotationMatrix().eulerAngles(0, 1, 2);

            cmds << "add " << n << " world ";
            cmds << "p " << cur_pose.x() << " " << cur_pose.y() << " " << cur_pose.z();
            cmds << " r " << cur_rot.x() << " " << cur_rot.y() << " " << cur_rot.z();
            cmds << std::endl;
        }
    }

    for (std::map<std::string, transform3>::iterator i = last_objs.begin();
         i != last_objs.end(); i++) {
        // Delete commands
        if (current_objs.count(i->first) == 0) {
            objs_changed = true;
            cmds << "delete " << i->first << " " << std::endl;
            continue;
        }

        // Change commands
        std::string n = i->first;
        vec3 last_pose;
        i->second.position(last_pose);
        vec3 cur_pose;
        current_objs[n].position(cur_pose);
        Eigen::Quaterniond last_rot;
        i->second.rotation(last_rot);
        Eigen::Quaterniond cur_rot;
        current_objs[n].rotation(cur_rot);

        if (t_diff(last_pose, cur_pose) || t_diff(last_rot, cur_rot)) {
            objs_changed = true;
            cmds << "change " << n;
            if (t_diff(last_pose, cur_pose)) {
                cmds << " p " << cur_pose.x() << " " << cur_pose.y() << " " << cur_pose.z();
            }
            if (t_diff(last_rot, cur_rot)) {
                vec3 rpy = cur_rot.toRotationMatrix().eulerAngles(0, 1, 2);
                cmds << " r " << rpy.x() << " " << rpy.y() << " " << rpy.z();
            }
            cmds << std::endl;
        }
    }

    last_objs = current_objs;
    if (objs_changed) {
        svs_ptr->add_input(cmds.str());
    }
}

// (Will do something?) when a new arm position is received
void ros_interface::joints_callback(const sensor_msgs::JointState::ConstPtr& msg) {
    //std::cout << "Received joints!" << std::endl;
}

// Updates the images in SVS states when a new point cloud is received
void ros_interface::pc_callback(const pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr& msg) {
    svs_ptr->image_callback(msg);
}
