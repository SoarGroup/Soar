#ifndef IMAGE_H
#define IMAGE_H

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

class svs;

class image
{
public:
    image();
    void update_image(const pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr& new_img);
    void copy_from(image* other);

private:
    pcl::PointCloud<pcl::PointXYZRGB> pc;
};

#endif
