#include "image.h"
#include <iostream>

image::image() {
}

// Copies a point cloud ROS message into this image container.
void image::update_image(const pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr& new_img) {
    pc.header = new_img->header;
    pc.width = new_img->width;
    pc.height = new_img->height;
    pc.is_dense = new_img->is_dense;
    pc.sensor_orientation_ = new_img->sensor_orientation_;
    pc.sensor_origin_ = new_img->sensor_origin_;
    pc.points = new_img->points;
}

// Copies the point cloud from another image into this image container.
void image::copy_from(image* other) {
    pc.header = other->pc.header;
    pc.width = other->pc.width;
    pc.height = other->pc.height;
    pc.is_dense = other->pc.is_dense;
    pc.sensor_orientation_ = other->pc.sensor_orientation_;
    pc.sensor_origin_ = other->pc.sensor_origin_;
    pc.points = other->pc.points;
}
