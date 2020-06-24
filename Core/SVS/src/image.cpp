#include "image.h"
#include <iostream>

image::image() : source("none") {
}

// Copies a point cloud ROS message into this image container.
void image::update_image(const pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr& new_img) {
    bool prev_empty = is_empty();

    pc.header = new_img->header;
    pc.width = new_img->width;
    pc.height = new_img->height;
    pc.is_dense = new_img->is_dense;
    pc.sensor_orientation_ = new_img->sensor_orientation_;
    pc.sensor_origin_ = new_img->sensor_origin_;
    pc.points = new_img->points;

    if (is_empty() != prev_empty) notify_listeners();
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
    source = other->source;
}

void image::set_source(std::string src) {
    source = src;
    notify_listeners();
}

void image::add_listener(image_descriptor* id) {
    listeners.push_back(id);
}

void image::remove_listener(image_descriptor* id) {
    listeners.remove(id);
}

bool image::is_empty() {
    return (pc.width == 0 && pc.height == 0);
}

void image::notify_listeners() {
    for (std::list<image_descriptor*>::iterator i = listeners.begin();
         i != listeners.end(); i++) {
        (*i)->update_desc();
    }
}

const std::string image_descriptor::source_tag = "source";
const std::string image_descriptor::empty_tag = "empty";

image_descriptor::image_descriptor(soar_interface* si, Symbol* ln, image* im)
    : img(im), link(ln), si(si), source("none"), empty("true")
{
    img->add_listener(this);

    source_wme = si->make_wme(link,
                              source_tag,
                              source);
    empty_wme = si->make_wme(link,
                             empty_tag,
                             empty);
}

void image_descriptor::update_desc() {
    bool updated = false;
    if (empty == "true" && !img->is_empty()) {
        empty = "false";
        updated = true;
    } else if (empty == "false" && img->is_empty()) {
        empty = "true";
        updated = true;
    }
    if (updated) {
        si->remove_wme(empty_wme);
        empty_wme = si->make_wme(link,
                                 empty_tag,
                                 empty);
    }

    if (source != img->get_source()) {
        source = img->get_source();
        si->remove_wme(source_wme);
        source_wme = si->make_wme(link,
                                  source_tag,
                                  source);
    }
}
