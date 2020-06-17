#include "image.h"
#include <iostream>

image::image() {
}

void image::update_image(pcl::PointCloud<pcl::PointXYZRGB> new_img) {
    pc.swap(new_img);
}

void image::copy_from(image* other) {
    pc.swap(other->pc);
}
