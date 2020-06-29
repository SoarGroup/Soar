#ifndef IMAGE_H
#define IMAGE_H

#ifdef ENABLE_ROS
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#endif

#include <list>

#include "soar_interface.h"

class svs;
class image_descriptor;

/*
 * image_base class
 *
 * Abstract base class for all image types. Allows the ROS version
 * and non-ROS version of this code to both work. Designed to
 * hold the pixel data of the image in SVS WM. The base class does
 * not actually have an image data structure through, just the
 * interface.
 *
 */

class image_base
{
public:
    virtual int get_width() = 0;
    virtual int get_height() = 0;
    virtual bool is_empty() = 0;

    void set_source(std::string src);
    std::string get_source() { return source; }

    void add_listener(image_descriptor* id);
    void remove_listener(image_descriptor* id);

protected:
    void notify_listeners();

    std::string source;
    std::list<image_descriptor*> listeners;
};


/*
 * basic_image class
 *
 * The derived image class for the non-ROS version of SVS. The image
 * data is just a 2D array.
 *
 */

struct pixel {
    int r, g, b;
};

class basic_image : public image_base
{
public:
    basic_image();

    void update_image(std::vector<std::vector<pixel> >& new_img);
    void copy_from(basic_image* other);

    int get_width();
    int get_height();
    bool is_empty();

private:
    std::vector<std::vector<pixel> > img_array;
};

/*
 * pcl_image class
 *
 * The derived image class for holding XYZRGB point clouds in a PCL data
 * structure in the ROS version of SVS. update_image is called when new
 * data is received from ROS; copy_from is called when a new Soar state is
 * created.
 *
 */

#ifdef ENABLE_ROS
class pcl_image : public image_base
{
public:
    pcl_image();

    void update_image(const pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr& new_img);
    void copy_from(pcl_image* other);

    int get_width() { return pc.width; }
    int get_height() { return pc.height; }
    bool is_empty();

private:
    pcl::PointCloud<pcl::PointXYZRGB> pc;
};
#endif

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
    image_descriptor(soar_interface* si, Symbol* ln, image_base* im);
    ~image_descriptor();

    image_base* get_image() { return img; }
    void update_desc();

    const static std::string source_tag;
    const static std::string empty_tag;

private:

    image_base* img;
    Symbol* link;
    wme* source_wme;
    wme* empty_wme;
    soar_interface* si;

    std::string source;
    std::string empty;
};

#endif
