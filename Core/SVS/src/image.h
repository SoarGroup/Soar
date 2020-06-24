#ifndef IMAGE_H
#define IMAGE_H

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <list>

#include "soar_interface.h"

class svs;
class image_descriptor;

/*
 * image class
 *
 * Holds the pixel data of the image in SVS WM. The update_image
 * function allows it to accept outside data as input, and its listeners
 * allow for an image_desriptor to be updated with high-level information
 * about the image, such as whether or not it's empty.
 *
 */

class image
{
public:
    image();
    void update_image(const pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr& new_img);
    void copy_from(image* other);
    int get_width() { return pc.width; }
    int get_height() { return pc.height; }
    void set_source(std::string src);
    std::string get_source() { return source; }

    void add_listener(image_descriptor* id);
    void remove_listener(image_descriptor* id);

    bool is_empty();

private:
    void notify_listeners();

    pcl::PointCloud<pcl::PointXYZRGB> pc;
    std::string source;
    std::list<image_descriptor*> listeners;
};

/*
 * image_descriptor class
 *
 * Similar to a sgwme, this class listens to an image and updates
 * the image link wme with high-level information about the image.
 *
 */

class image_descriptor
{
public:
    image_descriptor(soar_interface* si, Symbol* ln, image* im);
    ~image_descriptor();

    image* get_image() { return img; }
    void update_desc();

    const static std::string source_tag;
    const static std::string empty_tag;

private:

    image* img;
    Symbol* link;
    wme* source_wme;
    wme* empty_wme;
    soar_interface* si;

    std::string source;
    std::string empty;
};

#endif
